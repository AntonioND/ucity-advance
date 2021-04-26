// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef ROOM_GAME_BUILD_MENU_H__
#define ROOM_GAME_BUILD_MENU_H__

void BuildMenuReset(void);

void BuildSelectMenuLoadGfx(void);

void BuildMenuHandleInput(void);

void BuildSelectMenuShow(void);
void BuildSelectMenuHide(void);

int BuildMenuSelection(void);

void BuildIconPlace(int building, int x, int y);

#endif // ROOM_GAME_BUILD_MENU_H__
