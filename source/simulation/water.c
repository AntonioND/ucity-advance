// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <stdlib.h>

#include "random.h"
#include "room_game/draw_common.h"
#include "room_game/room_game.h"
#include "room_game/tileset_info.h"

void Simulation_WaterAnimate(void)
{
    int count = (rand_fast() & 31) + 1;

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            if (count > 0)
            {
                count--;
                continue;
            }

            count = (rand_fast() & 31) + 1;

            uint16_t tile = CityMapGetTile(i, j);
            if (tile == T_WATER)
                CityMapDrawTile(T_WATER_EXTRA, i, j);
            else if (tile == T_WATER_EXTRA)
                CityMapDrawTile(T_WATER, i, j);
        }
    }
}
