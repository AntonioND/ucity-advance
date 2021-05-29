// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <ugba/ugba.h>

#include "date.h"
#include "input_utils.h"
#include "main.h"
#include "money.h"
#include "text_utils.h"
#include "room_game/draw_common.h"
#include "room_game/room_game.h"
#include "simulation/simulation_calculate_stats.h"
#include "simulation/simulation_pollution.h"
#include "simulation/simulation_traffic.h"

// Assets

#include "maps/city_stats_bg_bin.h"
#include "maps/minimap_frame_palette_bin.h"
#include "maps/minimap_frame_tiles_bin.h"

#define BG_CITY_STATS_PALETTE           (0)
#define BG_CITY_STATS_TILES_BASE        MEM_BG_TILES_BLOCK_ADDR(3)
#define BG_CITY_STATS_MAP_BASE          MEM_BG_MAP_BLOCK_ADDR(28)

static void Room_City_Stats_Print(int x, int y, const char *text)
{
    uintptr_t addr = BG_CITY_STATS_MAP_BASE + (y * 32 + x) * 2;

    while (1)
    {
        int c = (uint8_t)*text++;
        if (c == '\0')
            break;

        uint16_t *ptr = (uint16_t *)addr;

        *ptr = MAP_REGULAR_TILE(c) | MAP_REGULAR_PALETTE(BG_CITY_STATS_PALETTE);

        addr += 2;
    }
}

static void Room_City_Stats_Draw(void)
{
    // Calculate land usage statistics
    // -------------------------------

    int total_land_area = 0; // Area that isn't water (in tiles)
    int developed_land_area = 0; // Area where there is something built in

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t type = CityMapGetType(i, j) & TYPE_MASK;

            // Use complete type, including flags

            if (type == TYPE_WATER)
                continue;

            // This is land

            total_land_area++;

            if ((type == TYPE_FIELD) || (type == TYPE_FOREST))
                continue;

            // This is developed land

            developed_land_area++;
        }
    }

    char str[31];
    int val;

    // Print statistics
    // ----------------

    // City name
    Room_City_Stats_Print(16, 4, Room_Game_Get_City_Name());

    // Population
    Print_Integer_Decimal_Right(str, 11, Simulation_GetTotalPopulation());
    Room_City_Stats_Print(18, 5, str);

    // City class
    Room_City_Stats_Print(18, 6, Simulation_GetCityClassString());

    // Date
    Room_City_Stats_Print(14, 7, DateString());

    // Money
    Print_Integer_Decimal_Right(str, 11, MoneyGet());
    Room_City_Stats_Print(18, 8, str);

    // Developed land
    if (total_land_area > 0)
        val = (100 * developed_land_area) / total_land_area;
    else
        val = 0;
    Print_Integer_Decimal_Right(str, 3, val);
    Room_City_Stats_Print(25, 10, str);

    // Total RCI land / Total developed land

    int r, c, i;
    Simulation_GetRCIAreasTotal(&r, &c, &i);

    if (developed_land_area > 0)
        val = (100 * r) / developed_land_area;
    else
        val = 0;
    Print_Integer_Decimal_Right(str, 3, val);
    Room_City_Stats_Print(25, 11, str);

    if (developed_land_area > 0)
        val = (100 * c) / developed_land_area;
    else
        val = 0;
    Print_Integer_Decimal_Right(str, 3, val);
    Room_City_Stats_Print(25, 12, str);

    if (developed_land_area > 0)
        val = (100 * i) / developed_land_area;
    else
        val = 0;
    Print_Integer_Decimal_Right(str, 3, val);
    Room_City_Stats_Print(25, 13, str);

    // Traffic

    Print_Integer_Decimal_Right(str, 3, Simulation_TrafficGetTrafficJamPercent());
    Room_City_Stats_Print(25, 15, str);

    // Pollution

    Print_Integer_Decimal_Right(str, 3, Simulation_PollutionGetPercentage());
    Room_City_Stats_Print(25, 17, str);
}

void Room_City_Stats_Load(void)
{
    // Load frame map
    // --------------

    // Load the tiles
    SWI_CpuSet_Copy16(minimap_frame_tiles_bin, (void *)BG_CITY_STATS_TILES_BASE,
                      minimap_frame_tiles_bin_size);

    // Load the map
    SWI_CpuSet_Copy16(city_stats_bg_bin, (void *)BG_CITY_STATS_MAP_BASE,
                      city_stats_bg_bin_size);

    // Setup background
    BG_RegularInit(1, BG_REGULAR_256x256, BG_16_COLORS,
                   BG_CITY_STATS_TILES_BASE, BG_CITY_STATS_MAP_BASE);
    BG_RegularScrollSet(1, 0, 0);

    // Update room state
    // -----------------

    // Update map

    Room_City_Stats_Draw();

    // Setup display mode

    DISP_ModeSet(1);
    DISP_LayersEnable(0, 1, 0, 0, 0);

    // Load palettes
    // -------------

    // Load frame palettes
    SWI_CpuSet_Copy16(minimap_frame_palette_bin,
                      &MEM_PALETTE_BG[BG_CITY_STATS_PALETTE],
                      minimap_frame_palette_bin_size);

    MEM_PALETTE_BG[0] = RGB15(31, 31, 31);
}

void Room_City_Stats_Unload(void)
{
    Game_Clear_Screen();
}

void Room_City_Stats_Handle(void)
{
    uint16_t keys_held = KEYS_Held();
    uint16_t keys_pressed = KEYS_Pressed();

    if (keys_pressed & KEY_B)
    {
        Game_Room_Prepare_Switch(ROOM_GAME);
        return;
    }

    // Cheat to set the amount of money
    const uint16_t mask = KEY_SELECT | KEY_UP | KEY_LEFT | KEY_A;
    if ((keys_held & mask) == mask)
    {
#define MONEY_AMOUNT_CHEAT 999999999
        MoneySet(MONEY_AMOUNT_CHEAT);
        Room_City_Stats_Draw();
    }
}
