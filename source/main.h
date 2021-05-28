// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#ifndef MAIN_H__
#define MAIN_H__

typedef enum {
    ROOM_INVALID,
    ROOM_GAME,
    ROOM_MINIMAP,
    ROOM_BANK,
    ROOM_BUDGET,
    ROOM_CITY_STATS,
    ROOM_INPUT,
    ROOM_GENERATE_MAP,
    ROOM_MAIN_MENU,
    ROOM_SCENARIOS,
    ROOM_GRAPHS,
    ROOM_SAVE_SLOTS,
    ROOM_CREDITS,
} room_type;

void Game_Room_Prepare_Switch(room_type new_room);

void Game_Clear_Screen(void);

#endif // MAIN_H__
