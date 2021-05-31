// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <ugba/ugba.h>

// Assets

#include "sprites/cursor_palette_bin.h"
#include "sprites/cursor_tiles_bin.h"

static int curx = (GBA_SCREEN_W / 2) - 8;
static int cury = (GBA_SCREEN_H / 2) - 8;
static int curw = 8;
static int curh = 8;
static int curframe = 0;
static int cursor_tiles_index = 0;
static int cursor_palette = 0;

void Cursor_Reset_Position(void)
{
    int offsetx = curw / 2;
    if ((offsetx % 8) != 0)
        offsetx += 4;

    int offsety = curh / 2;
    if ((offsety % 8) != 0)
        offsety += 4;

    curx = (GBA_SCREEN_W / 2) - offsetx;
    cury = (GBA_SCREEN_H / 2) - offsety;
}

void Cursor_Set_Position(int x, int y)
{
    curx = x;
    cury = y;
}

void Cursor_Get_Position(int *x, int *y)
{
    *x = curx;
    *y = cury;
}

void Cursor_Hide(void)
{
    OBJ_RegularInit(120, 0, 200, OBJ_SIZE_8x8, OBJ_16_COLORS, 0, 0);
    OBJ_RegularEnableSet(120, 0);
    OBJ_RegularInit(121, 0, 200, OBJ_SIZE_8x8, OBJ_16_COLORS, 0, 0);
    OBJ_RegularEnableSet(121, 0);
    OBJ_RegularInit(122, 0, 200, OBJ_SIZE_8x8, OBJ_16_COLORS, 0, 0);
    OBJ_RegularEnableSet(122, 0);
    OBJ_RegularInit(123, 0, 200, OBJ_SIZE_8x8, OBJ_16_COLORS, 0, 0);
    OBJ_RegularEnableSet(123, 0);
}

void Cursor_Update(void)
{
    curframe++;
    if (curframe == 30)
        curframe = 0;
}

void Cursor_Set_Size(int w, int h)
{
    curw = w;
    curh = h;

    // Make sure that the cursor doesn't go outside of the screen

    if ((curx + curw) > GBA_SCREEN_W)
        curx = GBA_SCREEN_W - curw;

    if ((cury + curh) > GBA_SCREEN_H)
        cury = GBA_SCREEN_H - curh;
}

void Cursor_Get_Size(int *w, int *h)
{
    *w = curw;
    *h = curh;
}

void Cursor_Refresh(void)
{
    int add = 0;
    if (curframe >= 15)
        add = 1;

    int x = curx;
    int y = cury;

    OBJ_RegularInit(120, x - 4 - add, y - 4 - add,
                    OBJ_SIZE_8x8, OBJ_16_COLORS,
                    cursor_palette, cursor_tiles_index);
    OBJ_PrioritySet(120, 1);

    OBJ_RegularInit(121, x + curw - 4 + add, y - 4 - add,
                    OBJ_SIZE_8x8, OBJ_16_COLORS,
                    cursor_palette, cursor_tiles_index);
    OBJ_RegularHFlipSet(121, 1);
    OBJ_PrioritySet(121, 1);

    OBJ_RegularInit(122, x - 4 - add, y + curh - 4 + add,
                    OBJ_SIZE_8x8, OBJ_16_COLORS,
                    cursor_palette, cursor_tiles_index);
    OBJ_RegularVFlipSet(122, 1);
    OBJ_PrioritySet(122, 1);

    OBJ_RegularInit(123, x + curw - 4 + add, y + curh - 4 + add,
                    OBJ_SIZE_8x8, OBJ_16_COLORS,
                    cursor_palette, cursor_tiles_index);
    OBJ_RegularHFlipSet(123, 1);
    OBJ_RegularVFlipSet(123, 1);
    OBJ_PrioritySet(123, 1);
}

void Load_Cursor_Graphics(void *tiles_base, int tiles_index)
{
    Cursor_Hide();

    // Load the tiles
    SWI_CpuSet_Copy16(cursor_tiles_bin, tiles_base, cursor_tiles_bin_size);

    cursor_tiles_index = tiles_index;
}

void Load_Cursor_Palette(int pal_index)
{
    cursor_palette = pal_index;

    VRAM_OBJPalette16Copy(cursor_palette_bin, cursor_palette_bin_size,
                          pal_index);
}
