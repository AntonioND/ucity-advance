// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2017-2019, 2021, Antonio Niño Díaz

#include <stdint.h>

#include <ugba/ugba.h>

#include "room_game/status_bar.h"

// Assets

#include "maps/status_bar/text_palette_bin.h"
#include "maps/status_bar/text_tiles_bin.h"

static int status_bar_position = STATUS_BAR_UP;
static int status_bar_hidden = 1;

void StatusBarLoad(void)
{
    // Load the palettes
    VRAM_BGPalette16Copy(text_palette_bin, text_palette_bin_size, TEXT_PALETTE);

    // Load the tiles
    SWI_CpuSet_Copy16(text_tiles_bin, (void *)TEXT_TILES_BASE,
                      text_tiles_bin_size);

    // Load the map

    uint32_t zero = 0;
    SWI_CpuSet_Fill32(&zero, (void *)TEXT_MAP_BASE, 32 * 32 * sizeof(uint16_t));

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
    // Setup background
    BG_RegularInit(1, BG_REGULAR_256x256, BG_16_COLORS,
                   TEXT_TILES_BASE, TEXT_MAP_BASE);

    if (status_bar_hidden)
    {
        BG_RegularScrollSet(1, 0, 0);
    }
    else
    {
        if (status_bar_position == STATUS_BAR_UP)
            BG_RegularScrollSet(1, 0, -16);
        else if (status_bar_position == STATUS_BAR_DOWN)
            BG_RegularScrollSet(1, 0, -GBA_SCREEN_H);
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
    uintptr_t addr = TEXT_MAP_BASE + ((y + 30) * 32 + x) * 2;

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
