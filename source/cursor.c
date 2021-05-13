// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <ugba/ugba.h>

// Assets

#include "graphics/cursor.h"

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
    OBJ_RegularInit(64, 0, 200, OBJ_SIZE_8x8, OBJ_16_COLORS, 0, 0);
    OBJ_RegularEnableSet(64, 0);
    OBJ_RegularInit(65, 0, 200, OBJ_SIZE_8x8, OBJ_16_COLORS, 0, 0);
    OBJ_RegularEnableSet(65, 0);
    OBJ_RegularInit(66, 0, 200, OBJ_SIZE_8x8, OBJ_16_COLORS, 0, 0);
    OBJ_RegularEnableSet(66, 0);
    OBJ_RegularInit(67, 0, 200, OBJ_SIZE_8x8, OBJ_16_COLORS, 0, 0);
    OBJ_RegularEnableSet(67, 0);
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

    OBJ_RegularInit(64, x - 4 - add, y - 4 - add,
                    OBJ_SIZE_8x8, OBJ_16_COLORS,
                    cursor_palette, cursor_tiles_index);
    OBJ_PrioritySet(64, 1);

    OBJ_RegularInit(65, x + curw - 4 + add, y - 4 - add,
                    OBJ_SIZE_8x8, OBJ_16_COLORS,
                    cursor_palette, cursor_tiles_index);
    OBJ_RegularHFlipSet(65, 1);
    OBJ_PrioritySet(65, 1);

    OBJ_RegularInit(66, x - 4 - add, y + curh - 4 + add,
                    OBJ_SIZE_8x8, OBJ_16_COLORS,
                    cursor_palette, cursor_tiles_index);
    OBJ_RegularVFlipSet(66, 1);
    OBJ_PrioritySet(66, 1);

    OBJ_RegularInit(67, x + curw - 4 + add, y + curh - 4 + add,
                    OBJ_SIZE_8x8, OBJ_16_COLORS,
                    cursor_palette, cursor_tiles_index);
    OBJ_RegularHFlipSet(67, 1);
    OBJ_RegularVFlipSet(67, 1);
    OBJ_PrioritySet(67, 1);
}

void Load_Cursor_Graphics(void *tiles_base, int tiles_index)
{
    Cursor_Hide();

    // Load the tiles
    SWI_CpuSet_Copy16(cursorTiles, tiles_base, cursorTilesLen);

    cursor_tiles_index = tiles_index;
}

void Load_Cursor_Palette(int pal_index)
{
    cursor_palette = pal_index;

    VRAM_OBJPalette16Copy(cursorPal, cursorPalLen, pal_index);
}
