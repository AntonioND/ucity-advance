// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <string.h>

#include <ugba/ugba.h>

#include "input_utils.h"
#include "main.h"
#include "room_game/room_game.h"
#include "room_graphs/graphs_handler.h"

// Assets

#include "sprites/graph_menu_map.h"
#include "sprites/graph_menu_tiles.h"
#include "maps/graphs_frame_bg.h"
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
    GRAPHS_SELECTION_MIN,

    GRAPHS_SELECTION_POPULATION = GRAPHS_SELECTION_MIN,
    GRAPHS_SELECTION_RCI,
    GRAPHS_SELECTION_MONEY,

    GRAPHS_SELECTION_MAX = GRAPHS_SELECTION_MONEY,
} minimap_type;

static minimap_type selected_minimap;

static void Room_Graphs_Refresh_Icons(void)
{
    int x = ((GBA_SCREEN_W - 16) / 2) - selected_minimap * 16;
    int y = GBA_SCREEN_H - 16 - 8;

    for (int i = 0; i < 14; i++)
    {
        int pal = graph_menu_map_map[i * 4] >> 12;
        int tile = i * 4 + GRAPH_MENU_ICONS_TILES_INDEX;
        if (i == selected_minimap)
        {
            const obj_affine_src objsrc_init[] =
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

static void Graphs_Title(const char *text)
{
    char full_title[20];
    memset(full_title, ' ', sizeof(full_title));

    size_t l = strlen(text);
    int start = (sizeof(full_title) - l) / 2;
    int end = start + l;
    for (int i = start; i < end; i++)
        full_title[i] = *text++;

    uintptr_t addr = BG_FRAME_MAP_BASE + ((30 - sizeof(full_title)) / 2) * 2;

    for (size_t i = 0; i < sizeof(full_title); i++)
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

typedef enum {
     C_WHITE = FRAMEBUFFER_COLOR_BASE,
     C_PURPLE,
     C_ORANGE,
     C_BLACK,
     C_RED,
     C_GREEN,
     C_DARK_GREEN,
     C_BLUE,
     C_DARK_BLUE,
} color_index;

static void Palettes_Set_Colors(void)
{
    // Load palette
    MEM_PALETTE_BG[C_WHITE] = RGB15(31, 31, 31);
    MEM_PALETTE_BG[C_PURPLE] = RGB15(15, 0, 15);
    MEM_PALETTE_BG[C_ORANGE] = RGB15(31, 20, 0);
    MEM_PALETTE_BG[C_BLACK] = RGB15(0, 0, 0);
    MEM_PALETTE_BG[C_RED] = RGB15(31, 0, 0);
    MEM_PALETTE_BG[C_DARK_GREEN] = RGB15(8, 31, 8);
    MEM_PALETTE_BG[C_GREEN] = RGB15(0, 31, 0);
    MEM_PALETTE_BG[C_DARK_BLUE] = RGB15(8, 8, 31);
    MEM_PALETTE_BG[C_BLUE] = RGB15(0, 0, 31);
}

static void Draw_Graphs_Population(void)
{
    Palettes_Set_White();

    uint16_t fill = 0;
    SWI_CpuSet_Fill16(&fill, (void *)FRAMEBUFFER_TILES_BASE, 64 * 256);

    Graphs_Title("Total Population");

    graph_info *info = Graph_Get(GRAPH_INFO_POPULATION);

    int index = info->write_ptr;

    for (int i = 0; i < 64; i++)
    {
        int8_t val = info->values[index];
        if (val != GRAPH_INVALID_ENTRY)
            Plot_Tile((void *)FRAMEBUFFER_TILES_BASE, i, 63 - val, C_BLACK);

        index++;
        if (index == GRAPH_SIZE)
            index = 0;
    }

    Palettes_Set_Colors();
}

static void Draw_Graphs_Selected(void)
{
    switch (selected_minimap)
    {
        case GRAPHS_SELECTION_POPULATION:
            Draw_Graphs_Population();
            break;
        default:
            UGBA_Assert(0);
            break;
    }
}

static void Room_Graphs_Set_Watching_Mode(void)
{
    current_mode = MODE_WATCHING;

    for (int i = 0; i < 14; i++)
    {
        OBJ_RegularInit(i, i * 16, 0, OBJ_SIZE_16x16, OBJ_16_COLORS, 0, 0);
        OBJ_RegularEnableSet(i, 0);
    }
}

static void Room_Graphs_Set_Selecting_Mode(void)
{
    current_mode = MODE_SELECTING;

    Room_Graphs_Refresh_Icons();
}

void Room_Graphs_Load(void)
{
    // Load icons
    // ----------

    // Load the tiles
    SWI_CpuSet_Copy16(graph_menu_tiles_tiles,
                      (void *)GRAPH_MENU_ICONS_TILES_BASE,
                      graph_menu_tiles_tiles_size);

    // Load frame map
    // --------------

    // Load the tiles
    SWI_CpuSet_Copy16(minimap_frame_tiles_tiles, (void *)BG_FRAME_TILES_BASE,
                      minimap_frame_tiles_tiles_size);

    // Load the map
    SWI_CpuSet_Copy16(graphs_frame_bg_map, (void *)BG_FRAME_MAP_BASE,
                      graphs_frame_bg_map_size);

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

    const int bgdstx = (GBA_SCREEN_W - 128) / 2;
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
    DISP_Object1DMappingEnable(1);
    DISP_LayersEnable(0, 1, 1, 0, 1);

    selected_minimap = GRAPHS_SELECTION_POPULATION;

    Room_Graphs_Set_Watching_Mode();

    Draw_Graphs_Selected();

    // Load palettes
    // -------------

    // Load icon palettes
    VRAM_OBJPalette16Copy(graph_menu_tiles_pal, graph_menu_tiles_pal_size,
                          GRAPH_MENU_ICONS_PALETTE);

    // Load frame palettes
    SWI_CpuSet_Copy16(minimap_frame_tiles_pal, &MEM_PALETTE_BG[BG_FRAME_PALETTE],
                      minimap_frame_tiles_pal_size);

    MEM_PALETTE_BG[0] = RGB15(31, 31, 31);
}

void Room_Graphs_Unload(void)
{
    Game_Clear_Screen();
}

void Room_Graphs_Handle(void)
{
    uint16_t keys_pressed = KEYS_Pressed();

    switch (current_mode)
    {
        case MODE_WATCHING:
        {
            int left = Key_Autorepeat_Pressed_Left();
            int right = Key_Autorepeat_Pressed_Right();

            if (left || right)
                Room_Graphs_Set_Selecting_Mode();

            if (keys_pressed & (KEY_START | KEY_B))
            {
                Game_Room_Prepare_Switch(ROOM_GAME);
                return;
            }

            break;
        }
        case MODE_SELECTING:
        {
            if (keys_pressed & KEY_A)
            {
                Room_Graphs_Set_Watching_Mode();
                Draw_Graphs_Selected();
            }

            if (keys_pressed & KEY_B)
                Room_Graphs_Set_Watching_Mode();

            if (keys_pressed & KEY_START)
            {
                Game_Room_Prepare_Switch(ROOM_GAME);
                return;
            }

            if (Key_Autorepeat_Pressed_Left())
            {
                if (selected_minimap > GRAPHS_SELECTION_MIN)
                {
                    selected_minimap--;
                    Room_Graphs_Refresh_Icons();
                }
            }
            else if (Key_Autorepeat_Pressed_Right())
            {
                if (selected_minimap < GRAPHS_SELECTION_MAX)
                {
                    selected_minimap++;
                    Room_Graphs_Refresh_Icons();
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
