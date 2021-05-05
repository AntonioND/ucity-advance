// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef SIMULATION_BUILDING_DENSITY_H__
#define SIMULATION_BUILDING_DENSITY_H__

#include <stdint.h>

// Values refer to the top left corner of a building
typedef struct {
    uint8_t population;
    uint8_t energy_cost;
    uint8_t pollution_level;
    uint8_t fire_probability;
} city_tile_density_info;

// Only the top left tiles of each building has information
city_tile_density_info *CityTileDensityInfo(uint16_t tile);

#endif // SIMULATION_BUILDING_DENSITY_H__
