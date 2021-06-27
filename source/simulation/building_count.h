// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2017-2019, 2021, Antonio Niño Díaz

#ifndef SIMULATION_BUILDING_COUNT_H__
#define SIMULATION_BUILDING_COUNT_H__

#include <stdint.h>

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

building_count_info *Simulation_CountBuildingsGet(void);
void Simulation_CountBuildings(void);

#endif // SIMULATION_BUILDING_COUNT_H__
