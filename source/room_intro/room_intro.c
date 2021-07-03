// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <stdlib.h>

#include <ugba/ugba.h>

#include "main.h"
#include "map_utils.h"
#include "random.h"
#include "room_game/room_game.h"

// Assets

#include "maps/city/city_map_palette_bin.h"
#include "maps/city/city_map_palette_gbc_bin.h"
#include "maps/city/city_map_tiles_bin.h"
#include "maps/city/city_map_tiles_gbc_bin.h"
#include "maps/city/scenario_0_rock_river_bin.h"
#include "maps/city/scenario_1_boringtown_bin.h"
#include "maps/city/scenario_2_portville_bin.h"
#include "maps/city/scenario_3_newdale_bin.h"
#include "maps/intro/ucity_logo_map_bin.h"
#include "maps/intro/ucity_logo_palette_bin.h"
#include "maps/intro/ucity_logo_tiles_bin.h"
#include "maps/intro/ucity_logo_gbc_map_bin.h"
#include "maps/intro/ucity_logo_gbc_palette_bin.h"
#include "maps/intro/ucity_logo_gbc_tiles_bin.h"

// ----------------------------------------------------------------------------

static int mapx = 0;
static int mapy = 0;
static int deltax = 0;
static int deltay = 0;

void Room_Intro_Load(void)
{
    Game_Clear_Screen();

    // Load city background
    // --------------------

    if (Room_Game_Graphics_New_Get())
    {
        // Load the palettes
        VRAM_BGPalette16Copy(city_map_palette_bin, city_map_palette_bin_size,
                             CITY_MAP_PALETTE);

        // Load the tiles
        SWI_CpuSet_Copy16(city_map_tiles_bin, (void *)CITY_TILES_BASE,
                          city_map_tiles_bin_size);
    }
    else
    {
        // Load the palettes
        VRAM_BGPalette16Copy(city_map_palette_gbc_bin,
                             city_map_palette_gbc_bin_size,
                             CITY_MAP_PALETTE);

        // Load the tiles
        SWI_CpuSet_Copy16(city_map_tiles_gbc_bin, (void *)CITY_TILES_BASE,
                          city_map_tiles_gbc_bin_size);
    }

    // Load the map
    const void *maps[] = {
        scenario_0_rock_river_bin,
        scenario_1_boringtown_bin,
        scenario_2_portville_bin,
        scenario_3_newdale_bin,
    };

    copy_map_to_sbb(maps[rand_fast() & 3], (void *)CITY_MAP_BASE,
                    CITY_MAP_HEIGHT, CITY_MAP_WIDTH);

    // Setup background
    BG_RegularInit(2, BG_REGULAR_512x512, BG_256_COLORS,
                   CITY_TILES_BASE, CITY_MAP_BASE);

    // Load game logo
    // --------------

#define GAME_LOGO_PALETTE       (10)
#define GAME_LOGO_TILES_BASE    MEM_BG_TILES_BLOCK_ADDR(2)
#define GAME_LOGO_MAP_BASE      MEM_BG_MAP_BLOCK_ADDR(30)

    if (Room_Game_Graphics_New_Get())
    {
        // Load the palettes
        VRAM_BGPalette16Copy(ucity_logo_palette_bin,
                             ucity_logo_palette_bin_size,
                             GAME_LOGO_PALETTE);

        // Load the tiles
        SWI_CpuSet_Copy16(ucity_logo_tiles_bin,
                          (void *)GAME_LOGO_TILES_BASE,
                          ucity_logo_tiles_bin_size);

        // Load the map
        SWI_CpuSet_Copy16(ucity_logo_map_bin, (void *)GAME_LOGO_MAP_BASE,
                          ucity_logo_map_bin_size);
    }
    else
    {
        // Load the palettes
        VRAM_BGPalette16Copy(ucity_logo_gbc_palette_bin,
                             ucity_logo_gbc_palette_bin_size,
                             GAME_LOGO_PALETTE);

        // Load the tiles
        SWI_CpuSet_Copy16(ucity_logo_gbc_tiles_bin,
                          (void *)GAME_LOGO_TILES_BASE,
                          ucity_logo_gbc_tiles_bin_size);

        // Load the map
        SWI_CpuSet_Copy16(ucity_logo_gbc_map_bin, (void *)GAME_LOGO_MAP_BASE,
                          ucity_logo_gbc_map_bin_size);
    }

    // Setup background
    BG_RegularInit(0, BG_REGULAR_256x256, BG_16_COLORS,
                   GAME_LOGO_TILES_BASE, GAME_LOGO_MAP_BASE);

    // Initialize room state
    // ---------------------

    mapx = rand_fast() % ((CITY_MAP_WIDTH * 8) - GBA_SCREEN_W);
    mapy = rand_fast() % ((CITY_MAP_HEIGHT * 8) - GBA_SCREEN_H);
    BG_RegularScrollSet(2, mapx, mapy);

    if (rand_fast() & 1)
        deltax = 1;
    else
        deltax = -1;

    if (rand_fast() & 1)
        deltay = 1;
    else
        deltay = -1;

    /// Setup display
    //---------------

    DISP_ModeSet(0);
    DISP_Object1DMappingEnable(1);
    DISP_LayersEnable(1, 0, 1, 0, 0);
}

void Room_Intro_Unload(void)
{
    Game_Clear_Screen();
}

void Room_Intro_Handle(void)
{
    uint16_t keys_released = KEYS_Released();

    if (keys_released & (KEY_A | KEY_B | KEY_START))
    {
        Game_Room_Prepare_Switch(ROOM_MAIN_MENU);
    }
}

void Room_Intro_FastVBLHandler(void)
{
    // Scroll map

    static int frame = 0;

    frame++;
    if (frame & 1)
        return;

    mapx += deltax;
    if ((mapx > ((CITY_MAP_WIDTH * 8) - GBA_SCREEN_W)) || (mapx < 0))
    {
        deltax = -deltax;
        mapx += deltax;
    }

    mapy += deltay;
    if ((mapy > ((CITY_MAP_HEIGHT * 8) - GBA_SCREEN_H)) || (mapy < 0))
    {
        deltay = -deltay;
        mapy += deltay;
    }

    BG_RegularScrollSet(2, mapx, mapy);
}
