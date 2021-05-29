// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <stdlib.h>

#include <ugba/ugba.h>

#include "main.h"
#include "map_utils.h"
#include "room_game/room_game.h"

// Assets

#include "maps/city_tileset.h"
#include "maps/scenario_0_rock_river.h"
#include "maps/scenario_1_boringtown.h"
#include "maps/scenario_2_portville.h"
#include "maps/scenario_3_newdale.h"

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

    // Load the palettes
    VRAM_BGPalette16Copy(city_tileset_pal, city_tileset_pal_size, CITY_MAP_PALETTE);

    // Load the tiles
    SWI_CpuSet_Copy16(city_tileset_tiles, (void *)CITY_TILES_BASE, city_tileset_tiles_size);

    // Load the map
    const void *maps[] = {
        scenario_0_rock_river_map,
        scenario_1_boringtown_map,
        scenario_2_portville_map,
        scenario_3_newdale_map,
    };

    copy_map_to_sbb(maps[rand() & 3], (void *)CITY_MAP_BASE,
                    CITY_MAP_HEIGHT, CITY_MAP_WIDTH);

    // Setup background
    BG_RegularInit(2, BG_REGULAR_512x512, BG_16_COLORS,
                   CITY_TILES_BASE, CITY_MAP_BASE);

    // Initialize room state
    // ---------------------

    mapx = rand() % ((CITY_MAP_WIDTH * 8) - GBA_SCREEN_W);
    mapy = rand() % ((CITY_MAP_HEIGHT * 8) - GBA_SCREEN_H);
    BG_RegularScrollSet(2, mapx, mapy);

    if (rand() & 1)
        deltax = 1;
    else
        deltax = -1;

    if (rand() & 1)
        deltay = 1;
    else
        deltay = -1;

    /// Setup display
    //---------------

    DISP_ModeSet(0);
    DISP_Object1DMappingEnable(1);
    DISP_LayersEnable(0, 0, 1, 0, 0);
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
