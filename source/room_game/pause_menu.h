// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef ROOM_GAME_PAUSE_MENU_H__
#define ROOM_GAME_PAUSE_MENU_H__

typedef enum {
    PAUSE_MENU_MIN,

    PAUSE_MENU_BUDGET = PAUSE_MENU_MIN,
    PAUSE_MENU_BANK,
    PAUSE_MENU_MINIMAPS,
    PAUSE_MENU_GRAPHS,
    PAUSE_MENU_CITY_STATS,
    PAUSE_MENU_OPTIONS,
    PAUSE_MENU_PAUSE,

    PAUSE_MENU_SAVE_GAME,
    PAUSE_MENU_MAIN_MENU,

    PAUSE_MENU_MAX = PAUSE_MENU_MAIN_MENU,

    PAUSE_MENU_INVALID_OPTION,
} pause_menu_options;

void PauseMenuLoad(void);
void PauseMenuDrawPauseResume(int pause);

pause_menu_options PauseMenuHandleInput(void);

#endif // ROOM_GAME_PAUSE_MENU_H__
