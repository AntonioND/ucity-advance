// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <ugba/ugba.h>

#include "input_utils.h"
#include "date.h"
#include "money.h"
#include "map_utils.h"
#include "room_game/building_info.h"
#include "room_game/build_menu.h"
#include "room_game/draw_building.h"
#include "room_game/room_game.h"
#include "room_game/tileset_info.h"
#include "room_game/status_bar.h"

// Assets

#include "graphics/cursor.h"
#include "maps/city_tileset.h"
#include "maps/test_map.h"

typedef enum {
    MODE_RUNNING,
    MODE_WATCH,
    MODE_SELECT_BUILDING,
    MODE_MODIFY_MAP,
} room_game_mode;

static room_game_mode current_mode;

static int mapx = 64;
static int mapy = 64;
static int scrollx = 0;
static int scrolly = 0;

static int last_build_x = -1;
static int last_build_y = -1;

void Load_City_Graphics(void)
{
    // Load the palettes
    VRAM_BGPalette16Copy(city_tileset_pal, city_tileset_pal_size, CITY_MAP_PALETTE);

    // Load the tiles
    SWI_CpuSet_Copy16(city_tileset_tiles, (void *)CITY_TILES_BASE, city_tileset_tiles_size);

    // Load the map
    copy_map_to_sbb(test_map_map, (void *)CITY_MAP_BASE,
                    test_map_map_height, test_map_map_width);

    // Setup background
    BG_RegularInit(2, BG_REGULAR_512x512, BG_16_COLORS,
                   CITY_TILES_BASE, CITY_MAP_BASE);
}

#define CURSOR_PALETTE      (15)
#define CURSOR_TILES_BASE   MEM_BG_TILES_BLOCK_ADDR(5)
#define CURSOR_TILES_INDEX  (512)

static int curx = (GBA_SCREEN_W / 2) - 8;
static int cury = (GBA_SCREEN_H / 2) - 8;
static int curw = 8;
static int curh = 8;
static int curframe = 0;

static void Cursor_Reset_Position(void)
{
    int offsetx = curw / 2;
    if ((offsetx % 8) != 0)
        offsetx += 4;

    int offsety = curh / 2;
    if ((offsety % 8) != 0)
        offsety += 4;

    curx = (GBA_SCREEN_W / 2) - offsetx;
    cury = (GBA_SCREEN_H / 2) - offsety;
}

static void Cursor_Set_Position(int x, int y)
{
    curx = x;
    cury = y;
}

static void Cursor_Hide(void)
{
    OBJ_RegularInit(64, 0, 200, OBJ_SIZE_8x8, OBJ_16_COLORS, 0, 0);
    OBJ_RegularEnableSet(64, 0);
    OBJ_RegularInit(65, 0, 200, OBJ_SIZE_8x8, OBJ_16_COLORS, 0, 0);
    OBJ_RegularEnableSet(65, 0);
    OBJ_RegularInit(66, 0, 200, OBJ_SIZE_8x8, OBJ_16_COLORS, 0, 0);
    OBJ_RegularEnableSet(66, 0);
    OBJ_RegularInit(67, 0, 200, OBJ_SIZE_8x8, OBJ_16_COLORS, 0, 0);
    OBJ_RegularEnableSet(67, 0);
}

static void Cursor_Update(void)
{
    curframe++;
    if (curframe == 30)
        curframe = 0;
}

static void Cursor_Set_Size(int w, int h)
{
    curw = w;
    curh = h;

    // Make sure that the cursor doesn't go outside of the screen

    if ((curx + curw) > GBA_SCREEN_W)
        curx = GBA_SCREEN_W - curw;

    if ((cury + curh) > GBA_SCREEN_H)
        cury = GBA_SCREEN_H - curh;
}

static void Cursor_Refresh(void)
{
    int add = 0;
    if (curframe >= 15)
        add = 1;

    int x = curx;
    int y = cury;

    OBJ_RegularInit(64, x - 4 - add, y - 4 - add,
                    OBJ_SIZE_8x8, OBJ_16_COLORS,
                    CURSOR_PALETTE, CURSOR_TILES_INDEX);
    OBJ_PrioritySet(64, 1);

    OBJ_RegularInit(65, x + curw - 4 + add, y - 4 - add,
                    OBJ_SIZE_8x8, OBJ_16_COLORS,
                    CURSOR_PALETTE, CURSOR_TILES_INDEX);
    OBJ_RegularHFlipSet(65, 1);
    OBJ_PrioritySet(65, 1);

    OBJ_RegularInit(66, x - 4 - add, y + curh - 4 + add,
                    OBJ_SIZE_8x8, OBJ_16_COLORS,
                    CURSOR_PALETTE, CURSOR_TILES_INDEX);
    OBJ_RegularVFlipSet(66, 1);
    OBJ_PrioritySet(66, 1);

    OBJ_RegularInit(67, x + curw - 4 + add, y + curh - 4 + add,
                    OBJ_SIZE_8x8, OBJ_16_COLORS,
                    CURSOR_PALETTE, CURSOR_TILES_INDEX);
    OBJ_RegularHFlipSet(67, 1);
    OBJ_RegularVFlipSet(67, 1);
    OBJ_PrioritySet(67, 1);
}

static void Load_Cursor_Graphics(void)
{
    // Load the palettes
    VRAM_OBJPalette16Copy(cursorPal, cursorPalLen, CURSOR_PALETTE);

    // Load the tiles
    SWI_CpuSet_Copy16(cursorTiles, (void *)CURSOR_TILES_BASE, cursorTilesLen);

    Cursor_Set_Position(48, 48);
}

void Room_Game_Load(void)
{
    BuildSelectMenuLoadGfx();
    StatusBarLoad();
    StatusBarShow();
    Load_Cursor_Graphics();

    // Load background
    // ===============

    Load_City_Graphics();

    BG_RegularScrollSet(2, 0, 0);

    DISP_ModeSet(0);
    DISP_Object1DMappingEnable(1);
    DISP_LayersEnable(1, 1, 1, 0, 1);

    CON_InitDefault();

    // Prepare cursor

    Cursor_Set_Size(8, 8);
    Cursor_Reset_Position();
    Cursor_Refresh();
}

static void Room_Game_Draw_RCI_Bars(void)
{
    char RCI[4] = { 0 };

    RCI[0] = 144 + 0;
    RCI[1] = 160 + 6;
    RCI[2] = 176 + 2;
    StatusBarPrint(27, 0, RCI);

    RCI[0] = 128;
    RCI[1] = 129;
    RCI[2] = 130;
    StatusBarPrint(27, 1, RCI);
}

static void Room_Game_Handle_Scroll(void)
{
    const int maxscrollx = (CITY_MAP_WIDTH * 8) - GBA_SCREEN_W;
    const int maxscrolly = (CITY_MAP_HEIGHT * 8) - GBA_SCREEN_H;

    const int cur_win_x_min = 8 * 4;
    const int cur_win_x_max = GBA_SCREEN_W - 8 * 4;
    const int cur_win_y_min = 8 * 4;
    const int cur_win_y_max = GBA_SCREEN_H - 8 * 4;

    if (scrolly == 0)
    {
        if (Key_Autorepeat_Pressed_Up())
        {
            if (cury > cur_win_y_min)
            {
                Cursor_Set_Position(curx, cury - 8);
            }
            else
            {
                if (mapy > 0)
                {
                    scrolly = -1;
                }
                else
                {
                    if (cury > 0)
                        Cursor_Set_Position(curx, cury - 8);
                }
            }
        }
        else if (Key_Autorepeat_Pressed_Down())
        {
            if ((cury + curh) < cur_win_y_max)
            {
                Cursor_Set_Position(curx, cury + 8);
            }
            else
            {
                if (mapy < maxscrolly)
                {
                    scrolly = 1;
                }
                else
                {
                    if ((cury + curh) < GBA_SCREEN_H)
                        Cursor_Set_Position(curx, cury + 8);
                }
            }
        }
    }

    if (scrollx == 0)
    {
        if (Key_Autorepeat_Pressed_Left())
        {
            if (curx > cur_win_x_min)
            {
                Cursor_Set_Position(curx - 8, cury);
            }
            else
            {
                if (mapx > 0)
                {
                    scrollx = -1;
                }
                else
                {
                    if (curx > 0)
                        Cursor_Set_Position(curx - 8, cury);
                }
            }
        }
        else if (Key_Autorepeat_Pressed_Right())
        {
            if ((curx + curw) < cur_win_x_max)
            {
                Cursor_Set_Position(curx + 8, cury);
            }
            else
            {
                if (mapx < maxscrollx)
                {
                    scrollx = 1;
                }
                else
                {
                    if ((curx + curw) < GBA_SCREEN_W)
                        Cursor_Set_Position(curx + 8, cury);
                }
            }
        }
    }

    // Refresh cursor

    Cursor_Update();
    Cursor_Refresh();

    // Update position of status bar

    if (cury <= cur_win_y_min)
    {
        StatusBarPositionSet(STATUS_BAR_DOWN);
        if (current_mode == MODE_MODIFY_MAP)
        {
            int building = BuildMenuSelection();
            BuildIconPlace(building, GBA_SCREEN_W - 16, GBA_SCREEN_H - 16);
        }
    }
    else if ((cury + curh) >= cur_win_y_max)
    {
        StatusBarPositionSet(STATUS_BAR_UP);
        if (current_mode == MODE_MODIFY_MAP)
        {
            int building = BuildMenuSelection();
            BuildIconPlace(building, GBA_SCREEN_W - 16, 0);
        }
    }
}

static void Room_Game_Handle_Scroll_Fast(void)
{
    const int maxscrollx = (CITY_MAP_WIDTH * 8) - GBA_SCREEN_W;
    const int maxscrolly = (CITY_MAP_HEIGHT * 8) - GBA_SCREEN_H;

    uint16_t keys = KEYS_Held();

    if (scrolly == 0)
    {
        if (keys & KEY_UP)
        {
            if (mapy > 0)
                scrolly = -1;
        }
        else if (keys & KEY_DOWN)
        {
            if (mapy < maxscrolly)
                scrolly = 1;
        }
    }

    if (scrollx == 0)
    {
        if (keys & KEY_LEFT)
        {
            if (mapx > 0)
                scrollx = -1;
        }
        else if (keys & KEY_RIGHT)
        {
            if (mapx < maxscrollx)
                scrollx = 1;
        }
    }
}

static void Room_Game_Handle_Drift(void)
{
    const int maxscrollx = (CITY_MAP_WIDTH * 8) - GBA_SCREEN_W;
    const int maxscrolly = (CITY_MAP_HEIGHT * 8) - GBA_SCREEN_H;

    mapx += scrollx;
    mapy += scrolly;

    if (mapx < 0)
        mapx = 0;
    if (mapx > maxscrollx)
        mapx = maxscrollx;
    if (mapy < 0)
        mapy = 0;
    if (mapy > maxscrolly)
        mapy = maxscrolly;

    BG_RegularScrollSet(2, mapx, mapy);

    // End map scroll if reached a grid spot

    if ((mapx % 8) == 0)
        scrollx = 0;
    if ((mapy % 8) == 0)
        scrolly = 0;
}

void BuildModeUpdateStatusBar(void)
{
    int building = BuildMenuSelection();
    building_info *bi = Get_Building_Info(building);

    char str[31];

    snprintf(str, sizeof(str), "Cost:       %9" PRId32, bi->price);
    StatusBarPrint(0, 0, str);

    int32_t money = MoneyGet();
    snprintf(str, sizeof(str), "Funds:     %10" PRId32, money);
    StatusBarPrint(0, 1, str);

    Room_Game_Draw_RCI_Bars();
}

void ModifyModeUpdateStatusBar(void)
{
    int building = BuildMenuSelection();
    building_info *bi = Get_Building_Info(building);

    char str[31];

    snprintf(str, sizeof(str), "Cost:       %9" PRId32, bi->price);
    StatusBarPrint(0, 0, str);

    int32_t money = MoneyGet();
    snprintf(str, sizeof(str), "Funds:     %10" PRId32, money);
    StatusBarPrint(0, 1, str);

    int y = (StatusBarPositionGet() == STATUS_BAR_UP) ? 0 : (GBA_SCREEN_H - 16);
    BuildIconPlace(building, GBA_SCREEN_W - 16, y);
}

void Room_Game_Set_Mode(int mode)
{
    current_mode = mode;

    switch (mode)
    {
        case MODE_RUNNING:
        {
            BuildSelectMenuHide();
            Cursor_Set_Size(8, 8);
            Cursor_Refresh();
            StatusBarClear();
            StatusBarShow();
            break;
        }
        case MODE_WATCH:
        {
            Cursor_Hide();
            StatusBarHide();
            break;
        }
        case MODE_SELECT_BUILDING:
        {
            Cursor_Hide();
            StatusBarClear();
            BuildModeUpdateStatusBar();
            BuildMenuReset();
            BuildSelectMenuShow();
            StatusBarShow();
            break;
        }
        case MODE_MODIFY_MAP:
        {
            BuildSelectMenuHide();

            building_info *info = Get_Building_Info(BuildMenuSelection());
            Cursor_Set_Size(info->width * 8, info->height * 8);
            Cursor_Refresh();
            StatusBarClear();
            ModifyModeUpdateStatusBar();
            StatusBarShow();

            last_build_x = -1;
            last_build_y = -1;
            break;
        }
        default:
        {
            UGBA_Assert(0);
            break;
        }
    }
}

void ViewModeUpdateStatusBar(void)
{
    char str[31];

    int32_t money = MoneyGet();
    snprintf(str, sizeof(str), "Funds:     %10" PRId32, money);
    StatusBarPrint(0, 0, str);

    snprintf(str, sizeof(str), "Date:  %s", DateString());
    StatusBarPrint(0, 1, str);

    Room_Game_Draw_RCI_Bars();
}

void Room_Game_Handle(void)
{
    uint16_t keys_pressed = KEYS_Pressed();
    uint16_t keys_released = KEYS_Released();
    uint16_t keys = KEYS_Held();

    switch (current_mode)
    {
        case MODE_RUNNING:
        {
            Room_Game_Handle_Scroll();

            ViewModeUpdateStatusBar();

            if (keys_pressed & KEY_SELECT)
                Room_Game_Set_Mode(MODE_SELECT_BUILDING);
            else if (keys_pressed & KEY_B)
                Room_Game_Set_Mode(MODE_WATCH);

            break;
        }
        case MODE_WATCH:
        {
            Room_Game_Handle_Scroll_Fast();

            if (keys_released & KEY_B)
                Room_Game_Set_Mode(MODE_RUNNING);

            break;
        }
        case MODE_SELECT_BUILDING:
        {
            BuildMenuHandleInput();

            if (keys_released & KEY_B)
                Room_Game_Set_Mode(MODE_RUNNING);
            else if (keys_released & KEY_A)
                Room_Game_Set_Mode(MODE_MODIFY_MAP);

            break;
        }
        case MODE_MODIFY_MAP:
        {
            Room_Game_Handle_Scroll();

            if (keys_released & KEY_B)
                Room_Game_Set_Mode(MODE_RUNNING);
            else if (keys_pressed & KEY_SELECT)
                Room_Game_Set_Mode(MODE_SELECT_BUILDING);

            int x = (mapx + curx) / 8;
            int y = (mapy + cury) / 8;

            if (keys & KEY_A)
            {
                if ((last_build_x != x) || (last_build_y != y))
                    Building_Build(0, BuildMenuSelection(), x, y);

                last_build_x = x;
                last_build_y = y;

                ModifyModeUpdateStatusBar();
            }

            break;
        }
        default:
        {
            UGBA_Assert(0);
            break;
        }
    }

    Room_Game_Handle_Drift();
}
