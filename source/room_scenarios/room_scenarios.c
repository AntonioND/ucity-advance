// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <string.h>

#include <ugba/ugba.h>

#include "date.h"
#include "input_utils.h"
#include "main.h"
#include "random.h"
#include "text_utils.h"
#include "room_game/draw_common.h"
#include "room_game/room_game.h"
#include "room_game/text_messages.h"
#include "room_game/tileset_info.h"
#include "room_input/room_input.h"
#include "simulation/simulation_common.h"
#include "simulation/simulation_money.h"
#include "simulation/simulation_technology.h"

// Assets

#include "maps/scenario_selection_bg_bin.h"
#include "maps/minimap_frame_palette_bin.h"
#include "maps/minimap_frame_tiles_bin.h"

#include "maps/city/scenario_0_rock_river_bin.h"
#include "maps/city/scenario_1_boringtown_bin.h"
#include "maps/city/scenario_2_portville_bin.h"
#include "maps/city/scenario_3_newdale_bin.h"
#include "maps/city/test_map_bin.h"

#define FRAMEBUFFER_TILES_BASE          MEM_BG_TILES_BLOCK_ADDR(0)
#define FRAMEBUFFER_MAP_BASE            MEM_BG_MAP_BLOCK_ADDR(30)
#define FRAMEBUFFER_COLOR_BASE          (192)

#define SCENARIOS_BG_PALETTE            (0)
#define SCENARIOS_BG_TILES_BASE         MEM_BG_TILES_BLOCK_ADDR(2)
#define SCENARIOS_BG_MAP_BASE           MEM_BG_MAP_BLOCK_ADDR(28)

#define MAX_PERMANENT_MSGS_TO_DISABLE   (5)

typedef struct {
    const void *map;
    const char *name;
    int start_scroll_x, start_scroll_y;
    uint32_t start_funds;
    int start_month, start_year;
    int tax_percentage;
    int payments_left;
    int amount_per_payment;
    int technology_level;
    int permanent_msgs_to_disable[MAX_PERMANENT_MSGS_TO_DISABLE];
} scenario_info;

typedef enum {
    SCENARIO_MIN,

    SCENARIO_ROCK_RIVER = SCENARIO_MIN,
    SCENARIO_BORINGTOWN,
    SCENARIO_PORTVILLE,
    SCENARIO_NEWDALE,

    SCENARIO_MAX = SCENARIO_NEWDALE,

    SCENARIO_TEST_MAP,
} scenario_enum;

static const scenario_info scenarios[] = {
    [SCENARIO_ROCK_RIVER] = {
        .map                = scenario_0_rock_river_bin,
        .name               = "Rock River",
        .start_scroll_x     = 14,
        .start_scroll_y     = 33,
        .start_funds        = 20000,
        .start_month        = 0,
        .start_year         = 1950,
        .tax_percentage     = 10,
        .payments_left      = 0,
        .amount_per_payment = 0,
        .technology_level   = 0,
        .permanent_msgs_to_disable = {
            ID_MSG_CLASS_TOWN, ID_MSG_CLASS_CITY, 0, 0, 0
        }
    },
    [SCENARIO_BORINGTOWN] = {
        .map                = scenario_1_boringtown_bin,
        .name               = "Boringtown",
        .start_scroll_x     = 24,
        .start_scroll_y     = 24,
        .start_funds        = 9000,
        .start_month        = 3,
        .start_year         = 1975,
        .tax_percentage     = 10,
        .payments_left      = 0,
        .amount_per_payment = 0,
        .technology_level   = 10,
        .permanent_msgs_to_disable = {
            ID_MSG_CLASS_TOWN, 0, 0, 0, 0
        }
    },
    [SCENARIO_PORTVILLE] = {
        .map                = scenario_2_portville_bin,
        .name               = "Portville",
        .start_scroll_x     = 7,
        .start_scroll_y     = 26,
        .start_funds        = 20000,
        .start_month        = 0,
        .start_year         = 1960,
        .tax_percentage     = 10,
        .payments_left      = 0,
        .amount_per_payment = 0,
        .technology_level   = 10,
        .permanent_msgs_to_disable = {
            ID_MSG_CLASS_TOWN, ID_MSG_CLASS_CITY, 0, 0, 0
        }
    },
    [SCENARIO_NEWDALE] = {
        .map                = scenario_3_newdale_bin,
        .name               = "Newdale",
        .start_scroll_x     = 22,
        .start_scroll_y     = 23,
        .start_funds        = 20000,
        .start_month        = 0,
        .start_year         = 1950,
        .tax_percentage     = 10,
        .payments_left      = 0,
        .amount_per_payment = 0,
        .technology_level   = 0,
        .permanent_msgs_to_disable = {
            ID_MSG_CLASS_TOWN, ID_MSG_CLASS_CITY, 0, 0, 0
        }
    },
    [SCENARIO_TEST_MAP] = {
        .map                = test_map_bin,
        .name               = "Test Map",
        .start_scroll_x     = 22,
        .start_scroll_y     = 23,
        .start_funds        = 99999999,
        .start_month        = 0,
        .start_year         = 1950,
        .tax_percentage     = 10,
        .payments_left      = 0,
        .amount_per_payment = 0,
        .technology_level   = TECH_LEVEL_MAX,
        .permanent_msgs_to_disable = {
            ID_MSG_CLASS_TOWN, ID_MSG_CLASS_CITY,
            ID_MSG_TECH_NUCLEAR, ID_MSG_TECH_FUSION, 0
        }
    },
};

static int selected_scenario = SCENARIO_MIN;

static void Room_Scenarios_Print(int x, int y, const char *text)
{
    uintptr_t addr = SCENARIOS_BG_MAP_BASE + (y * 32 + x) * 2;

    while (1)
    {
        int c = (uint8_t)*text++;
        if (c == '\0')
            break;

        uint16_t *ptr = (uint16_t *)addr;

        *ptr = MAP_REGULAR_TILE(c) | MAP_REGULAR_PALETTE(SCENARIOS_BG_PALETTE);

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
     C_YELLOW,
     C_GREY,
     C_GREY_BLUE,
     C_BLACK,
     C_RED,
     C_GREEN,
     C_LIGHT_GREEN,
     C_BLUE,
     C_LIGHT_BLUE,
     C_DARK_BLUE,
} color_index;

static void Palettes_Set_Colors(void)
{
    // Load palette
    MEM_PALETTE_BG[C_WHITE] = RGB15(31, 31, 31);
    MEM_PALETTE_BG[C_PURPLE] = RGB15(15, 0, 15);
    MEM_PALETTE_BG[C_YELLOW] = RGB15(31, 31, 0);
    MEM_PALETTE_BG[C_GREY] = RGB15(15, 15, 15);
    MEM_PALETTE_BG[C_GREY_BLUE] = RGB15(15, 15, 20);
    MEM_PALETTE_BG[C_BLACK] = RGB15(0, 0, 0);

    MEM_PALETTE_BG[C_RED] = RGB15(31, 0, 0);

    MEM_PALETTE_BG[C_LIGHT_GREEN] = RGB15(16, 31, 16);
    MEM_PALETTE_BG[C_GREEN] = RGB15(0, 31, 0);

    MEM_PALETTE_BG[C_LIGHT_BLUE] = RGB15(16, 16, 31);
    MEM_PALETTE_BG[C_DARK_BLUE] = RGB15(8, 8, 31);
    MEM_PALETTE_BG[C_BLUE] = RGB15(0, 0, 31);
}

static void Draw_Selected_Map(void)
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

    const scenario_info *s = &scenarios[selected_scenario];
    const uint16_t *map = s->map;

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t tile = map[j * CITY_MAP_WIDTH + i];
            const city_tile_info *tile_info = City_Tileset_Entry_Info(tile);
            uint16_t type = tile_info->element_type;

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

static void Draw_User_Interface(void)
{
    char str[31];
    const scenario_info *s = &scenarios[selected_scenario];

    // Name
    Room_Game_Set_City_Name(s->name);
    Room_Scenarios_Print(3, 5, Room_Game_Get_City_Name());

    // Money
    Print_Integer_Decimal_Right(str, 11, s->start_funds);
    Room_Scenarios_Print(5, 9, str);

    // Date
    DateSet(s->start_month, s->start_year);
    Room_Scenarios_Print(1, 13, DateString());
}

void Room_Scenarios_Load(void)
{
    // Load frame map
    // --------------

    // Load the tiles
    SWI_CpuSet_Copy16(minimap_frame_tiles_bin, (void *)SCENARIOS_BG_TILES_BASE,
                      minimap_frame_tiles_bin_size);

    // Load the map
    SWI_CpuSet_Copy16(scenario_selection_bg_bin, (void *)SCENARIOS_BG_MAP_BASE,
                      scenario_selection_bg_bin_size);

    // Setup background
    BG_RegularInit(1, BG_REGULAR_256x256, BG_16_COLORS,
                   SCENARIOS_BG_TILES_BASE, SCENARIOS_BG_MAP_BASE);
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

    const int bgdstx = 136;
    const int bgdsty = 48;

    const int bgx = -(bgdstx << 8);
    const int bgy = -(bgdsty << 8);

    bg_affine_src bg_src_start = {
        bgx, bgy,
        0, 0,
        1 << 8, 1 << 8,
        0
    };

    bg_affine_dst bg_dst;
    SWI_BgAffineSet(&bg_src_start, &bg_dst, 1);
    BG_AffineTransformSet(2, &bg_dst);

    // Setup display mode

    DISP_ModeSet(1);
    DISP_LayersEnable(0, 1, 1, 0, 0);

    // Initialize state

    selected_scenario = SCENARIO_MIN;
    Draw_User_Interface();
    Draw_Selected_Map();

    // Load palettes
    // -------------

    // Load frame palettes
    SWI_CpuSet_Copy16(minimap_frame_palette_bin,
                      &MEM_PALETTE_BG[SCENARIOS_BG_PALETTE],
                      minimap_frame_palette_bin_size);

    MEM_PALETTE_BG[0] = RGB15(31, 31, 31);
}

void Room_Scenarios_Unload(void)
{
    Game_Clear_Screen();
}

void Room_Scenarios_Handle(void)
{
    uint16_t keys_pressed = KEYS_Pressed();
    uint16_t keys_held = KEYS_Held();

    if (keys_pressed & KEY_B)
    {
        Game_Room_Prepare_Switch(ROOM_MAIN_MENU);
        return;
    }

    if (keys_pressed & KEY_A)
    {
        const scenario_info *s = &scenarios[selected_scenario];

        // Setup initial game state
        Room_Game_Load_City(s->map, s->name, s->start_scroll_x, s->start_scroll_y);
        Room_Game_Set_City_Date(s->start_month, s->start_year);
        Room_Game_Set_City_Economy(s->start_funds, s->tax_percentage,
                                   s->payments_left, s->amount_per_payment);
        Technology_SetLevel(s->technology_level);
        for (int i = 0; i < MAX_PERMANENT_MSGS_TO_DISABLE; i++)
        {
            int id = s->permanent_msgs_to_disable[i];
            if (id != 0)
                PersistentMessageFlagAsShown(id);
        }
        Simulation_NegativeBudgetCountSet(0);
        Simulation_GraphsResetAll();
        Room_Game_Set_Initial_Load_State();
        rand_slow_set_seed(rand_fast()); // Generate a new seed
        Game_Room_Prepare_Switch(ROOM_GAME);
        return;
    }

    // Cheat code to load test map
    const uint16_t mask = KEY_SELECT | KEY_L | KEY_UP;
    if ((keys_held & mask) == mask)
    {
        selected_scenario = SCENARIO_TEST_MAP;
        Draw_User_Interface();
        Draw_Selected_Map();
    }

    if (Key_Autorepeat_Pressed_Left())
    {
        if (selected_scenario > SCENARIO_MIN)
            selected_scenario--;
        else
            selected_scenario = SCENARIO_MAX;

        Draw_User_Interface();
        Draw_Selected_Map();
    }
    else if (Key_Autorepeat_Pressed_Right())
    {
        if (selected_scenario < SCENARIO_MAX)
            selected_scenario++;
        else
            selected_scenario = SCENARIO_MIN;

        Draw_User_Interface();
        Draw_Selected_Map();
    }
}
