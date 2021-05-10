// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <stdint.h>

#include <ugba/ugba.h>

#include "room_game/status_bar.h"

#define MENU_MAP_BASE       (TEXT_MAP_BASE + (32 * 32 * 2))

void PauseMenuLoad(void)
{
    // Load the map

    // TODO: Load actual map

    uint32_t fill = ' ';
    fill = (fill << 24) | (fill << 16) | (fill << 8) | (fill << 0);
    int height = (GBA_SCREEN_H / 8) - 2;
    SWI_CpuSet_Fill32(&fill, (void *)MENU_MAP_BASE,
                      32 * height * sizeof(uint16_t));

    // Setup background

    BG_RegularScrollSet(1, 0, 256 - 16);

    BG_RegularInit(1, BG_REGULAR_256x512, BG_16_COLORS,
                   TEXT_TILES_BASE, TEXT_MAP_BASE);
}
