// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <ugba/ugba.h>

#include "input_utils.h"
#include "main.h"
#include "text_utils.h"
#include "room_game/room_game.h"
#include "room_save_slots/room_save_slots.h"
#include "simulation/budget.h"
#include "simulation/common.h"

// Assets

#include "maps/menus/main_menu_bg_bin.h"
#include "maps/menus/menus_palette_bin.h"
#include "maps/menus/menus_palette_gbc_bin.h"
#include "maps/menus/menus_tileset_bin.h"
#include "maps/menus/menus_tileset_gbc_bin.h"

#define BG_MAIN_MENU_PALETTE                (0)
#define BG_MAIN_MENU_TILES_BASE             MEM_BG_TILES_BLOCK_ADDR(3)
#define BG_MAIN_MENU_MAP_BASE               MEM_BG_MAP_BLOCK_ADDR(28)

typedef enum {
    LOAD_SAVED_GAME,
    RANDOM_MAP,
    LOAD_SCENARIO,
    CREDITS,
} main_menu_options;

static main_menu_options selected_option;

static void Room_Main_Menu_Putc(int x, int y, uint16_t c)
{
    uintptr_t addr = BG_MAIN_MENU_MAP_BASE + (y * 32 + x) * 2;
    uint16_t *ptr = (uint16_t *)addr;
    *ptr = MAP_REGULAR_TILE(c) | MAP_REGULAR_PALETTE(BG_MAIN_MENU_PALETTE);
}

static void Room_Main_Menu_DrawCursor(void)
{
    uint16_t tile_clear = ' ';
    uint16_t tile_cursor = 138; // TODO: Replace magic number

    Room_Main_Menu_Putc(6, 7,
            (selected_option == LOAD_SAVED_GAME) ? tile_cursor : tile_clear);
    Room_Main_Menu_Putc(6, 9,
            (selected_option == RANDOM_MAP) ? tile_cursor : tile_clear);
    Room_Main_Menu_Putc(6, 11,
            (selected_option == LOAD_SCENARIO) ? tile_cursor : tile_clear);
    Room_Main_Menu_Putc(6, 13,
            (selected_option == CREDITS) ? tile_cursor : tile_clear);
}

static void Room_Main_Menu_Draw(void)
{
    Room_Main_Menu_DrawCursor();
}

void Room_Main_Menu_Load(void)
{
    // Load frame map
    // --------------

    // Load the tiles
    if (Room_Game_Graphics_New_Get())
    {
        SWI_CpuSet_Copy16(menus_tileset_bin, (void *)BG_MAIN_MENU_TILES_BASE,
                          menus_tileset_bin_size);
    }
    else
    {
        SWI_CpuSet_Copy16(menus_tileset_gbc_bin,
                          (void *)BG_MAIN_MENU_TILES_BASE,
                          menus_tileset_gbc_bin_size);
    }

    // Load the map
    SWI_CpuSet_Copy16(main_menu_bg_bin, (void *)BG_MAIN_MENU_MAP_BASE,
                      main_menu_bg_bin_size);

    // Setup background
    BG_RegularInit(1, BG_REGULAR_256x256, BG_16_COLORS,
                   BG_MAIN_MENU_TILES_BASE, BG_MAIN_MENU_MAP_BASE);
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
                          &MEM_PALETTE_BG[BG_MAIN_MENU_PALETTE],
                          menus_palette_bin_size);
    }
    else
    {
        SWI_CpuSet_Copy16(menus_palette_gbc_bin,
                          &MEM_PALETTE_BG[BG_MAIN_MENU_PALETTE],
                          menus_palette_gbc_bin_size);
    }

    MEM_PALETTE_BG[0] = RGB15(31, 31, 31);

    // Initialize state
    // ----------------

    selected_option = LOAD_SAVED_GAME;

    // Update map

    Room_Main_Menu_Draw();
}

void Room_Main_Menu_Unload(void)
{
    Game_Clear_Screen();
}

void Room_Main_Menu_Handle(void)
{
    uint16_t keys_pressed = KEYS_Pressed();

    if (keys_pressed & KEY_A)
    {
        if (selected_option == LOAD_SAVED_GAME)
        {
            Room_Save_Slots_Set_Mode(ROOM_SAVE_SLOTS_LOAD);
            Game_Room_Prepare_Switch(ROOM_SAVE_SLOTS);
            return;
        }
        else if (selected_option == RANDOM_MAP)
        {
            Game_Room_Prepare_Switch(ROOM_INPUT);
            return;
        }
        else if (selected_option == LOAD_SCENARIO)
        {
            Game_Room_Prepare_Switch(ROOM_SCENARIOS);
            return;
        }
        else if (selected_option == CREDITS)
        {
            Game_Room_Prepare_Switch(ROOM_CREDITS);
            return;
        }
    }

    if (Key_Autorepeat_Pressed_Up())
    {
        if (selected_option > LOAD_SAVED_GAME)
        {
            selected_option--;
            Room_Main_Menu_DrawCursor();
        }
    }
    else if (Key_Autorepeat_Pressed_Down())
    {
        if (selected_option < CREDITS)
        {
            selected_option++;
            Room_Main_Menu_DrawCursor();
        }
    }
}
