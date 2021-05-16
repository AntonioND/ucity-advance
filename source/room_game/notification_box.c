// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <ugba/ugba.h>

#include "room_game/status_bar.h"

// Assets

#include "maps/notification_bg.h"

#define NOTIFICATION_BOX_MAP_BASE   MEM_BG_MAP_BLOCK_ADDR(31)

void Notification_Box_Show(void)
{
    BG_RegularScrollSet(0, 0, -(8 * 7));
}

void Notification_Box_Hide(void)
{
    BG_RegularScrollSet(0, 0, 8 * 8);
}

void Notification_Box_Clear(void)
{
    uint16_t *src = (uint16_t *)notification_bg_map;
    uint16_t *dst = (uint16_t *)NOTIFICATION_BOX_MAP_BASE;

    for (int i = 0; i < 32 * 32; i++)
    {
        *dst = MAP_REGULAR_TILE(*src) | MAP_REGULAR_PALETTE(TEXT_PALETTE);
        src++;
        dst++;
    }
}

void Notification_Box_Load(void)
{
    Notification_Box_Clear();

    Notification_Box_Hide();

    // Setup background
    BG_RegularInit(0, BG_REGULAR_256x256, BG_16_COLORS,
                   TEXT_TILES_BASE, NOTIFICATION_BOX_MAP_BASE);
}

void Notification_Box_Print(int x, int y, const char *text)
{
    const int minx = 4;
    const int miny = 1;
    const int maxx = 25;
    const int maxy = 4;

    x += minx;
    y += miny;

    while (1)
    {
        int c = (uint8_t)*text++;
        if (c == '\0')
        {
            return;
        }
        else if (c == '\n')
        {
            x = minx;
            y++;
        }
        else
        {
            uintptr_t addr = NOTIFICATION_BOX_MAP_BASE + (y * 32 + x) * 2;

            uint16_t *ptr = (uint16_t *)addr;

            *ptr = MAP_REGULAR_TILE(c) | MAP_REGULAR_PALETTE(TEXT_PALETTE);

            x++;
            if (x > maxx)
            {
                x = minx;
                y++;
            }
        }

        if (y == maxy)
            return;
    }
}
