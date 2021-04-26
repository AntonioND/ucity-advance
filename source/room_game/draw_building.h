// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef ROOM_GAME_DRAW_BUILDING_H__
#define ROOM_GAME_DRAW_BUILDING_H__

void MapUpdateBuildingSuroundingPowerLines(int x, int y, int h, int w);

int MapDrawBuilding(int forced, int type, int x, int y);

int MapDeleteBuilding(int forced, int x, int y);

void MapClearDemolishedTile(int x, int y);

int BuildingIsCoordinateOrigin(int x, int y);

// Top level drawing functions
void Building_Remove(int force, int x, int y);
void Building_Build(int force, int type, int x, int y);

#endif // ROOM_GAME_DRAW_BUILDING_H__
