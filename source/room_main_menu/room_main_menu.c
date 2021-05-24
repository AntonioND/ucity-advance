// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <ugba/ugba.h>

#include "input_utils.h"
#include "main.h"
#include "text_utils.h"
#include "room_game/room_game.h"

// Assets

#include "maps/main_menu_bg.h"
#include "maps/minimap_frame_tiles.h"
#include "maps/test_map.h"

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
    SWI_CpuSet_Copy16(minimap_frame_tiles_tiles, (void *)BG_MAIN_MENU_TILES_BASE,
                      minimap_frame_tiles_tiles_size);

    // Load the map
    SWI_CpuSet_Copy16(main_menu_bg_map, (void *)BG_MAIN_MENU_MAP_BASE,
                      main_menu_bg_map_size);

    // Setup background
    BG_RegularInit(1, BG_REGULAR_256x256, BG_16_COLORS,
                   BG_MAIN_MENU_TILES_BASE, BG_MAIN_MENU_MAP_BASE);
    BG_RegularScrollSet(1, 0, 0);

    // Update room state
    // -----------------

    // Setup display mode

    DISP_ModeSet(1);
    DISP_Object1DMappingEnable(1);
    DISP_LayersEnable(0, 1, 1, 0, 1);

    // Load palettes
    // -------------

    // Load frame palettes
    SWI_CpuSet_Copy16(minimap_frame_tiles_pal, &MEM_PALETTE_BG[BG_MAIN_MENU_PALETTE],
                      minimap_frame_tiles_pal_size);

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
        if (selected_option == LOAD_SCENARIO)
        {
            Game_Room_Prepare_Switch(ROOM_SCENARIOS);
            return;
        }
        else if (selected_option == RANDOM_MAP)
        {
            Game_Room_Prepare_Switch(ROOM_INPUT);
            return;
        }
        else if (selected_option == CREDITS)
        {
            // TODO: Remove this and implement the real credits
            Room_Game_Load_City(test_map_map, "Test Map", 9, 9);
            Room_Game_Set_City_Date(0, 1950);
            Room_Game_Set_City_Economy(99999999, 10, 0, 0);
            Game_Room_Prepare_Switch(ROOM_GAME);
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
