// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef ROOM_GAME_DRAW_ROAD_H__
#define ROOM_GAME_DRAW_ROAD_H__

void MapTileUpdateRoad(int x, int y);

void MapUpdateNeighboursRoad(int x, int y);

// Adds road and updates neighbours
void MapDrawRoad(int force, int x, int y);

#endif // ROOM_GAME_DRAW_ROAD_H__
