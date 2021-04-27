// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <ugba/ugba.h>

#include "input_utils.h"
#include "main.h"

// Assets

#include "sprites/minimap_menu_map.h"
#include "sprites/minimap_menu_tiles.h"

#define GRAPH_MENU_ICONS_PALETTE        (0)
#define GRAPH_MENU_ICONS_TILES_BASE     MEM_BG_TILES_BLOCK_ADDR(5)
#define GRAPH_MENU_ICONS_TILES_INDEX    (512)

typedef enum {
    MODE_SELECTING,
    MODE_WATCHING,
} room_minimap_mode;

static room_minimap_mode current_mode;

static void Room_Minimap_Set_Watching_Mode(void)
{
    current_mode = MODE_WATCHING;

    for (int i = 0; i < 14; i++)
    {
        OBJ_RegularInit(i, i * 16, 0, OBJ_SIZE_16x16, OBJ_16_COLORS, 0, 0);
        OBJ_RegularEnableSet(i, 0);
    }
}

static void Room_Minimap_Set_Selecting_Mode(void)
{
    current_mode = MODE_SELECTING;

    for (int i = 0; i < 14; i++)
    {
        int pal = minimap_menu_map_map[i * 4] >> 12;
        int tile = i * 4 + GRAPH_MENU_ICONS_TILES_INDEX;
        OBJ_RegularInit(i, i * 16, 0, OBJ_SIZE_16x16, OBJ_16_COLORS, pal, tile);
    }
}

void Room_Minimap_Load(void)
{
    // Load icons

    // Load the palettes
    VRAM_OBJPalette16Copy(minimap_menu_tiles_pal, minimap_menu_tiles_pal_size,
                          GRAPH_MENU_ICONS_PALETTE);

    // Load the tiles
    SWI_CpuSet_Copy16(minimap_menu_tiles_tiles,
                      (void *)GRAPH_MENU_ICONS_TILES_BASE,
                      minimap_menu_tiles_tiles_size);

    Room_Minimap_Set_Watching_Mode();
}

void Room_Minimap_Handle(void)
{
    uint16_t keys_pressed = KEYS_Pressed();
    uint16_t keys_released = KEYS_Released();

    switch (current_mode)
    {
        case MODE_WATCHING:
        {
            int left = Key_Autorepeat_Pressed_Left();
            int right = Key_Autorepeat_Pressed_Right();

            if (left || right)
                Room_Minimap_Set_Selecting_Mode();

            if (keys_released & KEY_START)
                Game_Room_Load(ROOM_GAME);

            break;
        }
        case MODE_SELECTING:
        {
            if (keys_pressed & KEY_A)
                Room_Minimap_Set_Watching_Mode();

            if (keys_pressed & KEY_B)
                Room_Minimap_Set_Watching_Mode();

            if (keys_released & KEY_START)
                Game_Room_Load(ROOM_GAME);

            break;
        }
        default:
        {
            UGBA_Assert(0);
            break;
        }
    }
}
