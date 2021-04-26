// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2017-2019, 2021, Antonio Niño Díaz

#include <stdint.h>

#include <ugba/ugba.h>

#include "room_game/status_bar.h"

// Assets

#include "graphics/text.h"

#define TEXT_PALETTE        (14)
#define TEXT_TILES_BASE     MEM_BG_TILES_BLOCK_ADDR(2)
#define TEXT_MAP_BASE       MEM_BG_MAP_BLOCK_ADDR(20)

static int status_bar_position = STATUS_BAR_UP;
static int status_bar_hidden = 1;

void StatusBarLoad(void)
{
    // Load the palettes
    VRAM_BGPalette16Copy(textPal, textPalLen, TEXT_PALETTE);

    // Load the tiles
    SWI_CpuSet_Copy16(textTiles, (void *)TEXT_TILES_BASE, textTilesLen);

    // Load the map
    StatusBarPrint(0, 0, "                              ");
    StatusBarPrint(0, 1, "                              ");

    // Setup background
    BG_RegularInit(1, BG_REGULAR_256x256, BG_16_COLORS,
                   TEXT_TILES_BASE, TEXT_MAP_BASE);

    status_bar_position = STATUS_BAR_DOWN;
    status_bar_hidden = 1;
}

void StatusBarClear(void)
{
    StatusBarPrint(0, 0, "                              ");
    StatusBarPrint(0, 1, "                              ");
}

static void StatusBarRefresh(void)
{
    if (status_bar_hidden)
    {
        BG_RegularScrollSet(1, 0, 16);
    }
    else
    {
        if (status_bar_position == STATUS_BAR_UP)
            BG_RegularScrollSet(1, 0, 0);
        else if (status_bar_position == STATUS_BAR_DOWN)
            BG_RegularScrollSet(1, 0, -(GBA_SCREEN_H - 16));
    }
}

void StatusBarPositionSet(int position)
{
    status_bar_position = position;
    StatusBarRefresh();
}

int StatusBarPositionGet(void)
{
    return status_bar_position;
}

void StatusBarShow(void)
{
    status_bar_hidden = 0;
    StatusBarRefresh();
}

void StatusBarHide(void)
{
    status_bar_hidden = 1;
    StatusBarRefresh();
}

void StatusBarPrint(int x, int y, const char *text)
{
    uintptr_t addr = TEXT_MAP_BASE + (y * 32 + x) * 2;

    while (1)
    {
        int c = (uint8_t)*text++;
        if (c == '\0')
            break;

        uint16_t *ptr = (uint16_t *)addr;

        *ptr = MAP_REGULAR_TILE(c) | MAP_REGULAR_PALETTE(TEXT_PALETTE);

        addr += 2;
    }
}
