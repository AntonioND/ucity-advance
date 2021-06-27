// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2017-2019, 2021, Antonio Niño Díaz

#include <stdint.h>
#include <string.h>

#include "room_game/draw_common.h"
#include "room_game/room_game.h"
#include "room_game/tileset_info.h"

typedef struct {
    uint32_t airports;
    uint32_t ports;
    uint32_t docks;
    uint32_t fire_stations;
    uint32_t nuclear_power_plants;
    uint32_t universities;
    uint32_t stadiums;
    uint32_t museums;
    uint32_t libraries;

    uint32_t roads;
    uint32_t train_tracks;
} building_count_info;

static building_count_info building_count;

building_count_info *Simulation_CountBuildingsGet(void)
{
    return &building_count;
}

// Call this function whenever a building is built or demolished. For example, it
// has to be called after exiting edit mode, after a fire is finally extinguished
// or simply when the map is loaded.
void Simulation_CountBuildings(void)
{
    memset(&building_count, 0, sizeof(building_count));

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t tile, type;
            CityMapGetTypeAndTileUnsafe(i, j, &tile, &type);

            // Count number of buildings. Check top left tiles

            if (tile == T_AIRPORT)
            {
                building_count.airports++;
            }
            else if (tile == T_FIRE_DEPT)
            {
                building_count.fire_stations++;
            }
            else if (tile == T_PORT)
            {
                building_count.ports++;
            }
            else if ((tile == T_PORT_WATER_L) || (tile == T_PORT_WATER_R) ||
                     (tile == T_PORT_WATER_D) || (tile == T_PORT_WATER_U))
            {
                building_count.docks++;
            }
            else if (tile == T_POWER_PLANT_NUCLEAR)
            {
                building_count.nuclear_power_plants++;
            }
            else if (tile == T_UNIVERSITY)
            {
                building_count.universities++;
            }
            else if (tile == T_STADIUM)
            {
                building_count.stadiums++;
            }
            else if (tile == T_MUSEUM)
            {
                building_count.museums++;
            }
            else if (tile == T_LIBRARY)
            {
                building_count.libraries++;
            }
            else
            {
                // Count the number of roads and train tracks
                if (type & TYPE_HAS_ROAD)
                    building_count.roads++;
                if (type & TYPE_HAS_TRAIN)
                    building_count.train_tracks++;
            }
        }
    }
}
