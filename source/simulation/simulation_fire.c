// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <stdlib.h>
#include <string.h>

#include <ugba/ugba.h>

#include "room_game/building_info.h"
#include "room_game/draw_building.h"
#include "room_game/draw_common.h"
#include "room_game/room_game.h"
#include "room_game/tileset_info.h"
#include "simulation/building_density.h"
#include "simulation/simulation_building_count.h"
#include "simulation/simulation_traffic.h"

// The count saturates to 31 - 1. The number of fire stations used for the
// calculations is always increased by 1 to make sure that fires end even in a
// city with no fire stations.
//
// This number is calculated before the fire starts. In case the fire destroys a
// fire station, it still counts for the current fire.

#define MAX_USEFUL_NUMBER_FIRE_STATIONS     (30)

int initial_number_fire_stations;
int extinguish_fire_probability;

EWRAM_BSS static uint8_t fire_map[CITY_MAP_WIDTH * CITY_MAP_HEIGHT];

// Removes a building and replaces it with fire. Fire SFX
void MapDeleteBuildingFire(int x, int y)
{
    // Check origin of coordinates of the building
    // ------------------------------------------

    uint16_t tile, type;
    CityMapGetTypeAndTile(x, y, &tile, &type);

    const city_tile_info *info = City_Tileset_Entry_Info(tile);

    int ox = x + info->base_x_delta;
    int oy = y + info->base_y_delta;

    uint16_t origin_tile = CityMapGetTile(ox, oy);

    const building_info *bi = Get_BuildingFromBaseTile(origin_tile);
    int w = bi->width;
    int h = bi->height;

    // Force removal
    // -------------

    Building_Remove(1, ox, oy);

    // TODO: SFX_FireExplosion()

    // TODO: Maybe support burning bridges?

    // Handle nuclear power plant radiation
    // ------------------------------------

    // If the building is a nuclear power plant, spread radiation around it.
    // This can call recursively to the function we are in right now!
    // TODO: Simulation_RadiationSpread(ox, oy)

    // Replace building by fire
    // ------------------------

    for (int j = oy; j < (oy + h); j++)
    {
        for (int i = ox; i < (ox + w); i++)
        {
            CityMapDrawTile(T_FIRE_1, i, j);
        }
    }
}

// If "force" is 1, it will always create a fire (if there are tiles that can be
// burnt). If it is 0, it is random.
void Simulation_FireTryStart(int force)
{
    // Don't start a fire if there is already a fire
    if (Room_Game_IsInDisasterMode())
        return;

    // Save initial number of fire stations
    // ------------------------------------

    building_count_info *info = Simulation_CountBuildingsGet();

    initial_number_fire_stations = info->fire_stations;

    if (force == 0)
    {
        // Check if a fire has to start or not
        // -----------------------------------

        // The chances of it happening depend on the number of fire stations

        int probabilities;
        if (initial_number_fire_stations > 4)
            probabilities = 0;
        else
            probabilities = 4 - initial_number_fire_stations;

        probabilities++; // Leave at least a 1 in 256 chance of fire!

        int r = rand() & 0xFF;

        if (r > probabilities)
            return;
    }

    // Look for a place to start the fire

    // Try to get valid starting coordinates a few times, there must be a valid
    // burnable tile there. If all of them fail, give up and return.

    int fire_x = -1;
    int fire_y = -1;

    const int retries = 10;

    for (int c = 0; c < retries; c++)
    {
        int x = rand() & (CITY_MAP_WIDTH - 1);
        int y = rand() & (CITY_MAP_HEIGHT - 1);

        uint16_t tile = CityMapGetTile(x, y);

        // Note: Docks can't burn. Check soruce/simulation/building_density.c
        // and make sure that the fire_probability of the dock tiles is 0.
        // Bridges can't burn either.
        const city_tile_density_info *di = CityTileDensityInfo(tile);

        if (di->fire_probability > 0)
        {
            // This tile is burnable
            fire_x = x;
            fire_y = y;
            break;
        }
    }

    if ((fire_x == -1) || (fire_y == -1))
    {
        // Well, it seems that we couldn't find a valid starting point. Someone
        // has been lucky... :)
        return;
    }

    MapDeleteBuildingFire(fire_x, fire_y);

    // TODO: MessageRequestAdd(ID_MSG_FIRE_INITED)

    int scx = fire_x - ((GBA_SCREEN_W / 2) / 8);
    if (scx < 0)
        scx = 0;

    int scy = fire_y - ((GBA_SCREEN_H / 2) / 8);
    if (scy < 0)
        scy = 0;

    Room_Game_Request_Scroll(scx, scy);

    // Remove all traffic tiles from the map, as well as other animations
    // ------------------------------------------------------------------

    Simulation_TrafficRemoveAnimationTiles();

    // TODO: Simulation_TransportAnimsHide()

    // Enable disaster mode
    // --------------------

    Room_Game_SetDisasterMode(1);
}

static void Simulation_FireExpand(int x, int y)
{
    if (y > 0) // Expand up
    {
        int tx = x;
        int ty = y - 1;

        uint16_t tile = CityMapGetTile(tx, ty);

        const city_tile_density_info *di = CityTileDensityInfo(tile);
        if (di->fire_probability > 0)
        {
            int val = fire_map[ty * CITY_MAP_WIDTH + tx];
            val += di->fire_probability;
            if (val > 255)
                val = 255;
            fire_map[ty * CITY_MAP_WIDTH + tx] = val;
        }
    }

    if (y < (CITY_MAP_HEIGHT - 1)) // Expand down
    {
        int tx = x;
        int ty = y + 1;

        uint16_t tile = CityMapGetTile(tx, ty);

        const city_tile_density_info *di = CityTileDensityInfo(tile);
        if (di->fire_probability > 0)
        {
            int val = fire_map[ty * CITY_MAP_WIDTH + tx];
            val += di->fire_probability;
            if (val > 255)
                val = 255;
            fire_map[ty * CITY_MAP_WIDTH + tx] = val;
        }
    }

    if (x > 0) // Expand left
    {
        int tx = x - 1;
        int ty = y;

        uint16_t tile = CityMapGetTile(tx, ty);

        const city_tile_density_info *di = CityTileDensityInfo(tile);
        if (di->fire_probability > 0)
        {
            int val = fire_map[ty * CITY_MAP_WIDTH + tx];
            val += di->fire_probability;
            if (val > 255)
                val = 255;
            fire_map[ty * CITY_MAP_WIDTH + tx] = val;
        }
    }

    if (x < (CITY_MAP_WIDTH - 1)) // Expand right
    {
        int tx = x + 1;
        int ty = y;

        uint16_t tile = CityMapGetTile(tx, ty);

        const city_tile_density_info *di = CityTileDensityInfo(tile);
        if (di->fire_probability > 0)
        {
            int val = fire_map[ty * CITY_MAP_WIDTH + tx];
            val += di->fire_probability;
            if (val > 255)
                val = 255;
            fire_map[ty * CITY_MAP_WIDTH + tx] = val;
        }
    }
}

void Simulation_Fire(void)
{
    // This should only be called during disaster mode!

    // Clear
    // -----

    // Each tile can receive fire from the 4 neighbours. In this bank the code
    // adds the probabilities of the tile to catch fire as many times as needed
    // (e.g. 2 neighbours with fire, 2 x probabilities). Afterwards, a random
    // number is generated for each tile and if it is lower the tile catches
    // fire or not.

    memset(fire_map, 0, sizeof(fire_map));

    // For each tile check if it is type TYPE_FIRE and try to expand fire
    // ------------------------------------------------------------------

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t type = CityMapGetType(i, j);
            if (type == TYPE_FIRE)
                Simulation_FireExpand(i, j);
        }
    }

    // Remove fire
    // -----------

    // Calculate probability of the fire in a tile being extinguished

    // Add one fire station so that fire can end even with no fire stations.
    int extinguish_fire_probability = (initial_number_fire_stations + 1) * 2;
    if (extinguish_fire_probability > 255)
        extinguish_fire_probability = 255;

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t type = CityMapGetType(i, j);
            if (type != TYPE_FIRE)
                continue;

            int r = rand() & 0xFF;
            if (r < extinguish_fire_probability)
                CityMapDrawTile(T_DEMOLISHED, i, j);
        }
    }

    // Place fire wherever it was flagged in the previous loop
    // -------------------------------------------------------

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            int val = fire_map[j * CITY_MAP_WIDTH + i];

            if (val == 0)
                continue;

            int r = rand() & 0xFF;
            if (r < val)
                MapDeleteBuildingFire(i, j);
        }
    }

    // Check if there is fire or not. If not, go back to non-disaster mode
    // -------------------------------------------------------------------

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t type = CityMapGetType(i, j);
            if (type == TYPE_FIRE)
                return;
        }
    }

    // If not found fire, go back to normal mode

    Room_Game_SetDisasterMode(0);

    // A fire may have destroyed buildings, we need to refresh the counts
    Simulation_CountBuildings();

    // Force reset of all planes to new coordinates
    // TODO: Simulation_TransportAnimsInit()
    // TODO: Simulation_TransportAnimsShow()
}

void Simulation_FireAnimate(void)
{
    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t tile = CityMapGetTile(i, j);
            if (tile == T_FIRE_1)
                CityMapDrawTile(T_FIRE_2, i, j);
            else if (tile == T_FIRE_2)
                CityMapDrawTile(T_FIRE_1, i, j);
        }
    }
}
