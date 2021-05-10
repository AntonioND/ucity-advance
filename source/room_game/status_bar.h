// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef ROOM_GAME_STATUS_BAR_H__
#define ROOM_GAME_STATUS_BAR_H__

#define TEXT_PALETTE        (14)
#define TEXT_TILES_BASE     MEM_BG_TILES_BLOCK_ADDR(2)
#define TEXT_MAP_BASE       MEM_BG_MAP_BLOCK_ADDR(24)

#define STATUS_BAR_UP       0
#define STATUS_BAR_DOWN     1

void StatusBarLoad(void);

void StatusBarPositionSet(int position);
int StatusBarPositionGet(void);

void StatusBarShow(void);
void StatusBarHide(void);

void StatusBarClear(void);
void StatusBarPrint(int x, int y, const char *text);

#endif // ROOM_GAME_STATUS_BAR_H__
