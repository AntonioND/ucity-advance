// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <string.h>

#include <ugba/ugba.h>

#include "input_utils.h"
#include "main.h"
#include "room_game/draw_common.h"
#include "room_game/room_game.h"
#include "room_game/tileset_info.h"
#include "room_gen_map/generate_map.h"
#include "room_input/room_input.h"
#include "simulation/simulation_common.h"
#include "simulation/simulation_money.h"
#include "simulation/simulation_technology.h"

// Assets

#include "maps/generate_map_bg.h"
#include "maps/minimap_frame_tiles.h"

#define FRAMEBUFFER_TILES_BASE          MEM_BG_TILES_BLOCK_ADDR(0)
#define FRAMEBUFFER_MAP_BASE            MEM_BG_MAP_BLOCK_ADDR(16)
#define FRAMEBUFFER_COLOR_BASE          (192)

#define GEN_MAP_BG_PALETTE              (0)
#define GEN_MAP_BG_TILES_BASE           MEM_BG_TILES_BLOCK_ADDR(3)
#define GEN_MAP_BG_MAP_BASE             MEM_BG_MAP_BLOCK_ADDR(28)

typedef enum {
    MENU_TYPE_LAND,
    MENU_TYPE_WATER,
    MENU_DONE,
} menu_selections;

static menu_selections menu_selection = MENU_TYPE_LAND;

static uint8_t map_seed;
static int map_is_generated;

static void Room_Generate_Map_Print(int x, int y, const char *text)
{
    uintptr_t addr = GEN_MAP_BG_MAP_BASE + (y * 32 + x) * 2;

    while (1)
    {
        int c = (uint8_t)*text++;
        if (c == '\0')
            break;

        uint16_t *ptr = (uint16_t *)addr;

        *ptr = MAP_REGULAR_TILE(c) | MAP_REGULAR_PALETTE(GEN_MAP_BG_PALETTE);

        addr += 2;
    }
}

static void Plot_Tile(void *tiles, int x, int y, int color)
{
    int tile_index = (y / 8) * (CITY_MAP_WIDTH / 8) + (x / 8);
    int pixel_index = tile_index * (8 * 8) + ((y % 8) * 8) + (x % 8);

    uint16_t *base = tiles;
    uint16_t entry;

    if (pixel_index & 1)
    {
        entry = base[pixel_index >> 1] & 0xFF;
        entry |= color << 8;
    }
    else
    {
        entry = base[pixel_index >> 1] & 0xFF00;
        entry |= color;
    }

    base[pixel_index >> 1] = entry;
}

static void Palettes_Set_White(void)
{
    for (int i = FRAMEBUFFER_COLOR_BASE; i < 256; i++)
        MEM_PALETTE_BG[i] = RGB15(31, 31, 31);
}

typedef enum {
     C_WHITE = FRAMEBUFFER_COLOR_BASE,

     C_GREEN,
     C_LIGHT_GREEN,

     C_BLUE,
     C_LIGHT_BLUE,
} color_index;

static void Palettes_Set_Colors(void)
{
    // Load palette
    MEM_PALETTE_BG[C_WHITE] = RGB15(31, 31, 31);

    MEM_PALETTE_BG[C_LIGHT_GREEN] = RGB15(16, 31, 16);
    MEM_PALETTE_BG[C_GREEN] = RGB15(0, 31, 0);

    MEM_PALETTE_BG[C_LIGHT_BLUE] = RGB15(16, 16, 31);
    MEM_PALETTE_BG[C_BLUE] = RGB15(0, 0, 31);
}

static void Room_Generate_Map_Generate_Map(void)
{
    // 21 is the default of the algorithm. The water map generated for seed 0
    // isn't great, so rotate the seed to get something else.
    uint8_t seed1 = map_seed + 21;
    uint8_t seed2 = 229;

    int offset;
    if (menu_selection == MENU_TYPE_LAND)
        offset = -0x18; // More land
    else
        offset = 0; // More water

    Generate_Map(seed1, seed2, offset);
}

static void Draw_Generated_Map(void)
{
    Palettes_Set_White();

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t type = CityMapGetType(i, j);

            int color = C_WHITE;

            if (type == TYPE_FOREST)
                color = C_LIGHT_GREEN;
            else if (type == TYPE_WATER)
                color = C_LIGHT_BLUE;

            Plot_Tile((void *)FRAMEBUFFER_TILES_BASE, i, j, color);
        }
    }

    Palettes_Set_Colors();
}

static void Room_Generate_Map_Putc(int x, int y, uint16_t c)
{
    uintptr_t addr = GEN_MAP_BG_MAP_BASE + (y * 32 + x) * 2;
    uint16_t *ptr = (uint16_t *)addr;
    *ptr = MAP_REGULAR_TILE(c) | MAP_REGULAR_PALETTE(GEN_MAP_BG_PALETTE);
}

static int inttohex(unsigned int c)
{
    if (c < 10)
        return c + '0';
    else if (c < 16)
        return c - 10 + 'A';
    return '?';
}

static void Draw_User_Interface(void)
{
    char seed_text[3] = {
        inttohex((map_seed & 0xF0) >> 4),
        inttohex(map_seed & 0xF),
        0
    };

    Room_Generate_Map_Print(5, 5, seed_text);

    const uint16_t tile_clear = ' ';
    const uint16_t tile_cursor = 138; // TODO: Replace magic number

    Room_Generate_Map_Putc(1, 10,
            (menu_selection == MENU_TYPE_LAND) ? tile_cursor : tile_clear);
    Room_Generate_Map_Putc(1, 12,
            (menu_selection == MENU_TYPE_WATER) ? tile_cursor : tile_clear);
    Room_Generate_Map_Putc(1, 17,
            (menu_selection == MENU_DONE) ? tile_cursor : tile_clear);
}

void Room_Generate_Map_Load(void)
{
    // Load frame map
    // --------------

    // Load the tiles
    SWI_CpuSet_Copy16(minimap_frame_tiles_tiles, (void *)GEN_MAP_BG_TILES_BASE,
                      minimap_frame_tiles_tiles_size);

    // Load the map
    SWI_CpuSet_Copy16(generate_map_bg_map, (void *)GEN_MAP_BG_MAP_BASE,
                      generate_map_bg_map_size);

    // Setup background
    BG_RegularInit(1, BG_REGULAR_256x256, BG_16_COLORS,
                   GEN_MAP_BG_TILES_BASE, GEN_MAP_BG_MAP_BASE);
    BG_RegularScrollSet(1, 0, 0);

    // Load framebuffer
    // ----------------

    uint16_t fill = 0;
    SWI_CpuSet_Fill16(&fill, (void *)FRAMEBUFFER_TILES_BASE, 64 * 256);
    SWI_CpuSet_Fill16(&fill, (void *)FRAMEBUFFER_MAP_BASE, 128 * 128);

    for (int j = 0; j < (CITY_MAP_HEIGHT / 8); j++)
    {
        for (int i = 0; i < (CITY_MAP_WIDTH / 8); i += 2)
        {
            unsigned int index = j * (128 / 8) + i;

            uint16_t entry = j * (CITY_MAP_WIDTH / 8) + i;
            entry |= (entry + 1) << 8;

            uint16_t *base = (uint16_t *)FRAMEBUFFER_MAP_BASE;
            base[index >> 1] = entry;
        }
    }

    // Setup background

    BG_AffineInit(2, BG_AFFINE_128x128,
                  FRAMEBUFFER_TILES_BASE, FRAMEBUFFER_MAP_BASE, 0);

    const int bgdstx = 104;
    const int bgdsty = 24;

    const int bgx = -((bgdstx / 2) << 8);
    const int bgy = -((bgdsty / 2) << 8);

    bg_affine_src bg_src_start = {
        bgx, bgy,
        0, 0,
        1 << 7, 1 << 7,
        0
    };

    bg_affine_dst bg_dst;
    SWI_BgAffineSet(&bg_src_start, &bg_dst, 1);
    BG_AffineTransformSet(2, &bg_dst);

    // Setup display mode

    DISP_ModeSet(1);
    DISP_LayersEnable(0, 1, 1, 0, 0);

    // Initialize state

    map_seed = 0;
    map_is_generated = 0;

    Draw_User_Interface();

    // Load palettes
    // -------------

    // Load frame palettes
    SWI_CpuSet_Copy16(minimap_frame_tiles_pal, &MEM_PALETTE_BG[GEN_MAP_BG_PALETTE],
                      minimap_frame_tiles_pal_size);

    MEM_PALETTE_BG[0] = RGB15(31, 31, 31);
}

void Room_Generate_Map_Unload(void)
{
    Game_Clear_Screen();
}

void Room_Generate_Map_Handle(void)
{
    uint16_t keys_pressed = KEYS_Pressed();

    if (keys_pressed & KEY_B)
    {
        Game_Room_Prepare_Switch(ROOM_INPUT);
        return;
    }

    if (keys_pressed & KEY_A)
    {
        if ((menu_selection == MENU_TYPE_LAND) ||
            (menu_selection == MENU_TYPE_WATER))
        {
            Room_Generate_Map_Generate_Map();
            Draw_Generated_Map();
            map_is_generated = 1;
        }
        else if (menu_selection == MENU_DONE)
        {
            if (map_is_generated)
            {
                // Setup initial game state
                int scx = (CITY_MAP_WIDTH - (GBA_SCREEN_W / 8)) / 2;
                int scy = (CITY_MAP_HEIGHT - (GBA_SCREEN_H / 8)) / 2;
                Room_Game_Load_City(NULL, Room_Input_Text_String(), scx, scy);
                Room_Game_Set_City_Date(0, 1950);
                Room_Game_Set_City_Economy(20000, 10, 0, 0);
                Technology_SetLevel(0);
                // TODO: Message flags
                Simulation_NegativeBudgetCountSet(0);
                Simulation_GraphsResetAll();
                Game_Room_Prepare_Switch(ROOM_GAME);
                return;
            }
        }
    }

    if (Key_Autorepeat_Pressed_Left())
    {
        map_seed--;
        Draw_User_Interface();
    }
    else if (Key_Autorepeat_Pressed_Right())
    {
        map_seed++;
        Draw_User_Interface();
    }

    if (Key_Autorepeat_Pressed_Up())
    {
        if (menu_selection == MENU_TYPE_LAND)
            menu_selection = MENU_DONE;
        else
            menu_selection--;

        Draw_User_Interface();
    }
    else if (Key_Autorepeat_Pressed_Down())
    {
        if (menu_selection == MENU_DONE)
            menu_selection = MENU_TYPE_LAND;
        else
            menu_selection++;

        Draw_User_Interface();
    }
}
