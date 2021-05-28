// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <ugba/ugba.h>

#include "date.h"
#include "input_utils.h"
#include "main.h"
#include "save.h"
#include "text_utils.h"
#include "room_game/status_bar.h"
#include "room_save_slots/room_save_slots.h"

// Assets

#include "maps/save_menu_bg.h"
#include "maps/minimap_frame_tiles.h"

#define SAVE_MENU_BG_PALETTE                (0)
#define SAVE_MENU_BG_TILES_BASE             MEM_BG_TILES_BLOCK_ADDR(3)
#define SAVE_MENU_BG_MAP_BASE               MEM_BG_MAP_BLOCK_ADDR(28)

static int selected_slot;

static room_save_slots_modes room_mode;

void Room_Save_Slots_Set_Mode(room_save_slots_modes mode)
{
    room_mode = mode;
}

static void Room_Save_Slots_DrawCursor(void)
{
    uint16_t *dst = (void *)SAVE_MENU_BG_MAP_BASE;

    uint16_t tile_clear = ' ';
    uint16_t tile_cursor = 138; // TODO: Replace magic number

    {
        int index = 4 * 32 + 2;
        uint16_t tile = (selected_slot == 0) ? tile_cursor : tile_clear;
        dst[index] =  MAP_REGULAR_TILE(tile) | MAP_REGULAR_PALETTE(SAVE_MENU_BG_PALETTE);
    }
    {
        int index = 8 * 32 + 2;
        uint16_t tile = (selected_slot == 1) ? tile_cursor : tile_clear;
        dst[index] =  MAP_REGULAR_TILE(tile) | MAP_REGULAR_PALETTE(SAVE_MENU_BG_PALETTE);
    }
    {
        int index = 12 * 32 + 2;
        uint16_t tile = (selected_slot == 2) ? tile_cursor : tile_clear;
        dst[index] =  MAP_REGULAR_TILE(tile) | MAP_REGULAR_PALETTE(SAVE_MENU_BG_PALETTE);
    }
    {
        int index = 16 * 32 + 2;
        uint16_t tile = (selected_slot == 3) ? tile_cursor : tile_clear;
        dst[index] =  MAP_REGULAR_TILE(tile) | MAP_REGULAR_PALETTE(SAVE_MENU_BG_PALETTE);
    }
}

static void Room_Save_Slots_Print(int x, int y, const char *text)
{
    uintptr_t addr = SAVE_MENU_BG_MAP_BASE + (y * 32 + x) * 2;

    while (1)
    {
        int c = (uint8_t)*text++;
        if (c == '\0')
            break;

        uint16_t *ptr = (uint16_t *)addr;

        *ptr = MAP_REGULAR_TILE(c) | MAP_REGULAR_PALETTE(SAVE_MENU_BG_PALETTE);

        addr += 2;
    }
}

static void Room_Save_Slots_Draw(void)
{
    char str[31];

    for (int i = 0; i < 4; i++)
    {
        city_save_data *city = Save_Data_Get_City(i);
        if (city->name[0] == '\0')
        {
            Room_Save_Slots_Print(5, 4 + 4 * i, "              No Data");
            Room_Save_Slots_Print(5, 5 + 4 * i, "                     ");
        }
        else
        {
            char city_name[CITY_MAX_NAME_LENGTH + 1];
            memcpy(city_name, city->name, CITY_MAX_NAME_LENGTH);
            city_name[CITY_MAX_NAME_LENGTH] = '\0';
            Room_Save_Slots_Print(14, 4 + 4 * i, city_name);

            DateStringMake(str, city->month, city->year);
            Room_Save_Slots_Print(12, 5 + 4 * i, str);
        }
    }

    Room_Save_Slots_DrawCursor();
}

void Room_Save_Slots_Load(void)
{
    selected_slot = 0;

    // Load frame map
    // --------------

    // Load the tiles
    SWI_CpuSet_Copy16(minimap_frame_tiles_tiles, (void *)SAVE_MENU_BG_TILES_BASE,
                      minimap_frame_tiles_tiles_size);

    // Load the map
    SWI_CpuSet_Copy16(save_menu_bg_map, (void *)SAVE_MENU_BG_MAP_BASE,
                      save_menu_bg_map_size);

    // Setup background
    BG_RegularInit(1, BG_REGULAR_256x256, BG_16_COLORS,
                   SAVE_MENU_BG_TILES_BASE, SAVE_MENU_BG_MAP_BASE);
    BG_RegularScrollSet(1, 0, 0);

    // Update room state
    // -----------------

    // Update map

    Room_Save_Slots_Draw();

    // Setup display mode

    DISP_ModeSet(1);
    DISP_LayersEnable(0, 1, 0, 0, 1);

    // Load palettes
    // -------------

    // Load frame palettes
    SWI_CpuSet_Copy16(minimap_frame_tiles_pal, &MEM_PALETTE_BG[SAVE_MENU_BG_PALETTE],
                      minimap_frame_tiles_pal_size);

    MEM_PALETTE_BG[0] = RGB15(31, 31, 31);
}

void Room_Save_Slots_Unload(void)
{
    Game_Clear_Screen();
}

void Room_Save_Slots_Handle(void)
{
    uint16_t keys_pressed = KEYS_Pressed();

    if (keys_pressed & KEY_B)
    {
        if (room_mode == ROOM_SAVE_SLOTS_LOAD)
        {
            Game_Room_Prepare_Switch(ROOM_MAIN_MENU);
            return;
        }
        else
        {
            Game_Room_Prepare_Switch(ROOM_GAME);
            return;
        }
    }
    else if (keys_pressed & KEY_A)
    {
        if (room_mode == ROOM_SAVE_SLOTS_LOAD)
        {
            // If this is a valid slot, and the city has been loaded
            if (Room_Game_City_Load(selected_slot) != 0)
            {
                Game_Room_Prepare_Switch(ROOM_GAME);
                return;
            }
        }
        else
        {
            // Save and exit
            Room_Game_City_Save(selected_slot);
            Save_Reset_Checksum();
            Game_Room_Prepare_Switch(ROOM_GAME);
            return;
        }
    }

    if (Key_Autorepeat_Pressed_Up())
    {
        if (selected_slot == 0)
            selected_slot = 3;
        else
            selected_slot--;
        Room_Save_Slots_DrawCursor();
    }
    else if (Key_Autorepeat_Pressed_Down())
    {
        if (selected_slot == 3)
            selected_slot = 0;
        else
            selected_slot++;
        Room_Save_Slots_DrawCursor();
    }
}
