// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <stdint.h>

#include <ugba/ugba.h>

#include "input_utils.h"
#include "text_utils.h"
#include "room_game/pause_menu.h"
#include "room_game/room_game.h"
#include "room_game/status_bar.h"
#include "simulation/simulation_calculate_stats.h"
#include "simulation/simulation_common.h"

// Assets

#include "maps/pause_menu_bg.h"

#define MENU_MAP_BASE       (TEXT_MAP_BASE + (32 * 32 * 2))

// ----------------------------------------------------------------------------

static pause_menu_options selected_option;
static pause_menu_submenus selected_submenu;

// ----------------------------------------------------------------------------

static void PauseMenuClear(void)
{
    for (size_t i = 0; i < (pause_menu_bg_map_size / 2); i++)
    {
        const uint16_t *src = (const uint16_t *)pause_menu_bg_map;
        uint16_t *dst = (void *)MENU_MAP_BASE;

        dst[i] =  MAP_REGULAR_TILE(src[i]) | MAP_REGULAR_PALETTE(TEXT_PALETTE);
    }
}

// ----------------------------------------------------------------------------

typedef struct {
    int x, y;
    const char *text;
} menu_entry_info;

static menu_entry_info menu_entry[] = {
    [PAUSE_MENU_BUDGET]             = { 11, 4, "Budget" },
    [PAUSE_MENU_BANK]               = { 11, 5, "Bank" },
    [PAUSE_MENU_MINIMAPS]           = { 11, 6, "Minimaps" },
    [PAUSE_MENU_GRAPHS]             = { 11, 7, "Graphs" },
    [PAUSE_MENU_CITY_STATS]         = { 11, 8, "City Stats" },
    [PAUSE_MENU_DISASTERS]          = { 11, 9, "Disasters" },
    [PAUSE_MENU_OPTIONS]            = { 11, 10, "Options" },
    [PAUSE_MENU_PAUSE]              = { 11, 11, "Pause" },
    [PAUSE_MENU_SAVE_GAME]          = { 11, 13, "Save Game" },
    [PAUSE_MENU_MAIN_MENU]          = { 11, 14, "Main Menu" },

    [DISASTERS_ENABLE]              = { 11, 6, "Enable" },
    [DISASTERS_START_FIRE]          = { 11, 8, "Start Fire" },
    [DISASTERS_MELTDOWN]            = { 11, 9, "Meltdown" },

    [OPTIONS_ANIMATIONS_ENABLE]     = { 11, 6, "Enable" },
    [OPTIONS_MUSIC_ENABLE]          = { 11, 10, "Enable" },
};

// ----------------------------------------------------------------------------

static void PauseMenuClearCursor(void)
{
    uint16_t *dst = (void *)MENU_MAP_BASE;

    uint16_t tile = ' ';

    int x = menu_entry[selected_option].x - 2;
    int y = menu_entry[selected_option].y; // + (32 - 30);

    int index = 32 * y + x;
    dst[index] =  MAP_REGULAR_TILE(tile) | MAP_REGULAR_PALETTE(TEXT_PALETTE);
}

static void PauseMenuDrawCursor(void)
{
    uint16_t *dst = (void *)MENU_MAP_BASE;

    uint16_t tile = 138; // TODO: Replace magic number

    int x = menu_entry[selected_option].x - 2;
    int y = menu_entry[selected_option].y; // + (32 - 30);

    int index = 32 * y + x;
    dst[index] =  MAP_REGULAR_TILE(tile) | MAP_REGULAR_PALETTE(TEXT_PALETTE);
}

// ----------------------------------------------------------------------------

static void PauseMenuDrawMain(void)
{
    for (int i = PAUSE_MENU_MIN; i <= PAUSE_MENU_MAX; i++)
    {
        int x = menu_entry[i].x;
        int y = menu_entry[i].y + (32 - 30);
        const char *text = menu_entry[i].text;

        StatusBarPrint(x, y, text);
    }

    int x = menu_entry[PAUSE_MENU_PAUSE].x;
    int y = menu_entry[PAUSE_MENU_PAUSE].y + (32 - 30);

    if (Room_Game_IsSimulationEnabled())
        StatusBarPrint(x, y, "Pause ");
    else
        StatusBarPrint(x, y, "Resume");
}

static void PauseMenuDrawDisasters(void)
{
    for (int i = DISASTERS_MENU_MIN; i <= DISASTERS_MENU_MAX; i++)
    {
        int x = menu_entry[i].x;
        int y = menu_entry[i].y + (32 - 30);
        const char *text = menu_entry[i].text;

        StatusBarPrint(x, y, text);
    }

    StatusBarPrint(9, 4 + (32 - 30), "Disasters:");

    int x = menu_entry[DISASTERS_ENABLE].x;
    int y = menu_entry[DISASTERS_ENABLE].y + (32 - 30);

    if (Simulation_AreDisastersEnabled())
        StatusBarPrint(x, y, "Disable");
    else
        StatusBarPrint(x, y, "Enable ");
}

static void PauseMenuDrawOptions(void)
{
    for (int i = OPTIONS_MENU_MIN; i <= OPTIONS_MENU_MAX; i++)
    {
        int x = menu_entry[i].x;
        int y = menu_entry[i].y + (32 - 30);
        const char *text = menu_entry[i].text;

        StatusBarPrint(x, y, text);
    }

    StatusBarPrint(9, 4 + (32 - 30), "Animations:");
    StatusBarPrint(9, 8 + (32 - 30), "Music:");

    int x = menu_entry[OPTIONS_ANIMATIONS_ENABLE].x;
    int y = menu_entry[OPTIONS_ANIMATIONS_ENABLE].y + (32 - 30);

    if (Room_Game_AreAnimationsEnabled())
        StatusBarPrint(x, y, "Disable");
    else
        StatusBarPrint(x, y, "Enable ");
}

void PauseMenuDraw(void)
{
    PauseMenuClear();

    // Print settlement class

    StatusBarPrint(11, 1 + (32 - 30), Simulation_GetCityClassString());

    // Print population

    char pop_str[31];
    Print_Integer_Decimal_Right(pop_str, 11, Simulation_GetTotalPopulation());
    StatusBarPrint(11, 0 + (32 - 30), pop_str);

    // Print city name
    StatusBarPrint(9, 2 + (32 - 30), Room_Game_Get_City_Name());

    if (selected_submenu == PAUSE_SUBMENU_MAIN)
    {
        PauseMenuDrawMain();
    }
    else if (selected_submenu == PAUSE_SUBMENU_DISASTERS)
    {
        PauseMenuDrawDisasters();
    }
    else if (selected_submenu == PAUSE_SUBMENU_OPTIONS)
    {
        PauseMenuDrawOptions();
    }
    else
    {
        UGBA_Assert(0);
    }

    PauseMenuDrawCursor();
}

// ----------------------------------------------------------------------------

void PauseMenuSetSubmenu(pause_menu_submenus submenu, pause_menu_options option)
{
    selected_submenu = submenu;

    PauseMenuDraw();
    PauseMenuClearCursor();

    if (option != PAUSE_MENU_INVALID_OPTION)
    {
        selected_option = option;
    }
    else
    {
        switch (submenu)
        {
            case PAUSE_SUBMENU_MAIN:
                selected_option = PAUSE_MENU_BUDGET;
                break;
            case PAUSE_SUBMENU_DISASTERS:
                selected_option = DISASTERS_ENABLE;
                break;
            case PAUSE_SUBMENU_OPTIONS:
                selected_option = OPTIONS_ANIMATIONS_ENABLE;
                break;
            default:
                UGBA_Assert(0);
                break;
        }
    }

    PauseMenuDrawCursor();
}

// ----------------------------------------------------------------------------

static pause_menu_options PauseMenuHandleInputMain(void)
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
    {
        if (selected_option == PAUSE_MENU_OPTIONS)
        {
            PauseMenuSetSubmenu(PAUSE_SUBMENU_OPTIONS, PAUSE_MENU_INVALID_OPTION);
            return PAUSE_MENU_INVALID_OPTION;
        }
        else if (selected_option == PAUSE_MENU_DISASTERS)
        {
            PauseMenuSetSubmenu(PAUSE_SUBMENU_DISASTERS, PAUSE_MENU_INVALID_OPTION);
            return PAUSE_MENU_INVALID_OPTION;
        }

        return selected_option;
    }

    uint16_t keys_released = KEYS_Released();

    if (keys_released & (KEY_B | KEY_START))
        return PAUSE_MENU_EXIT;

    return PAUSE_MENU_INVALID_OPTION;
}

static pause_menu_options PauseMenuHandleInputDisasters(void)
{
    if (Key_Autorepeat_Pressed_Up())
    {
        if (selected_option > DISASTERS_MENU_MIN)
        {
            PauseMenuClearCursor();
            selected_option--;
            PauseMenuDrawCursor();
        }
    }
    else if (Key_Autorepeat_Pressed_Down())
    {
        if (selected_option < DISASTERS_MENU_MAX)
        {
            PauseMenuClearCursor();
            selected_option++;
            PauseMenuDrawCursor();
        }
    }

    uint16_t keys_pressed = KEYS_Pressed();
    uint16_t keys_released = KEYS_Released();

    if (keys_released & KEY_B)
    {
        PauseMenuSetSubmenu(PAUSE_SUBMENU_MAIN, PAUSE_MENU_DISASTERS);
        return PAUSE_MENU_INVALID_OPTION;
    }

    if (keys_pressed & KEY_A)
    {
        if (selected_option == DISASTERS_START_FIRE)
            return DISASTERS_START_FIRE;
        else if (selected_option == DISASTERS_MELTDOWN)
            return DISASTERS_MELTDOWN;
        else if (selected_option == DISASTERS_ENABLE)
            return DISASTERS_ENABLE;
    }

    return PAUSE_MENU_INVALID_OPTION;
}

static pause_menu_options PauseMenuHandleInputOptions(void)
{
    if (Key_Autorepeat_Pressed_Up())
    {
        if (selected_option > OPTIONS_MENU_MIN)
        {
            PauseMenuClearCursor();
            selected_option--;
            PauseMenuDrawCursor();
        }
    }
    else if (Key_Autorepeat_Pressed_Down())
    {
        if (selected_option < OPTIONS_MENU_MAX)
        {
            PauseMenuClearCursor();
            selected_option++;
            PauseMenuDrawCursor();
        }
    }

    uint16_t keys_pressed = KEYS_Pressed();
    uint16_t keys_released = KEYS_Released();

    if (keys_released & KEY_B)
    {
        PauseMenuSetSubmenu(PAUSE_SUBMENU_MAIN, PAUSE_MENU_OPTIONS);
        return PAUSE_MENU_INVALID_OPTION;
    }

    if (keys_pressed & KEY_A)
    {
        if (selected_option == OPTIONS_ANIMATIONS_ENABLE)
            return OPTIONS_ANIMATIONS_ENABLE;
    }

    return PAUSE_MENU_INVALID_OPTION;
}

pause_menu_options PauseMenuHandleInput(void)
{
    if (selected_submenu == PAUSE_SUBMENU_MAIN)
        return PauseMenuHandleInputMain();
    else if (selected_submenu == PAUSE_SUBMENU_DISASTERS)
        return PauseMenuHandleInputDisasters();
    else if (selected_submenu == PAUSE_SUBMENU_OPTIONS)
        return PauseMenuHandleInputOptions();

    UGBA_Assert(0);

    return PAUSE_MENU_INVALID_OPTION;
}

// ----------------------------------------------------------------------------

void PauseMenuLoad(void)
{
    // Load the map

    PauseMenuClear();

    // Setup background

    BG_RegularScrollSet(1, 0, 256 - 16);

    BG_RegularInit(1, BG_REGULAR_256x512, BG_16_COLORS,
                   TEXT_TILES_BASE, TEXT_MAP_BASE);

    // Initialize state

    PauseMenuSetSubmenu(PAUSE_SUBMENU_MAIN, PAUSE_MENU_INVALID_OPTION);
}
