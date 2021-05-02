// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <stdint.h>
#include <string.h>

#include <ugba/ugba.h>

#include "simulation/simulation_happiness.h"
#include "room_game/room_game.h"

EWRAM_BSS static uint8_t happiness_map[CITY_MAP_HEIGHT * CITY_MAP_WIDTH];

uint8_t *Simulation_HappinessGetMap(void)
{
    return &happiness_map[0];
}

uint8_t Simulation_HappinessGetFlags(int x, int y)
{
    return happiness_map[y * CITY_MAP_WIDTH + x];
}

void Simulation_HappinessSetFlags(int x, int y, uint8_t flags)
{
    happiness_map[y * CITY_MAP_WIDTH + x] |= flags;
}

void Simulation_HappinessResetFlags(int x, int y, uint8_t flags)
{
    happiness_map[y * CITY_MAP_WIDTH + x] &= ~flags;
}

void Simulation_HappinessResetMap(void)
{
    memset(happiness_map, 0, sizeof(happiness_map));
}
