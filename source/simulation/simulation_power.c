// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <stdint.h>
#include <string.h>

#include <ugba/ugba.h>

#include "date.h"
#include "room_game/building_info.h"
#include "room_game/draw_common.h"
#include "room_game/draw_power_lines.h"
#include "room_game/room_game.h"
#include "room_game/tileset_info.h"
#include "simulation/building_density.h"
#include "simulation/queue.h"
#include "simulation/simulation_happiness.h"

#define TILE_HANDLED_BIT                7
#define TILE_HANDLED_POWER_PLANT_BIT    6

#define TILE_HANDLED                    (1 << TILE_HANDLED_BIT)
#define TILE_HANDLED_POWER_PLANT        (1 << TILE_HANDLED_POWER_PLANT_BIT)
// How much power there is now
#define TILE_POWER_LEVEL_MASK           (0x3F)

EWRAM_BSS static uint8_t power_map[CITY_MAP_HEIGHT * CITY_MAP_WIDTH];

uint8_t *Simulation_PowerDistributionGetMap(void)
{
    return &power_map[0];
}

static int power_plant_energy_left;

// Give as much energy as possible to the specified tile
static void AddPowerToTile(int x, int y)
{
    // If this is a power plant, flag as handled and return right away
    if (power_map[y * CITY_MAP_WIDTH + x] & TILE_HANDLED_POWER_PLANT)
    {
        power_map[y * CITY_MAP_WIDTH + x] |= TILE_HANDLED;
        return;
    }

    // If not, give power. All the tiles of one building have the same cost, but
    // only the top left one has the information about the cost.

    uint16_t tile = CityMapGetTile(x, y);
    const city_tile_info *tile_info = City_Tileset_Entry_Info(tile);

    if ((tile_info->base_x_delta != 0) || (tile_info->base_y_delta != 0))
    {
        tile = CityMapGetTile(x + tile_info->base_x_delta,
                              y + tile_info->base_y_delta);
    }

    const city_tile_density_info *info = CityTileDensityInfo(tile);

    uint8_t *ptr = &power_map[y * CITY_MAP_WIDTH + x];

    int current_energy = *ptr & TILE_POWER_LEVEL_MASK;
    int needed_energy = info->energy_cost - current_energy;

    UGBA_Assert(needed_energy > 0);

    int consumed_energy;

    if (needed_energy > power_plant_energy_left)
    {
        consumed_energy = power_plant_energy_left;
    }
    else
    {
        consumed_energy = needed_energy;
        Simulation_HappinessSetFlags(x, y, TILE_OK_POWER);
    }

    power_plant_energy_left -= consumed_energy;

    // Add to tile energy

    *ptr = ((*ptr + consumed_energy) & TILE_POWER_LEVEL_MASK) | TILE_HANDLED;
}

static void AddToQueueHorizontalDisplacement(int x, int y)
{
    if ((x < 0) || (x >= CITY_MAP_WIDTH))
        return;

    // Check if already handled
    if (power_map[y * CITY_MAP_WIDTH + x] & TILE_HANDLED)
        return;

    uint16_t tile, type;
    CityMapGetTypeAndTileUnsafe(x, y, &tile, &type);

    // Check if it transmits power
    if ((TypeHasElectricityExtended(type) & TYPE_HAS_POWER) == 0)
        return;

    // Check if it is a bridge with incorrect orientation
    // Return if vertical bridge!

    if (tile == T_POWER_LINES_TB_BRIDGE)
        return;

    // Add to queue!
    QueueAddPair(x, y);
}

static void AddToQueueVerticalDisplacement(int x, int y)
{
    if ((y < 0) || (y >= CITY_MAP_HEIGHT))
        return;

    // Check if already handled
    if (power_map[y * CITY_MAP_WIDTH + x] & TILE_HANDLED)
        return;

    uint16_t tile, type;
    CityMapGetTypeAndTileUnsafe(x, y, &tile, &type);

    // Check if it transmits power
    if ((TypeHasElectricityExtended(type) & TYPE_HAS_POWER) == 0)
        return;

    // Check if it is a bridge with incorrect orientation
    // Return if horizontal bridge!

    if (tile == T_POWER_LINES_LR_BRIDGE)
        return;

    // Add to queue!
    QueueAddPair(x, y);
}

// Flood fill from the power plant on the specified coordinates. It also needs
// the delta to reach the center of the power plant.
static void Simulation_PowerPlantFloodFill(uint16_t base_tile, int x, int y,
                                           int dx, int dy, int power)
{
    // Reset all TILE_HANDLED flags

    for (int i = 0; i < CITY_MAP_HEIGHT * CITY_MAP_WIDTH; i++)
        power_map[i] &= ~TILE_HANDLED;

    // Flag power plant as handled
    //
    // This is faster than setting the power of all other tiles of the central
    // to have power 0 because the TILE_HANDLED flag doesn't have to be cleared
    // this way.

    const building_info *info = Get_BuildingFromBaseTile(base_tile);

    for (int j = y; j < (y + info->height); j++)
    {
        for (int i = x; i < (x + info->width); i++)
            power_map[j * CITY_MAP_WIDTH + i] |= TILE_HANDLED_POWER_PLANT;
    }

    power_plant_energy_left = power;

    // Flood fill

    // For each connected tile with scratch RAM value of 0 reduce the fill
    // amount of the power plant by the energy consumption of that tile (if
    // possible) and add the energy given to that tile to the scratch RAM. Power
    // lines have energetic cost.

    QueueInit();

    QueueAddPair(x + dx, y + dy); // Add first element

    while (1)
    {
        // Check remaining power plant energy. If 0, exit loop.
        if (power_plant_energy_left == 0)
            break;

        // Check if queue is empty. If so, exit loop
        if (QueueIsEmpty())
            break;

        // 1) Get Queue element.

        uint16_t ex, ey;
        QueueGetPair(&ex, &ey);

        // 2) If not already handled by this plant, try to fill current
        //    coordinates.

        if (power_map[ey * CITY_MAP_WIDTH + ex] & TILE_HANDLED)
            continue; // Already handled by this power plant, ignore

        // Not handled. Get energy consumption of the tile and give as much
        // energy as needed. If there is not enough energy left for that, give
        // as much as possible, flag as handled and exit loop next iteration (at
        // the check at the top of the loop).

        AddPowerToTile(ex, ey);

        // 3) Add to queue all valid neighbours (power plants, buildings, lines)

        // If this is a vertical bridge only try to power top and bottom. If it
        // is horizontal, only left and right!

        uint16_t etile = CityMapGetTile(ex, ey);

        // If not horizontal bridge, check top and bottom
        if (etile != T_POWER_LINES_LR_BRIDGE)
        {
            AddToQueueVerticalDisplacement(ex, ey - 1);
            AddToQueueVerticalDisplacement(ex, ey + 1);
        }

        // If not vertical bridge, check left and right
        if (etile != T_POWER_LINES_TB_BRIDGE)
        {
            AddToQueueHorizontalDisplacement(ex - 1, ey);
            AddToQueueHorizontalDisplacement(ex + 1, ey);
        }
    }
}

void Simulation_PowerDistribution(void)
{
    memset(power_map, 0, sizeof(power_map));

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
            Simulation_HappinessResetFlags(i, j, TILE_OK_POWER);
    }

    int month = DateGetMonth();

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t tile = CityMapGetTile(i, j);

            // All power plants have fluctuations in power during the year:
            //
            // 1. Solar plants output more power in summer, wind plants in
            //    winter.
            //
            // 2. The others depend on thermodynamic cycles, which are more
            //    efficient in winter, when the external temperature is lower.

            switch (tile)
            {
                case T_POWER_PLANT_COAL:
                {
                    const int power[12] = {
                        3000, 2950, 2900, 2850, 2800, 2750, // Jan - Jun
                        2750, 2800, 2850, 2900, 2950, 3000  // Jul - Dec
                    };
                    Simulation_PowerPlantFloodFill(T_POWER_PLANT_COAL, i, j,
                                                   1, 1, power[month]);
                    break;
                }
                case T_POWER_PLANT_OIL:
                {
                    const int power[12] = {
                        2000, 1950, 1900, 1850, 1800, 1750, // Jan - Jun
                        1750, 1800, 1850, 1900, 1950, 2000 // Jul - Dec
                    };
                    Simulation_PowerPlantFloodFill(T_POWER_PLANT_OIL, i, j,
                                                   1, 1, power[month]);
                    break;
                }
                case T_POWER_PLANT_WIND:
                {
                    const int power[12] = {
                        200, 180, 160, 140, 120, 100, // Jan - Jun
                        100, 120, 140, 160, 180, 200  // Jul - Dec
                    };
                    Simulation_PowerPlantFloodFill(T_POWER_PLANT_WIND, i, j,
                                                   0, 0, power[month]);
                    break;
                }
                case T_POWER_PLANT_SOLAR:
                {
                    const int power[12] = {
                        1000, 1200, 1400, 1600, 1800, 2000, // Jan - Jun
                        2000, 1800, 1600, 1400, 1200, 1000  // Jul - Dec
                    };
                    Simulation_PowerPlantFloodFill(T_POWER_PLANT_SOLAR, i, j,
                                                   1, 1, power[month]);
                    break;
                }
                case T_POWER_PLANT_NUCLEAR:
                {
                    const int power[12] = {
                        5000, 4950, 4900, 4850, 4800, 4750, // Jan - Jun
                        4750, 4800, 4850, 4900, 4950, 5000  // Jul - Dec
                    };
                    Simulation_PowerPlantFloodFill(T_POWER_PLANT_NUCLEAR, i, j,
                                                   1, 1, power[month]);
                    break;
                }
                case T_POWER_PLANT_FUSION:
                {
                    const int power[12] = {
                        10000, 9500, 9000, 8500, 8000, 7500, // Jan - Jun
                        7500, 8000, 8500, 9000, 9500, 10000  // Jul - Dec
                    };
                    Simulation_PowerPlantFloodFill(T_POWER_PLANT_FUSION, i, j,
                                                   1, 1, power[month]);
                    break;
                }
                default:
                    break;
            }
        }
    }

    // Reset all remaining flags

    for (int i = 0; i < CITY_MAP_HEIGHT * CITY_MAP_WIDTH; i++)
        power_map[i] &= TILE_POWER_LEVEL_MASK;

    // Checks all tiles of this building and flags them as "not powered" unless
    // all of them are powered.

    uint8_t *happiness_map = Simulation_HappinessGetMap();

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; )
        {
            uint16_t tile, type;
            CityMapGetTypeAndTileUnsafe(i, j, &tile, &type);

            // If this isn't a powered tile, skip it
            if ((TypeHasElectricityExtended(type) & TYPE_HAS_POWER) == 0)
            {
                i++;
                continue;
            }

            // Only check top left corner of each building
            if (BuildingIsCoordinateOrigin(tile) == 0)
            {
                i++;
                continue;
            }

            const building_info *info = Get_BuildingFromBaseTile(tile);

            int total = info->height * info->width;
            int count = 0;

            for (int y = j; y < (j + info->height); y++)
            {
                for (int x = i; x < (i + info->width); x++)
                {
                    if (happiness_map[y * CITY_MAP_WIDTH + x] & TILE_OK_POWER)
                        count++;
                }
            }

            if ((count > 0) && (count < total))
            {
                // If any of the tiles is powered, but not all of them are,
                // reset them all.

                for (int y = j; y < (j + info->height); y++)
                {
                    for (int x = i; x < (i + info->width); x++)
                        happiness_map[y * CITY_MAP_WIDTH + x] &= ~TILE_OK_POWER;
                }
            }

            // Skip rest of the width of this building
            i += info->width;
        }
    }
}
