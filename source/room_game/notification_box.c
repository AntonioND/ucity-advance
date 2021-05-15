// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <ugba/ugba.h>

#include "room_game/status_bar.h"

// Assets

#include "maps/notification_bg.h"

void Notification_Box_Show(void)
{
    BG_RegularScrollSet(0, 0, -32);
}

void Notification_Box_Hide(void)
{
    BG_RegularScrollSet(0, 0, 64);
}

void Notification_Box_Load(void)
{
#define NOTIFICATION_BOX_MAP_BASE   MEM_BG_MAP_BLOCK_ADDR(31)

    uint16_t *src = (uint16_t *)notification_bg_map;
    uint16_t *dst = (uint16_t *)NOTIFICATION_BOX_MAP_BASE;

    for (int i = 0; i < 32 * 32; i++)
    {
        *dst = MAP_REGULAR_TILE(*src) | MAP_REGULAR_PALETTE(TEXT_PALETTE);
        src++;
        dst++;
    }

    Notification_Box_Hide();

    // Setup background
    BG_RegularInit(0, BG_REGULAR_256x256, BG_16_COLORS,
                   TEXT_TILES_BASE, NOTIFICATION_BOX_MAP_BASE);
}
