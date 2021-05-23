// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <ugba/ugba.h>

#include "input_utils.h"
#include "main.h"
#include "room_game/draw_common.h"
#include "room_game/room_game.h"

// Assets

#include "maps/name_input_menu_bg.h"
#include "maps/minimap_frame_tiles.h"

#define BG_INPUT_PALETTE            (0)
#define BG_INPUT_TILES_BASE         MEM_BG_TILES_BLOCK_ADDR(3)
#define BG_INPUT_MAP_BASE           MEM_BG_MAP_BLOCK_ADDR(28)

static void Room_Input_Print(int x, int y, const char *text)
{
    uintptr_t addr = BG_INPUT_MAP_BASE + (y * 32 + x) * 2;

    while (1)
    {
        int c = (uint8_t)*text++;
        if (c == '\0')
            break;

        uint16_t *ptr = (uint16_t *)addr;

        *ptr = MAP_REGULAR_TILE(c) | MAP_REGULAR_PALETTE(BG_INPUT_PALETTE);

        addr += 2;
    }
}

static void Room_Input_Putc(int x, int y, uint16_t c)
{
    uintptr_t addr = BG_INPUT_MAP_BASE + (y * 32 + x) * 2;
    uint16_t *ptr = (uint16_t *)addr;
    *ptr = MAP_REGULAR_TILE(c) | MAP_REGULAR_PALETTE(BG_INPUT_PALETTE);
}

static char text_string[CITY_MAX_NAME_LENGTH + 1];
static int text_string_len;

static void Text_String_Putc(uint16_t tile)
{
    if (text_string_len == (sizeof(text_string) - 2))
        return;

    text_string[text_string_len] = tile;
    text_string[text_string_len + 1] = '\0';
    text_string_len++;
}

static void Text_String_Unputc(void)
{
    if (text_string_len == 0)
        return;

    text_string_len--;
    text_string[text_string_len] = '\0';
}

const char *Room_Input_Text_String(void)
{
    return &text_string[0];
}

static void Text_String_Refresh(void)
{
    Room_Input_Print(9, 4, "____________");
    Room_Input_Print(9, 4, text_string);
}

#define KEYBOARD_W          25
#define KEYBOARD_H          5

#define KEYBOARD_BASE_X     3
#define KEYBOARD_BASE_Y     8

static int keyboard_x;
static int keyboard_y;

const uint16_t tile_clear = ' ';
const uint16_t tile_cursor = 133; // TODO: Replace magic number

const uint16_t keyboard_char[KEYBOARD_H][KEYBOARD_W] = {
    {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
        0, 0, 0,
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k'
    }, {
        'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
        0, 0, 0,
        'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v'
    }, {
        'W', 'X', 'Y', 'Z', 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0,
        'w', 'x', 'y', 'z', 0, 0, 0, 0, 0, 0, 0
    }, {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0,
        0, 0, 0,
        '\'', '-', ',', '.', 0, 0, 0, 0, 0, 0, 0
    }, {
        ' ', ' ', ' ', ' ', ' ', 0, 0, 0, 0, 0, 0,
        0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1
    }
};

static void Room_Input_Draw_Cursor(uint16_t tile)
{
    int x = KEYBOARD_BASE_X + keyboard_x;
    int y = KEYBOARD_BASE_Y + (keyboard_y * 2);

    Room_Input_Putc(x, y, tile);
}

void Room_Input_Load(void)
{
    // Load frame map
    // --------------

    // Load the tiles
    SWI_CpuSet_Copy16(minimap_frame_tiles_tiles, (void *)BG_INPUT_TILES_BASE,
                      minimap_frame_tiles_tiles_size);

    // Load the map
    SWI_CpuSet_Copy16(name_input_menu_bg_map, (void *)BG_INPUT_MAP_BASE,
                      name_input_menu_bg_map_size);

    // Setup background
    BG_RegularInit(1, BG_REGULAR_256x256, BG_16_COLORS,
                   BG_INPUT_TILES_BASE, BG_INPUT_MAP_BASE);
    BG_RegularScrollSet(1, 0, 0);

    // Update room state
    // -----------------

    // Setup display mode

    DISP_ModeSet(1);
    DISP_Object1DMappingEnable(1);
    DISP_LayersEnable(0, 1, 1, 0, 1);

    // Initialize room
    // ---------------

    keyboard_x = 0;
    keyboard_y = 0;

    Room_Input_Draw_Cursor(tile_cursor);

    text_string_len = 0;
    text_string[0] = '\0';

    // Load palettes
    // -------------

    // Load frame palettes
    SWI_CpuSet_Copy16(minimap_frame_tiles_pal,
                      &MEM_PALETTE_BG[BG_INPUT_PALETTE],
                      minimap_frame_tiles_pal_size);

    MEM_PALETTE_BG[0] = RGB15(31, 31, 31);
}

void Room_Input_Unload(void)
{
    Game_Clear_Screen();
}

static void Keyboard_Move_Up(void)
{
    Room_Input_Draw_Cursor(tile_clear);

    while (1)
    {
        if (keyboard_y == 0)
            keyboard_y = KEYBOARD_H - 1;
        else
            keyboard_y--;

        if (keyboard_char[keyboard_y][keyboard_x] != 0)
            break;
    }

    Room_Input_Draw_Cursor(tile_cursor);
}

static void Keyboard_Move_Down(void)
{
    Room_Input_Draw_Cursor(tile_clear);

    while (1)
    {
        if (keyboard_y == (KEYBOARD_H - 1))
            keyboard_y = 0;
        else
            keyboard_y++;

        if (keyboard_char[keyboard_y][keyboard_x] != 0)
            break;
    }

    Room_Input_Draw_Cursor(tile_cursor);
}

static void Keyboard_Move_Left(void)
{
    Room_Input_Draw_Cursor(tile_clear);

    while (1)
    {
        if (keyboard_x == 0)
            keyboard_x = KEYBOARD_W - 1;
        else
            keyboard_x--;

        if (keyboard_char[keyboard_y][keyboard_x] != 0)
            break;
    }

    Room_Input_Draw_Cursor(tile_cursor);
}

static void Keyboard_Move_Right(void)
{
    Room_Input_Draw_Cursor(tile_clear);

    while (1)
    {
        if (keyboard_x == (KEYBOARD_W - 1))
            keyboard_x = 0;
        else
            keyboard_x++;

        if (keyboard_char[keyboard_y][keyboard_x] != 0)
            break;
    }

    Room_Input_Draw_Cursor(tile_cursor);
}

void Room_Input_Handle(void)
{
    uint16_t keys_pressed = KEYS_Pressed();

    if (keys_pressed & KEY_A)
    {
        uint16_t tile = keyboard_char[keyboard_y][keyboard_x];

        if (tile == 1)
        {
            if (text_string_len > 0)
            {
                // Only end if the name isn't empty
                Game_Room_Prepare_Switch(ROOM_GENERATE_MAP);
            }
        }
        else
        {
            Text_String_Putc(tile);
            Text_String_Refresh();
        }
    }
    if (keys_pressed & KEY_B)
    {
        if (text_string_len > 0)
        {
            Text_String_Unputc();
            Text_String_Refresh();
        }
        else
        {
            Game_Room_Prepare_Switch(ROOM_MAIN_MENU);
        }
    }

    if (Key_Autorepeat_Pressed_Up())
        Keyboard_Move_Up();

    if (Key_Autorepeat_Pressed_Down())
        Keyboard_Move_Down();

    if (Key_Autorepeat_Pressed_Left())
        Keyboard_Move_Left();

    if (Key_Autorepeat_Pressed_Right())
        Keyboard_Move_Right();
}
