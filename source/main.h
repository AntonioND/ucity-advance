// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#ifndef MAIN_H__
#define MAIN_H__

#define ROOM_GAME       (0)
#define ROOM_MINIMAP    (1)

void Game_Room_Load(int room);
void Game_Room_Handle_Current(void);
void Game_Clear_Screen(void);

#endif // MAIN_H__
