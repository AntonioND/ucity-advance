// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <string.h>

#include <ugba/ugba.h>

#include "input_utils.h"
#include "main.h"

#include "room_game/draw_common.h"
#include "room_game/room_game.h"

// Assets

#include "sprites/minimap_menu_map.h"
#include "sprites/minimap_menu_tiles.h"
#include "maps/minimap_frame_bg.h"
#include "maps/minimap_frame_tiles.h"

#define FRAMEBUFFER_TILES_BASE          MEM_BG_TILES_BLOCK_ADDR(0)
#define FRAMEBUFFER_MAP_BASE            MEM_BG_MAP_BLOCK_ADDR(16)
#define FRAMEBUFFER_COLOR_BASE          (192)

#define BG_FRAME_PALETTE                (0)
#define BG_FRAME_TILES_BASE             MEM_BG_TILES_BLOCK_ADDR(3)
#define BG_FRAME_MAP_BASE               MEM_BG_MAP_BLOCK_ADDR(28)

#define GRAPH_MENU_ICONS_PALETTE        (0)
#define GRAPH_MENU_ICONS_TILES_BASE     MEM_BG_TILES_BLOCK_ADDR(5)
#define GRAPH_MENU_ICONS_TILES_INDEX    (512)

typedef enum {
    MODE_SELECTING,
    MODE_WATCHING,
} room_minimap_mode;

static room_minimap_mode current_mode;

typedef enum {
    MINIMAP_SELECTION_MIN,

    MINIMAP_SELECTION_OVERVIEW = MINIMAP_SELECTION_MIN,
    MINIMAP_SELECTION_ZONE_MAP,
    MINIMAP_SELECTION_TRANSPORT_MAP,
    MINIMAP_SELECTION_POLICE,
    MINIMAP_SELECTION_FIRE_PROTECTION,
    MINIMAP_SELECTION_HOSPITALS,
    MINIMAP_SELECTION_SCHOOLS,
    MINIMAP_SELECTION_HIGH_SCHOOLS,
    MINIMAP_SELECTION_POWER_GRID,
    MINIMAP_SELECTION_POWER_DENSITY,
    MINIMAP_SELECTION_POPULATION_DENSITY,
    MINIMAP_SELECTION_TRAFFIC,
    MINIMAP_SELECTION_POLLUTION,
    MINIMAP_SELECTION_HAPPINESS,

    MINIMAP_SELECTION_MAX = MINIMAP_SELECTION_HAPPINESS,
} minimap_type;

static minimap_type selected_minimap;

static void Room_Minimap_Refresh_Icons(void)
{
    const int lower_bound = 2;
    const int upper_bound = MINIMAP_SELECTION_MAX - 2;

    int x;
    int y = GBA_SCREEN_H - 16 - 8;

    if (selected_minimap < lower_bound)
        x = ((GBA_SCREEN_W - 16) / 2) - lower_bound * 16;
    else if (selected_minimap >= upper_bound)
        x = ((GBA_SCREEN_W - 16) / 2) - upper_bound * 16;
    else
        x = ((GBA_SCREEN_W - 16) / 2) - selected_minimap * 16;

    for (int i = 0; i < 14; i++)
    {
        int pal = minimap_menu_map_map[i * 4] >> 12;
        int tile = i * 4 + GRAPH_MENU_ICONS_TILES_INDEX;
        if (i == selected_minimap)
        {
            obj_affine_src objsrc_init[] =
            {
                { 0.6 * (1 << 8), 0.6 * (1 << 8), 0, 0 }
            };
            SWI_ObjAffineSet_OAM(&objsrc_init[0], MEM_OAM, 1);

            OBJ_AffineInit(i, x - 8, y - 8,
                           OBJ_SIZE_16x16, 0, OBJ_16_COLORS,
                           pal, tile, 1);
            OBJ_PrioritySet(i, 0);
        }
        else
        {
            OBJ_RegularInit(i, x, y, OBJ_SIZE_16x16, OBJ_16_COLORS, pal, tile);
            OBJ_PrioritySet(i, 1);
        }
        x += 16;
    }
}

static void Minimap_Title(const char *text)
{
    char full_title[16];
    memset(full_title, ' ', sizeof(full_title));

    size_t l = strlen(text);
    int start = (sizeof(full_title) - l) / 2;
    int end = start + l;
    for (int i = start; i < end; i++)
        full_title[i] = *text++;

    uintptr_t addr = BG_FRAME_MAP_BASE + ((30 - sizeof(full_title)) / 2) * 2;

    for (int i = 0; i < 16; i++)
    {
        int c = full_title[i];

        uint16_t *ptr = (uint16_t *)addr;

        *ptr = MAP_REGULAR_TILE(c) | MAP_REGULAR_PALETTE(BG_FRAME_PALETTE);

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

static void Palettes_Set_Colors(void)
{
#define C_WHITE             (FRAMEBUFFER_COLOR_BASE + 0)
#define C_LIGHT_GREEN       (FRAMEBUFFER_COLOR_BASE + 1)
#define C_GREEN             (FRAMEBUFFER_COLOR_BASE + 2)
#define C_LIGHT_BLUE        (FRAMEBUFFER_COLOR_BASE + 3)
#define C_BLUE              (FRAMEBUFFER_COLOR_BASE + 4)
#define C_DARK_BLUE         (FRAMEBUFFER_COLOR_BASE + 5)
#define C_PURPLE            (FRAMEBUFFER_COLOR_BASE + 6)
#define C_YELLOW            (FRAMEBUFFER_COLOR_BASE + 7)
#define C_RED               (FRAMEBUFFER_COLOR_BASE + 8)
#define C_GREY              (FRAMEBUFFER_COLOR_BASE + 9)
#define C_GREY_BLUE         (FRAMEBUFFER_COLOR_BASE + 10)
#define C_BLACK             (FRAMEBUFFER_COLOR_BASE + 11)

    // Load palette
    MEM_PALETTE_BG[C_WHITE] = RGB15(31, 31, 31);
    MEM_PALETTE_BG[C_LIGHT_GREEN] = RGB15(15, 31, 15);
    MEM_PALETTE_BG[C_GREEN] = RGB15(0, 31, 0);
    MEM_PALETTE_BG[C_LIGHT_BLUE] = RGB15(15, 15, 31);
    MEM_PALETTE_BG[C_BLUE] = RGB15(0, 0, 31);
    MEM_PALETTE_BG[C_DARK_BLUE] = RGB15(0, 0, 15);
    MEM_PALETTE_BG[C_PURPLE] = RGB15(15, 0, 15);
    MEM_PALETTE_BG[C_YELLOW] = RGB15(31, 31, 0);
    MEM_PALETTE_BG[C_RED] = RGB15(31, 0, 0);
    MEM_PALETTE_BG[C_GREY] = RGB15(15, 15, 15);
    MEM_PALETTE_BG[C_GREY_BLUE] = RGB15(15, 15, 20);
    MEM_PALETTE_BG[C_BLACK] = RGB15(0, 0, 0);
}

static void Draw_Minimap_Overview(void)
{
    Palettes_Set_White();

    static uint8_t color_array[] = {
        [TYPE_FIELD] = C_WHITE,
        [TYPE_FOREST] = C_LIGHT_GREEN,
        [TYPE_WATER] = C_LIGHT_BLUE,
        [TYPE_RESIDENTIAL] = C_GREEN,
        [TYPE_INDUSTRIAL] = C_YELLOW,
        [TYPE_COMMERCIAL] = C_BLUE,
        [TYPE_POLICE_DEPT] = C_DARK_BLUE,
        [TYPE_FIRE_DEPT] = C_RED,
        [TYPE_HOSPITAL] = C_BLUE,
        [TYPE_PARK] = C_LIGHT_GREEN,
        [TYPE_STADIUM] = C_DARK_BLUE,
        [TYPE_SCHOOL] = C_PURPLE,
        [TYPE_HIGH_SCHOOL] = C_PURPLE,
        [TYPE_UNIVERSITY] = C_PURPLE,
        [TYPE_MUSEUM] = C_PURPLE,
        [TYPE_LIBRARY] = C_PURPLE,
        [TYPE_AIRPORT] = C_GREY,
        [TYPE_PORT] = C_GREY,
        [TYPE_DOCK] = C_GREY_BLUE,
        [TYPE_POWER_PLANT] = C_YELLOW,
        [TYPE_FIRE] = C_RED, // Placeholder, never used.
        [TYPE_RADIATION] = C_RED,
    };

    Minimap_Title("Overview");

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t type = CityMapGetType(i, j);

            int color;

            if (type & TYPE_HAS_ROAD)
                color = C_BLACK;
            else if (type & TYPE_HAS_TRAIN)
                color = C_BLACK;
            else if (type == (TYPE_HAS_POWER | TYPE_FIELD))
                color = C_GREY;
            else if (type == (TYPE_HAS_POWER | TYPE_WATER))
                color = C_GREY_BLUE;
            else
                color = color_array[type & TYPE_MASK];

            Plot_Tile((void *)FRAMEBUFFER_TILES_BASE, i, j, color);
        }
    }

    Palettes_Set_Colors();
}

static void Draw_Minimap_Zone(void)
{
    Palettes_Set_White();

    static uint8_t color_array[] = {
        [TYPE_FIELD] = C_WHITE,
        [TYPE_FOREST] = C_LIGHT_GREEN,
        [TYPE_WATER] = C_LIGHT_BLUE,
        [TYPE_RESIDENTIAL] = C_GREEN,
        [TYPE_INDUSTRIAL] = C_YELLOW,
        [TYPE_COMMERCIAL] = C_BLUE,
        [TYPE_POLICE_DEPT] = C_GREY,
        [TYPE_FIRE_DEPT] = C_GREY,
        [TYPE_HOSPITAL] = C_GREY,
        [TYPE_PARK] = C_LIGHT_GREEN,
        [TYPE_STADIUM] = C_GREY,
        [TYPE_SCHOOL] = C_GREY,
        [TYPE_HIGH_SCHOOL] = C_GREY,
        [TYPE_UNIVERSITY] = C_GREY,
        [TYPE_MUSEUM] = C_GREY,
        [TYPE_LIBRARY] = C_GREY,
        [TYPE_AIRPORT] = C_GREY,
        [TYPE_PORT] = C_GREY,
        [TYPE_DOCK] = C_GREY,
        [TYPE_POWER_PLANT] = C_GREY,
        [TYPE_FIRE] = C_GREY, // Placeholder, never used.
        [TYPE_RADIATION] = C_GREY,
    };

    Minimap_Title("Zone Map");

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t type = CityMapGetType(i, j);

            int color = color_array[type & TYPE_MASK];

            Plot_Tile((void *)FRAMEBUFFER_TILES_BASE, i, j, color);
        }
    }

    Palettes_Set_Colors();
}

static void Draw_Minimap_Selected(void)
{
    switch (selected_minimap)
    {
        case MINIMAP_SELECTION_OVERVIEW:
            Draw_Minimap_Overview();
            break;
        case MINIMAP_SELECTION_ZONE_MAP:
            Draw_Minimap_Zone();
            break;
        default:
            UGBA_Assert(0);
            break;
    }
}

static void Room_Minimap_Set_Watching_Mode(void)
{
    current_mode = MODE_WATCHING;

    for (int i = 0; i < 14; i++)
    {
        OBJ_RegularInit(i, i * 16, 0, OBJ_SIZE_16x16, OBJ_16_COLORS, 0, 0);
        OBJ_RegularEnableSet(i, 0);
    }

    Draw_Minimap_Selected();
}

static void Room_Minimap_Set_Selecting_Mode(void)
{
    current_mode = MODE_SELECTING;

    Room_Minimap_Refresh_Icons();
}

void Room_Minimap_Load(void)
{
    // Load icons
    // ----------

    // Load the tiles
    SWI_CpuSet_Copy16(minimap_menu_tiles_tiles,
                      (void *)GRAPH_MENU_ICONS_TILES_BASE,
                      minimap_menu_tiles_tiles_size);

    // Load frame map
    // --------------

    // Load the tiles
    SWI_CpuSet_Copy16(minimap_frame_tiles_tiles, (void *)BG_FRAME_TILES_BASE,
                      minimap_frame_tiles_tiles_size);

    // Load the map
    SWI_CpuSet_Copy16(minimap_frame_bg_map, (void *)BG_FRAME_MAP_BASE,
                      minimap_frame_bg_map_size);

    // Setup background
    BG_RegularInit(1, BG_REGULAR_256x256, BG_16_COLORS,
                   BG_FRAME_TILES_BASE, BG_FRAME_MAP_BASE);
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

    int x = -((GBA_SCREEN_W - 128) / 2) / 2;
    int y = -(24 / 2);

    bg_affine_src bg_src_start = {
        x << 8, y << 8,
        0, 0,
        1 << 7, 1 << 7,
        0
    };

    bg_affine_dst bg_dst;
    SWI_BgAffineSet(&bg_src_start, &bg_dst, 1);
    BG_AffineTransformSet(2, &bg_dst);

    // Load palettes
    // -------------

    // Load icon palettes
    VRAM_OBJPalette16Copy(minimap_menu_tiles_pal, minimap_menu_tiles_pal_size,
                          GRAPH_MENU_ICONS_PALETTE);

    // Load frame palettes
    SWI_CpuSet_Copy16(minimap_frame_tiles_pal, &MEM_PALETTE_BG[BG_FRAME_PALETTE],
                      minimap_frame_tiles_pal_size);

    MEM_PALETTE_BG[0] = RGB15(31, 31, 31);

    // Setup display mode

    DISP_ModeSet(1);
    DISP_Object1DMappingEnable(1);
    DISP_LayersEnable(0, 1, 1, 0, 1);

    selected_minimap = MINIMAP_SELECTION_OVERVIEW;

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

            if (Key_Autorepeat_Pressed_Left())
            {
                if (selected_minimap > MINIMAP_SELECTION_MIN)
                {
                    selected_minimap--;
                    Room_Minimap_Refresh_Icons();
                }
            }
            else if (Key_Autorepeat_Pressed_Right())
            {
                if (selected_minimap < MINIMAP_SELECTION_MAX)
                {
                    selected_minimap++;
                    Room_Minimap_Refresh_Icons();
                }
            }

            break;
        }
        default:
        {
            UGBA_Assert(0);
            break;
        }
    }
}
