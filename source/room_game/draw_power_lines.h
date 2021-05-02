// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef ROOM_GAME_DRAW_POWER_LINES_H__
#define ROOM_GAME_DRAW_POWER_LINES_H__

uint16_t TypeHasElectricityExtended(uint16_t type);

void MapTileUpdatePowerLines(int x, int y);

void MapUpdateNeighboursPowerLines(int x, int y);

// Adds road and updates neighbours
void MapDrawPowerLines(int force, int x, int y);

#endif // ROOM_GAME_DRAW_POWER_LINES_H__
