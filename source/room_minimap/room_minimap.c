// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <string.h>

#include <ugba/ugba.h>

#include "input_utils.h"
#include "main.h"

#include "simulation/building_density.h"
#include "simulation/simulation_happiness.h"
#include "simulation/simulation_pollution.h"
#include "simulation/simulation_power.h"
#include "simulation/simulation_services.h"
#include "simulation/simulation_traffic.h"
#include "room_game/draw_common.h"
#include "room_game/draw_power_lines.h"
#include "room_game/room_game.h"
#include "room_game/tileset_info.h"

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

static void Minimap_Title(const char *text)
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
     C_WHITE    = FRAMEBUFFER_COLOR_BASE,
     C_PURPLE,
     C_YELLOW,
     C_ORANGE,
     C_GREY,
     C_GREY_BLUE,
     C_BLACK,

     C_RED_1,
     C_RED_2,
     C_RED_3,
     C_RED_4,
     C_RED_5,
     C_RED_6,
     C_RED_7,
     C_RED_8,

     C_GREEN_1,
     C_GREEN_2,
     C_GREEN_3,
     C_GREEN_4,
     C_GREEN_5,
     C_GREEN_6,
     C_GREEN_7,
     C_GREEN_8,

     C_BLUE_1,
     C_BLUE_2,
     C_BLUE_3,
     C_BLUE_4,
     C_BLUE_5,
     C_BLUE_6,
     C_BLUE_7,
     C_BLUE_8,

     C_PURPLE_1,
     C_PURPLE_2,
     C_PURPLE_3,
     C_PURPLE_4,
     C_PURPLE_5,
     C_PURPLE_6,
     C_PURPLE_7,
     C_PURPLE_8,

     C_YELLOW_RED_1,
     C_YELLOW_RED_2,
     C_YELLOW_RED_3,
     C_YELLOW_RED_4,
     C_YELLOW_RED_5,
     C_YELLOW_RED_6,
     C_YELLOW_RED_7,

     // Aliases

     C_RED = C_RED_8,

     C_GREEN = C_GREEN_8,
     C_LIGHT_GREEN = C_GREEN_4,
     C_DARK_GREEN = C_GREEN_6,

     C_BLUE = C_BLUE_8,
     C_LIGHT_BLUE = C_BLUE_4,
     C_DARK_BLUE = C_BLUE_6,
} color_index;

static void Palettes_Set_Colors(void)
{
    // Load palette
    MEM_PALETTE_BG[C_WHITE] = RGB15(31, 31, 31);
    MEM_PALETTE_BG[C_PURPLE] = RGB15(15, 0, 15);
    MEM_PALETTE_BG[C_YELLOW] = RGB15(31, 31, 0);
    MEM_PALETTE_BG[C_ORANGE] = RGB15(31, 20, 0);
    MEM_PALETTE_BG[C_GREY] = RGB15(15, 15, 15);
    MEM_PALETTE_BG[C_GREY_BLUE] = RGB15(15, 15, 20);
    MEM_PALETTE_BG[C_BLACK] = RGB15(0, 0, 0);

    MEM_PALETTE_BG[C_RED_1] = RGB15(31, 28, 28);
    MEM_PALETTE_BG[C_RED_2] = RGB15(31, 24, 24);
    MEM_PALETTE_BG[C_RED_3] = RGB15(31, 20, 20);
    MEM_PALETTE_BG[C_RED_4] = RGB15(31, 16, 16);
    MEM_PALETTE_BG[C_RED_5] = RGB15(31, 12, 12);
    MEM_PALETTE_BG[C_RED_6] = RGB15(31, 8, 8);
    MEM_PALETTE_BG[C_RED_7] = RGB15(31, 4, 4);
    MEM_PALETTE_BG[C_RED_8] = RGB15(31, 0, 0);

    MEM_PALETTE_BG[C_GREEN_1] = RGB15(28, 31, 28);
    MEM_PALETTE_BG[C_GREEN_2] = RGB15(24, 31, 24);
    MEM_PALETTE_BG[C_GREEN_3] = RGB15(20, 31, 20);
    MEM_PALETTE_BG[C_GREEN_4] = RGB15(16, 31, 16);
    MEM_PALETTE_BG[C_GREEN_5] = RGB15(12, 31, 12);
    MEM_PALETTE_BG[C_GREEN_6] = RGB15(8, 31, 8);
    MEM_PALETTE_BG[C_GREEN_7] = RGB15(4, 31, 4);
    MEM_PALETTE_BG[C_GREEN_8] = RGB15(0, 31, 0);

    MEM_PALETTE_BG[C_BLUE_1] = RGB15(28, 28, 31);
    MEM_PALETTE_BG[C_BLUE_2] = RGB15(24, 24, 31);
    MEM_PALETTE_BG[C_BLUE_3] = RGB15(20, 20, 31);
    MEM_PALETTE_BG[C_BLUE_4] = RGB15(16, 16, 31);
    MEM_PALETTE_BG[C_BLUE_5] = RGB15(12, 12, 31);
    MEM_PALETTE_BG[C_BLUE_6] = RGB15(8, 8, 31);
    MEM_PALETTE_BG[C_BLUE_7] = RGB15(4, 4, 31);
    MEM_PALETTE_BG[C_BLUE_8] = RGB15(0, 0, 31);

    MEM_PALETTE_BG[C_PURPLE_1] = RGB15(29, 28, 29);
    MEM_PALETTE_BG[C_PURPLE_2] = RGB15(27, 24, 27);
    MEM_PALETTE_BG[C_PURPLE_3] = RGB15(25, 20, 25);
    MEM_PALETTE_BG[C_PURPLE_4] = RGB15(23, 16, 23);
    MEM_PALETTE_BG[C_PURPLE_5] = RGB15(21, 12, 21);
    MEM_PALETTE_BG[C_PURPLE_6] = RGB15(19, 8, 19);
    MEM_PALETTE_BG[C_PURPLE_7] = RGB15(17, 4, 17);
    MEM_PALETTE_BG[C_PURPLE_8] = RGB15(15, 0, 15);

    MEM_PALETTE_BG[C_YELLOW_RED_1] = RGB15(31, 31, 10);
    MEM_PALETTE_BG[C_YELLOW_RED_2] = RGB15(31, 31, 0);
    MEM_PALETTE_BG[C_YELLOW_RED_3] = RGB15(31, 24, 0);
    MEM_PALETTE_BG[C_YELLOW_RED_4] = RGB15(31, 17, 0);
    MEM_PALETTE_BG[C_YELLOW_RED_5] = RGB15(31, 10, 0);
    MEM_PALETTE_BG[C_YELLOW_RED_6] = RGB15(31, 5, 0);
    MEM_PALETTE_BG[C_YELLOW_RED_7] = RGB15(31, 0, 0);
}

static void Draw_Minimap_Overview(void)
{
    Palettes_Set_White();

    static const uint8_t color_array[] = {
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

    static const uint8_t color_array[] = {
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

static void Draw_Minimap_TransportMap(void)
{
    Palettes_Set_White();

    static const uint8_t color_array[] = {
        [TYPE_FIELD] = C_WHITE,
        [TYPE_FOREST] = C_WHITE,
        [TYPE_WATER] = C_LIGHT_BLUE,
        [TYPE_RESIDENTIAL] = C_GREY,
        [TYPE_INDUSTRIAL] = C_GREY,
        [TYPE_COMMERCIAL] = C_GREY,
        [TYPE_POLICE_DEPT] = C_GREY,
        [TYPE_FIRE_DEPT] = C_GREY,
        [TYPE_HOSPITAL] = C_GREY,
        [TYPE_PARK] = C_GREY,
        [TYPE_STADIUM] = C_GREY,
        [TYPE_SCHOOL] = C_GREY,
        [TYPE_HIGH_SCHOOL] = C_GREY,
        [TYPE_UNIVERSITY] = C_GREY,
        [TYPE_MUSEUM] = C_GREY,
        [TYPE_LIBRARY] = C_GREY,
        [TYPE_AIRPORT] = C_PURPLE,
        [TYPE_PORT] = C_GREEN,
        [TYPE_DOCK] = C_LIGHT_BLUE,
        [TYPE_POWER_PLANT] = C_GREY,
        [TYPE_FIRE] = C_GREY, // Placeholder, never used.
        [TYPE_RADIATION] = C_GREY,
    };

    Minimap_Title("Transport");

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t type = CityMapGetType(i, j);

            int color;

            if (type == (TYPE_HAS_ROAD | TYPE_WATER))
                color = C_DARK_BLUE;
            else if (type == (TYPE_HAS_TRAIN | TYPE_WATER))
                color = C_PURPLE;
            else if (type & TYPE_HAS_ROAD)
                color = C_BLACK;
            else if (type & TYPE_HAS_TRAIN)
                color = C_RED;
            else
                color = color_array[type & TYPE_MASK];

            Plot_Tile((void *)FRAMEBUFFER_TILES_BASE, i, j, color);
        }
    }

    Palettes_Set_Colors();
}

static void Draw_Minimap_Police(void)
{
    Palettes_Set_White();

    Minimap_Title("Police Influence");

    Simulation_Services(T_POLICE_DEPT_CENTER);

    uint8_t *map = Simulation_ServicesGetMap();

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint8_t value = map[j * CITY_MAP_WIDTH + i];
            int color;

            if (value < 4)
                color = C_WHITE;
            else
                color = C_BLUE_1 + (value / 32);

            Plot_Tile((void *)FRAMEBUFFER_TILES_BASE, i, j, color);
        }
    }

    Palettes_Set_Colors();
}

static void Draw_Minimap_FireProtection(void)
{
    Palettes_Set_White();

    Minimap_Title("Fire Protection");

    Simulation_Services(T_FIRE_DEPT_CENTER);

    uint8_t *map = Simulation_ServicesGetMap();

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint8_t value = map[j * CITY_MAP_WIDTH + i];
            int color;

            if (value < 4)
                color = C_WHITE;
            else
                color = C_RED_1 + (value / 32);

            Plot_Tile((void *)FRAMEBUFFER_TILES_BASE, i, j, color);
        }
    }

    Palettes_Set_Colors();
}

static void Draw_Minimap_Hospitals(void)
{
    Palettes_Set_White();

    Minimap_Title("Hospitals");

    Simulation_Services(T_HOSPITAL_CENTER);

    uint8_t *map = Simulation_ServicesGetMap();

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint8_t value = map[j * CITY_MAP_WIDTH + i];
            int color;

            if (value < 4)
                color = C_WHITE;
            else
                color = C_GREEN_1 + (value / 32);

            Plot_Tile((void *)FRAMEBUFFER_TILES_BASE, i, j, color);
        }
    }

    Palettes_Set_Colors();
}

static void Draw_Minimap_Schools(void)
{
    Palettes_Set_White();

    Minimap_Title("Schools");

    Simulation_Services(T_SCHOOL_CENTER);

    uint8_t *map = Simulation_ServicesGetMap();

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint8_t value = map[j * CITY_MAP_WIDTH + i];
            int color;

            if (value < 4)
                color = C_WHITE;
            else
                color = C_PURPLE_1 + (value / 32);

            Plot_Tile((void *)FRAMEBUFFER_TILES_BASE, i, j, color);
        }
    }

    Palettes_Set_Colors();
}

static void Draw_Minimap_HighSchools(void)
{
    Palettes_Set_White();

    Minimap_Title("High Schools");

    Simulation_ServicesBig(T_HIGH_SCHOOL_CENTER);

    uint8_t *map = Simulation_ServicesGetMap();

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint8_t value = map[j * CITY_MAP_WIDTH + i];
            int color;

            if (value < 4)
                color = C_WHITE;
            else
                color = C_PURPLE_1 + (value / 32);

            Plot_Tile((void *)FRAMEBUFFER_TILES_BASE, i, j, color);
        }
    }

    Palettes_Set_Colors();
}

static void Draw_Minimap_PowerGrid(void)
{
    Palettes_Set_White();

    Minimap_Title("Power Grid");

    uint8_t *map = Simulation_PowerDistributionGetMap();

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            int color = C_WHITE;

            uint16_t type = CityMapGetType(i, j);

            if ((type & TYPE_MASK) == TYPE_POWER_PLANT)
            {
                color = C_BLUE;
            }
            else if (TypeHasElectricityExtended(type) & TYPE_HAS_POWER)
            {
                uint8_t flags = Simulation_HappinessGetFlags(i, j);

                if (flags & TILE_OK_POWER)
                {
                    // If this tile has enough power
                    color = C_GREEN;
                }
                else
                {
                    uint8_t entry = map[j * CITY_MAP_WIDTH + i];

                    // If this tile has some power, but not enough
                    if (entry > 0)
                        color = C_ORANGE;
                    else
                        color = C_RED;
                }
            }

            Plot_Tile((void *)FRAMEBUFFER_TILES_BASE, i, j, color);
        }
    }

    Palettes_Set_Colors();
}

static void Draw_Minimap_PowerDensity(void)
{
    Palettes_Set_White();

    Minimap_Title("Power Density");

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            int color = C_WHITE;

            uint16_t tile, type;
            CityMapGetTypeAndTile(i, j, &tile, &type);

            if ((type & TYPE_MASK) == TYPE_POWER_PLANT)
            {
                color = C_RED;
            }
            else if (TypeHasElectricityExtended(type) & TYPE_HAS_POWER)
            {
                const city_tile_info *tile_info = City_Tileset_Entry_Info(tile);

                if ((tile_info->base_x_delta != 0) ||
                    (tile_info->base_y_delta != 0))
                {
                    tile = CityMapGetTile(i + tile_info->base_x_delta,
                                          j + tile_info->base_y_delta);
                }

                const city_tile_density_info *info = CityTileDensityInfo(tile);

                int cost = info->energy_cost;

                if (cost == 0)
                {
                    color = C_WHITE;
                }
                else
                {
                    if (cost > 7)
                        cost = 7;

                    color = C_BLUE_1 + (cost - 1);
                }

            }

            Plot_Tile((void *)FRAMEBUFFER_TILES_BASE, i, j, color);
        }
    }

    Palettes_Set_Colors();
}

static void Draw_Minimap_PopulationDensity(void)
{
    Palettes_Set_White();

    Minimap_Title("Population Density");

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            int color = C_WHITE;

            uint16_t tile, type;
            CityMapGetTypeAndTile(i, j, &tile, &type);

            const city_tile_info *tile_info = City_Tileset_Entry_Info(tile);

            if ((tile_info->base_x_delta != 0) ||
                (tile_info->base_y_delta != 0))
            {
                tile = CityMapGetTile(i + tile_info->base_x_delta,
                                      j + tile_info->base_y_delta);
            }

            const city_tile_density_info *info = CityTileDensityInfo(tile);

            int population = info->population >> 3;

            if (population == 0)
            {
                color = C_WHITE;
            }
            else
            {
                if (population > 7)
                    population = 7;

                color = C_BLUE_1 + (population - 1);
            }

            Plot_Tile((void *)FRAMEBUFFER_TILES_BASE, i, j, color);
        }
    }

    Palettes_Set_Colors();
}

static void Draw_Minimap_Traffic(void)
{
    Palettes_Set_White();

    Minimap_Title("Traffic");

    uint8_t *map = Simulation_TrafficGetMap();

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t type = CityMapGetType(i, j);

            if (type & (TYPE_HAS_ROAD | TYPE_HAS_TRAIN))
            {
                // Has road or train

                int traffic_density = map[j * CITY_MAP_WIDTH + i];

                traffic_density >>= 3;
                if (traffic_density > 6)
                    traffic_density = 6;

                int color = C_BLUE_1 + traffic_density;

                Plot_Tile((void *)FRAMEBUFFER_TILES_BASE, i, j, color);

                continue;
            }

            // Tile doesn't have road or train

            // Check if this is a building. If so, draw as a green pattern if
            // all cars could leave/arrive, otherwise draw it red.

            type &= TYPE_MASK;

            if ((type == TYPE_FIELD) || (type  == TYPE_FOREST) ||
                (type  == TYPE_WATER) || (type  == TYPE_DOCK))
            {
                // Empty tile
                Plot_Tile((void *)FRAMEBUFFER_TILES_BASE, i, j, C_WHITE);
                continue;
            }

            uint16_t tile = CityMapGetTile(i, j);

            const city_tile_info *ti = City_Tileset_Entry_Info(tile);

            int ox = i + ti->base_x_delta;
            int oy = j + ti->base_y_delta;

            int remaining_density = map[oy * CITY_MAP_WIDTH + ox];

            // If remaining density is 0, building is ok
            if (remaining_density == 0)
                Plot_Tile((void *)FRAMEBUFFER_TILES_BASE, i, j, C_GREEN);
            else
                Plot_Tile((void *)FRAMEBUFFER_TILES_BASE, i, j, C_RED);
        }
    }

    Palettes_Set_Colors();
}

static void Draw_Minimap_Pollution(void)
{
    Palettes_Set_White();

    Minimap_Title("Pollution");

    uint8_t *map = Simulation_PollutionGetMap();

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            unsigned int pollution = map[j * CITY_MAP_WIDTH + i];

            unsigned int value = pollution >> 5;

            int color;

            if (value == 0)
                color = C_WHITE;
            else
                color = C_YELLOW_RED_1 + (value - 1);

            Plot_Tile((void *)FRAMEBUFFER_TILES_BASE, i, j, color);
        }
    }

    Palettes_Set_Colors();
}

static void Draw_Minimap_Happiness(void)
{
    Palettes_Set_White();

    // The needed flags must be a subset of the desired ones
    typedef struct {
        unsigned int desired;
        unsigned int needed;
    } flags_info;

#define FPOW    TILE_OK_POWER
#define FSER    TILE_OK_SERVICES
#define FEDU    TILE_OK_EDUCATION
#define FPOL    TILE_OK_POLLUTION
#define FTRA    TILE_OK_TRAFFIC

    const flags_info flags[] = {
        [TYPE_FIELD] = { 0, 0 }, // Not buildings, don't care...
        [TYPE_FOREST] = { 0, 0 },
        [TYPE_WATER] = { 0, 0 },
        [TYPE_RESIDENTIAL] = { FPOW | FSER | FEDU | FPOL | FTRA, FPOW | FPOL | FTRA },
        [TYPE_INDUSTRIAL] = { FPOW | FSER | FTRA, FPOW | FTRA },
        [TYPE_COMMERCIAL] = { FPOW | FSER | FPOL | FTRA, FPOW | FPOL | FTRA },
        [TYPE_POLICE_DEPT] = { FPOW | FSER | FPOL | FTRA, FPOL },
        [TYPE_FIRE_DEPT] = { FPOW | FSER | FPOL | FTRA, FPOL },
        [TYPE_HOSPITAL] = { FPOW | FSER | FPOL | FTRA, FPOL },
        [TYPE_PARK] = { FPOW | FSER | FPOL | FTRA, FPOL },
        [TYPE_STADIUM] = { FPOW | FSER | FPOL | FTRA, FPOW | FPOL },
        [TYPE_SCHOOL] = { FPOW | FSER | FPOL | FTRA, FPOW | FPOL },
        [TYPE_HIGH_SCHOOL] = { FPOW | FSER | FPOL | FTRA, FPOW | FPOL },
        [TYPE_UNIVERSITY] = { FPOW | FSER | FPOL | FTRA, FPOW | FPOL },
        [TYPE_MUSEUM] = { FPOW | FSER | FPOL | FTRA, FPOW | FPOL },
        [TYPE_LIBRARY] = { FPOW | FSER | FPOL | FTRA, FPOW | FPOL },
        [TYPE_AIRPORT] = { FPOW | FSER | FTRA, FPOW | FSER },
        [TYPE_PORT] = { FPOW | FSER | FTRA, FPOW | FSER },
        [TYPE_DOCK] = { FPOW | FSER, FPOL },
        [TYPE_POWER_PLANT] = { FSER | FTRA, 0 },
        [TYPE_FIRE] = { 0, 0 }, //  - Placeholder, never used.
        [TYPE_RADIATION] = { 0, 0 },
    };

    Minimap_Title("Happiness");

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            int color;

            uint16_t type = CityMapGetType(i, j);
            uint16_t type_masked = type & TYPE_MASK;

            unsigned int tile_flags = Simulation_HappinessGetFlags(i, j);

            unsigned int desired, needed;

            if (type & (TYPE_HAS_ROAD | TYPE_HAS_TRAIN))
            {
                desired = TILE_OK_TRAFFIC;
                needed = TILE_OK_TRAFFIC;
            }
            else if (type & TYPE_HAS_POWER)
            {
                desired = TILE_OK_POWER;
                needed = TILE_OK_POWER;
            }
            else if ((type == TYPE_FIELD) || (type  == TYPE_FOREST) ||
                     (type  == TYPE_WATER) || (type  == TYPE_DOCK))
            {
                desired = 0;
                needed = 0;
            }
            else
            {
                desired = flags[type_masked].desired;
                needed = flags[type_masked].needed;
            }

            if (needed == 0)
            {
                color = C_WHITE;
            }
            else
            {
                if ((tile_flags & desired) == desired)
                    color = C_GREEN;
                else if ((tile_flags & needed) == needed)
                    color = C_YELLOW;
                else
                    color = C_RED;
            }

            Plot_Tile((void *)FRAMEBUFFER_TILES_BASE, i, j, color);
        }
    }

    Palettes_Set_Colors();
}

#if 0

        push    hl

            ld      hl,.needed_flags_info
            ld      e,a
            ld      d,0
            add     hl,de
            add     hl,de
            ld      a,[hl+]
            ld      b,a ; b = desired flags
            ld      c,[hl] ; c = needed flags

        pop     hl

.check_flags:

        ; Register B = desired flags. Register C = needed flags

        ld      a,BANK_CITY_MAP_FLAGS
        ld      [rSVBK],a

        ld      a,[hl] ; get flags
        and     a,TILE_OK_MASK

        ld      d,a ; save flags in D

        ; Check if we have the needed flags
        and     a,c
        cp      a,c

        ld      a,d ; restore flags

        jr      z,.needed_flags_ok

            ; Needed flags not ok
            ld      a,C_RED
            jr      .paint_tile

.needed_flags_ok:

        ; Check if we have the desired flag
        and     a,b
        cp      a,b
        jr      z,.desired_flags_ok

            ; Desired flags not ok
            ld      a,C_YELLOW
            jr      .paint_tile

.desired_flags_ok:

        ; Desired and needed flags ok
        ld      a,C_GREEN
        ;jr      .paint_tile

.paint_tile:

        ld      b,a
        ld      c,a
        ld      d,a

        call    APA_SetColors ; a,b,c,d = color (0 to 3)
        LONG_CALL   APA_PixelStreamPlot2x2




; Flags: Power | Services | Education | Pollution | Traffic


;###############################################################################

#endif

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
        case MINIMAP_SELECTION_TRANSPORT_MAP:
            Draw_Minimap_TransportMap();
            break;
        case MINIMAP_SELECTION_POLICE:
            Draw_Minimap_Police();
            break;
        case MINIMAP_SELECTION_FIRE_PROTECTION:
            Draw_Minimap_FireProtection();
            break;
        case MINIMAP_SELECTION_HOSPITALS:
            Draw_Minimap_Hospitals();
            break;
        case MINIMAP_SELECTION_SCHOOLS:
            Draw_Minimap_Schools();
            break;
        case MINIMAP_SELECTION_HIGH_SCHOOLS:
            Draw_Minimap_HighSchools();
            break;
        case MINIMAP_SELECTION_POWER_GRID:
            Draw_Minimap_PowerGrid();
            break;
        case MINIMAP_SELECTION_POWER_DENSITY:
            Draw_Minimap_PowerDensity();
            break;
        case MINIMAP_SELECTION_POPULATION_DENSITY:
            Draw_Minimap_PopulationDensity();
            break;
        case MINIMAP_SELECTION_TRAFFIC:
            Draw_Minimap_Traffic();
            break;
        case MINIMAP_SELECTION_POLLUTION:
            Draw_Minimap_Pollution();
            break;
        case MINIMAP_SELECTION_HAPPINESS:
            Draw_Minimap_Happiness();
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

    Draw_Minimap_Selected();
}

void Room_Minimap_Unload(void)
{
    Game_Clear_Screen();
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

            if (keys_released & (KEY_START | KEY_B))
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
                Room_Minimap_Set_Watching_Mode();

                Draw_Minimap_Selected();
            }

            if (keys_released & KEY_B)
                Room_Minimap_Set_Watching_Mode();

            if (keys_released & KEY_START)
            {
                Game_Room_Prepare_Switch(ROOM_GAME);
                return;
            }

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
