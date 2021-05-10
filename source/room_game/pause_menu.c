// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <stdint.h>

#include <ugba/ugba.h>

#include "room_game/status_bar.h"

// Assets

#include "maps/pause_menu_bg.h"

#define MENU_MAP_BASE       (TEXT_MAP_BASE + (32 * 32 * 2))

void PauseMenuLoad(void)
{
    // Load the map

    for (size_t i = 0; i < (pause_menu_bg_map_size / 2); i++)
    {
        const uint16_t *src = (const uint16_t *)pause_menu_bg_map;
        uint16_t *dst = (void *)MENU_MAP_BASE;

        dst[i] =  MAP_REGULAR_TILE(src[i]) | MAP_REGULAR_PALETTE(TEXT_PALETTE);
    }

    // Setup background

    BG_RegularScrollSet(1, 0, 256 - 16);

    BG_RegularInit(1, BG_REGULAR_256x512, BG_16_COLORS,
                   TEXT_TILES_BASE, TEXT_MAP_BASE);
}
