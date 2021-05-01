// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <stdint.h>

#include "room_game/draw_common.h"
#include "room_game/room_game.h"

//EWRAM_BSS
static uint8_t type_matrix[CITY_MAP_WIDTH * CITY_MAP_HEIGHT];

void TypeMatrixRefresh(void)
{
    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t type = CityMapGetType(i, j);
            type_matrix[j * CITY_MAP_WIDTH + i] = type;
        }
    }
}

uint8_t *TypeMatrixGet(void)
{
    return &type_matrix[0];
}

#if 0
void Simulation_SimulateAll(void)
{
    Simulation_Services(T_POLICE_DEPT_CENTER);
}
#endif
