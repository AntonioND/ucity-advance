// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <ugba/ugba.h>

#include "main.h"
#include "room_game/draw_common.h"
#include "room_game/room_game.h"

// Assets

#include "maps/menus/credits_bg_bin.h"
#include "maps/menus/menus_palette_bin.h"
#include "maps/menus/menus_palette_gbc_bin.h"
#include "maps/menus/menus_tileset_bin.h"
#include "maps/menus/menus_tileset_gbc_bin.h"

#define BG_CREDITS_PALETTE          (0)
#define BG_CREDITS_TILES_BASE       MEM_BG_TILES_BLOCK_ADDR(3)
#define BG_CREDITS_MAP_BASE         MEM_BG_MAP_BLOCK_ADDR(28)

void Room_Credits_Load(void)
{
    // Load frame map
    // --------------

    // Load the tiles
    if (Room_Game_Graphics_New_Get())
    {
        SWI_CpuSet_Copy16(menus_tileset_bin, (void *)BG_CREDITS_TILES_BASE,
                          menus_tileset_bin_size);
    }
    else
    {
        SWI_CpuSet_Copy16(menus_tileset_gbc_bin, (void *)BG_CREDITS_TILES_BASE,
                          menus_tileset_gbc_bin_size);
    }

    // Load the map
    SWI_CpuSet_Copy16(credits_bg_bin, (void *)BG_CREDITS_MAP_BASE,
                      credits_bg_bin_size);

    // Setup background
    BG_RegularInit(1, BG_REGULAR_256x256, BG_16_COLORS,
                   BG_CREDITS_TILES_BASE, BG_CREDITS_MAP_BASE);
    BG_RegularScrollSet(1, 0, 0);

    // Update room state
    // -----------------

    // Setup display mode

    DISP_ModeSet(1);
    DISP_LayersEnable(0, 1, 0, 0, 0);

    // Load palettes
    // -------------

    // Load frame palettes
    if (Room_Game_Graphics_New_Get())
    {
        SWI_CpuSet_Copy16(menus_palette_bin,
                          &MEM_PALETTE_BG[BG_CREDITS_PALETTE],
                          menus_palette_bin_size);
    }
    else
    {
        SWI_CpuSet_Copy16(menus_palette_gbc_bin,
                          &MEM_PALETTE_BG[BG_CREDITS_PALETTE],
                          menus_palette_gbc_bin_size);
    }

    MEM_PALETTE_BG[0] = RGB15(31, 31, 31);
}

void Room_Credits_Unload(void)
{
    Game_Clear_Screen();
}

void Room_Credits_Handle(void)
{
    uint16_t keys_pressed = KEYS_Pressed();

    if (keys_pressed & KEY_B)
    {
        Game_Room_Prepare_Switch(ROOM_MAIN_MENU);
        return;
    }
}
