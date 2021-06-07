// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <stdlib.h>

#include <ugba/ugba.h>

#include "room_game/building_info.h"
#include "room_game/draw_building.h"
#include "room_game/draw_common.h"
#include "room_game/room_game.h"
#include "room_game/text_messages.h"
#include "room_game/tileset_info.h"
#include "simulation/building_density.h"
#include "simulation/simulation_building_count.h"
#include "simulation/simulation_fire.h"
#include "simulation/simulation_traffic.h"
#include "simulation/simulation_transport_anims.h"

void Simulation_Radiation(void)
{
    // Remove radiation

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t tile, type;
            CityMapGetTypeAndTile(i, j, &tile, &type);

            if (type != TYPE_RADIATION)
                continue;

            // Only remove radiation if rand() = 0 (1 in 256 chance)
            int r = rand() & 0xFF;
            if (r > 0)
                continue;

            // If water, set to water again. If ground, set to ground

            if (tile == T_RADIATION_GROUND)
            {
                CityMapDrawTile(T_GRASS, i, j);
            }
            else if (tile == T_RADIATION_WATER)
            {
                CityMapDrawTile(T_WATER, i, j);
                MapUpdateWater(i, j);
            }
        }
    }
}

// Spread radiation around here. The coordinates are considered to be the top
// left coordinates of the power plant that is the source of the radiation.
void Simulation_RadiationSpread(int x, int y)
{
    // Center of the power plant

    // This needs to change whenever room_game/building_info.c changes
    int cx = x + 1;
    int cy = y + 1;

    // Radiation can destroy buildings. Consider adding a tile of radiation the
    // same thing as destroying a tile with fire.

    const int count = 16; // Number of tiles of radiation to generate

    for (int c = 0; c < count; c++)
    {
        // Move to a random point surounding the center
        int r = rand();
        int rx = cx + ((r & 15) - 8);
        r = rand();
        int ry = cy + ((r & 15) - 8);

        // Check if the result is outside of the map
        if ((rx < 0) || (rx >= CITY_MAP_WIDTH))
            continue;
        if ((ry < 0) || (ry >= CITY_MAP_HEIGHT))
            continue;

        uint16_t tile, type;
        CityMapGetTypeAndTile(rx, ry, &tile, &type);

        // If there is already radiation here, skip this tile
        if (type == TYPE_RADIATION)
            continue;

        // Make the building in this tile explode and then place a radiation
        // tile on it.

        const city_tile_density_info *di = CityTileDensityInfo(tile);
        if (di->fire_probability > 0)
        {
            // This is burnable
            MapDeleteBuildingFire(rx, ry);
        }

        // Now, replace the tile by a radiation tile. Check if water or land to
        // write the correct radiation tile.

        uint16_t masked_type = type & TYPE_MASK;

        if ((masked_type == TYPE_WATER) || (masked_type == TYPE_DOCK))
            CityMapDrawTile(T_RADIATION_WATER, rx, ry);
        else
            CityMapDrawTile(T_RADIATION_GROUND, rx, ry);
    }
}

// If "force" is 1, it will always create a meltdown (if there are nuclear power
// plants in the map). If it is 0, it is random.
void Simulation_MeltdownTryStart(int force)
{
    // Don't start a disaster if there is already a fire
    if (Room_Game_IsInDisasterMode())
        return;

    // If there are no nucler power plants, return
    building_count_info *info = Simulation_CountBuildingsGet();

    int power_plant_count = info->nuclear_power_plants;
    if (power_plant_count == 0)
        return;

    int affected_plant = -1;

    // If forced, force explosion at the first plant
    if (force == 1)
    {
        affected_plant = 0;
    }
    else
    {
        // For each nucler power plant, check if it explodes. If it does, search
        // the map for its position and make it explode, force a fire there and
        // start disaster mode. When a nuclear plant catches fire it spreads
        // radiactive tiles (it is done in the function that burns buildings,
        // there's a special case for nuclear fission power plants), so the
        // tiles don't have to be spread in this function.

        for (int i = 0; i < power_plant_count; i++)
        {
            int r = rand() & 0xFF;
            if (r == 0)
            {
                affected_plant = i;
                break;
            }
        }

        // In 1 out of each 8 cases, simply return
        if ((rand() & 7) == 0)
            return;
    }

    // No affected plant, return
    if (affected_plant == -1)
        return;

    // Look for the power plant that generated the explosion
    // -----------------------------------------------------

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t tile, type;
            CityMapGetTypeAndTile(i, j, &tile, &type);

            if (affected_plant > 0)
            {
                affected_plant--;
                continue;
            }

            if (tile != T_POWER_PLANT_NUCLEAR_CENTER)
                continue;

            // This will spread radiation around the power plant when it detects
            // that the building is a nuclear power plant.
            MapDeleteBuildingFire(i, j);

            MessageQueueAdd(ID_MSG_NUCLEAR_MELTDOWN);

            int scx = i - ((GBA_SCREEN_W / 2) / 8);
            if (scx < 0)
                scx = 0;

            int scy = j - ((GBA_SCREEN_H / 2) / 8);
            if (scy < 0)
                scy = 0;

            Room_Game_Request_Scroll(scx, scy);

            // Remove all traffic tiles from the map and other animations
            // ----------------------------------------------------------

            Simulation_TrafficRemoveAnimationTiles();

            Simulation_TransportAnimsHide();

            // Enable disaster mode
            // --------------------

            Room_Game_SetDisasterMode(1);

            return;
        }
    }

    // A power plant should have exploded, this shouldn't be reached
    UGBA_Assert(0);
}
