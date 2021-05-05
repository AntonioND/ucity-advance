// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#ifndef MAIN_H__
#define MAIN_H__

#define ROOM_INVALID    (0)
#define ROOM_GAME       (1)
#define ROOM_MINIMAP    (2)

void Game_Room_Prepare_Switch(int new_room);

void Game_Clear_Screen(void);

#endif // MAIN_H__
