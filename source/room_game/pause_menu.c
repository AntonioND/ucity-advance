// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <stdint.h>

#include <ugba/ugba.h>

#include "input_utils.h"
#include "room_game/pause_menu.h"
#include "room_game/status_bar.h"

// Assets

#include "maps/pause_menu_bg.h"

#define MENU_MAP_BASE       (TEXT_MAP_BASE + (32 * 32 * 2))

// ----------------------------------------------------------------------------

static pause_menu_options selected_option;

// ----------------------------------------------------------------------------

static int cursor_y_coordinates[] = {
    [PAUSE_MENU_BUDGET] = 5,
    [PAUSE_MENU_BANK] = 6,
    [PAUSE_MENU_MINIMAPS] = 7,
    [PAUSE_MENU_GRAPHS] = 8,
    [PAUSE_MENU_CITY_STATS] = 9,
    [PAUSE_MENU_OPTIONS] = 10,
    [PAUSE_MENU_PAUSE] = 11,

    [PAUSE_MENU_SAVE_GAME] = 13,
    [PAUSE_MENU_MAIN_MENU] = 14,
};

static void PauseMenuClearCursor(void)
{
    uint16_t *dst = (void *)MENU_MAP_BASE;

    uint16_t tile = ' ';

    int index = 32 * cursor_y_coordinates[selected_option] + 9;
    dst[index] =  MAP_REGULAR_TILE(tile) | MAP_REGULAR_PALETTE(TEXT_PALETTE);
}

static void PauseMenuDrawCursor(void)
{
    uint16_t *dst = (void *)MENU_MAP_BASE;

    uint16_t tile = 138; // TODO: Replace magic number

    int index = 32 * cursor_y_coordinates[selected_option] + 9;
    dst[index] =  MAP_REGULAR_TILE(tile) | MAP_REGULAR_PALETTE(TEXT_PALETTE);
}

pause_menu_options PauseMenuHandleInput(void)
{
    if (Key_Autorepeat_Pressed_Up())
    {
        if (selected_option > PAUSE_MENU_MIN)
        {
            PauseMenuClearCursor();
            selected_option--;
            PauseMenuDrawCursor();
        }
    }
    else if (Key_Autorepeat_Pressed_Down())
    {
        if (selected_option < PAUSE_MENU_MAX)
        {
            PauseMenuClearCursor();
            selected_option++;
            PauseMenuDrawCursor();
        }
    }

    uint16_t keys_pressed = KEYS_Pressed();

    if (keys_pressed & KEY_A)
        return selected_option;

    return PAUSE_MENU_INVALID_OPTION;
}

void PauseMenuDrawPauseResume(int pause)
{
    int y = cursor_y_coordinates[PAUSE_MENU_PAUSE] + (32 - 30);
    int x = 10;

    if (pause)
        StatusBarPrint(x, y, "Pause ");
    else
        StatusBarPrint(x, y, "Resume");
}

void PauseMenuLoad(void)
{
    // Load the map

    for (size_t i = 0; i < (pause_menu_bg_map_size / 2); i++)
    {
        const uint16_t *src = (const uint16_t *)pause_menu_bg_map;
        uint16_t *dst = (void *)MENU_MAP_BASE;

        dst[i] =  MAP_REGULAR_TILE(src[i]) | MAP_REGULAR_PALETTE(TEXT_PALETTE);
    }

    // Setup background

    BG_RegularScrollSet(1, 0, 256 - 16);

    BG_RegularInit(1, BG_REGULAR_256x512, BG_16_COLORS,
                   TEXT_TILES_BASE, TEXT_MAP_BASE);

    // Initialize state

    selected_option = PAUSE_MENU_BUDGET;

    PauseMenuDrawCursor();
}
