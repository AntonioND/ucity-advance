// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <stdint.h>
#include <string.h>

#include <ugba/ugba.h>

#include "room_game/building_info.h"
#include "room_game/draw_common.h"
#include "room_game/room_game.h"
#include "room_game/text_messages.h"
#include "room_game/tileset_info.h"
#include "simulation/building_density.h"
#include "simulation/simulation_happiness.h"
#include "simulation/simulation_pollution.h"
#include "simulation/simulation_traffic.h"

// Max valid pollution for zones that need non-polluted air
#define POLLUTION_MAX_VALID_LEVEL   (256 / 2)

EWRAM_BSS uint8_t scratch_map_1[CITY_MAP_WIDTH * CITY_MAP_HEIGHT];
EWRAM_BSS uint8_t scratch_map_2[CITY_MAP_WIDTH * CITY_MAP_HEIGHT];

// Total pollution in the city. Max value = 255 * 64 * 64
int pollution_total;

// Percentage of pollution
int pollution_total_percent;

int Simulation_PollutionGetTotal(void)
{
    return pollution_total;
}

uint8_t *Simulation_PollutionGetMap(void)
{
    return &scratch_map_1[0];
}

static void Diffuminate_Central_Tile(uint8_t *src, uint8_t *dst, int i, int j)
{
    uint32_t total = 0;
    uint32_t weight = 5;
    uint32_t result;

    if (j > 0) // Top row
    {
        total += (uint32_t)src[(j - 1) * CITY_MAP_WIDTH + i];
        weight--;
    }

    if (j < (CITY_MAP_HEIGHT - 1)) // Bottom row
    {
        total += (uint32_t)src[(j + 1) * CITY_MAP_WIDTH + i];
        weight--;
    }

    if (i > 0) // Left column
    {
        total += (uint32_t)src[j * CITY_MAP_WIDTH + (i - 1)];
        weight--;
    }

    if (i < (CITY_MAP_WIDTH - 1)) // Right column
    {
        total += (uint32_t)src[j * CITY_MAP_WIDTH + (i + 1)];
        weight--;
    }

    total += weight * (uint32_t)src[j * CITY_MAP_WIDTH + i];

    // Saturate

    if (total > (255 * 3))
        result = 255;
    else
        result = total / 3;

    dst[j * CITY_MAP_WIDTH + i] = result;
}

static void Diffuminate_Loop(uint8_t *src, uint8_t *dst)
{
    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
            Diffuminate_Central_Tile(src, dst, i, j);
    }
}

static void Simulation_PollutionSetTileOkFlag(void)
{
    // List of terrains that ignore the pollution level. In general, any terrain
    // that generates pollution ignores it. This is only used for buildings, so
    // no need to check fields, forests or water zones.
    //
    // This array says whether a particular tile type has to be checked for
    // pollution or not. Type flags should be removed before accesing it.
    //
    // 1 = ignore this tile, 0 = handle pollution
    const uint8_t ignore_tile_array[] = {
        [TYPE_FIELD] = 1, // There's nothing here, don't check...
        [TYPE_FOREST] = 1,
        [TYPE_WATER] = 1,
        [TYPE_RESIDENTIAL] = 0, // R and C must be clean. I generates pollution.
        [TYPE_INDUSTRIAL] = 1,
        [TYPE_COMMERCIAL] = 0,
        [TYPE_POLICE_DEPT] = 1, // Services are supposed to work even in very
        [TYPE_FIRE_DEPT] = 1,   //  polluted areas.
        [TYPE_HOSPITAL] = 1,
        [TYPE_PARK] = 0, // Recreation and education, they must be clean.
        [TYPE_STADIUM] = 0,
        [TYPE_SCHOOL] = 0,
        [TYPE_HIGH_SCHOOL] = 0,
        [TYPE_UNIVERSITY] = 0,
        [TYPE_MUSEUM] = 0,
        [TYPE_LIBRARY] = 0,
        [TYPE_AIRPORT] = 1, // The tiles below generate pollution, so it's
        [TYPE_PORT] = 1,    // illogical to ask for no pollution there.
        [TYPE_DOCK] = 1,
        [TYPE_POWER_PLANT] = 1,
        [TYPE_FIRE] = 0, // Simulation should be off during fires.
        [TYPE_RADIATION] = 1, // Ignore pollution here
    };

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t type = CityMapGetType(i, j) & TYPE_MASK;

            if (ignore_tile_array[type])
            {
                // This tile ignores pollution - Set "valid pollution level" bit
                Simulation_HappinessSetFlags(i, j, TILE_OK_POLLUTION);
            }
            else
            {
                // Buildings require a level check...
                unsigned int pollution = scratch_map_1[j * CITY_MAP_WIDTH + i];
                if (pollution > POLLUTION_MAX_VALID_LEVEL)
                {
                    // Polluted
                    Simulation_HappinessResetFlags(i, j, TILE_OK_POLLUTION);
                }
                else
                {
                    // Not polluted
                    Simulation_HappinessSetFlags(i, j, TILE_OK_POLLUTION);
                }
            }
        }
    }
}

void Simulation_Pollution(void)
{
    // Cleanup
    // -------

    // It isn't needed to clean the second map, it is overwriten

    memset(scratch_map_1, 0, sizeof(scratch_map_1));

    pollution_total = 0;

    // Add to the map the corresponding pollution for each tile

    uint8_t *traffic_map = Simulation_TrafficGetMap();

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            // Get tile type:
            //
            // - If road, check traffic. Train doesn't pollute as it is
            //   electric.
            //
            // - If building, check if the building has power and add pollution
            //   if so. If it is a power plant, add the corresponding pollution
            //   level.
            //
            // - If park, forest or water set a neutral level of pollution
            //   (they reduce it)

            uint16_t tile, type;
            CityMapGetTypeAndTileUnsafe(i, j, &tile, &type);

            int value;

            if (type & TYPE_HAS_ROAD)
            {
                // Pollution is the amount of cars going through here
                value = traffic_map[j * CITY_MAP_WIDTH + i];
            }
            else
            {
                // Read pollution level array
                const city_tile_density_info *di = CityTileDensityInfo(tile);
                value = di->pollution_level;
            }

            scratch_map_1[j * CITY_MAP_WIDTH + i] = value;

            // Add to total pollution

            pollution_total += value;
        }
    }

    // Diffuminate map
    // ----------

    Diffuminate_Loop(&scratch_map_1[0], &scratch_map_2[0]);
    Diffuminate_Loop(&scratch_map_2[0], &scratch_map_1[0]);
    Diffuminate_Loop(&scratch_map_1[0], &scratch_map_2[0]);
    Diffuminate_Loop(&scratch_map_2[0], &scratch_map_1[0]);

    // Check if pollution is too high
    // ------------------------------

    // Max value = 255 * 64 * 64  = 0x0FF000
    // Complain if pollution >= 0x030000

    const int pollution_max = 255 * CITY_MAP_WIDTH * CITY_MAP_HEIGHT;

    pollution_total_percent = (pollution_total * 100) / pollution_max;

    // Check if we need to show a message
    if (pollution_total > 0x030000)
    {
        // This message is shown only once per year
        PersistentMessageShow(ID_MSG_POLLUTION_HIGH);
    }

    // Set flags
    // ---------

    Simulation_PollutionSetTileOkFlag();
}
