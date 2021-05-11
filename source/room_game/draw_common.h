// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef ROOM_GAME_DRAW_COMMON_H__
#define ROOM_GAME_DRAW_COMMON_H__

#include <stdint.h>

uint16_t CityMapGetType(int x, int y);
uint16_t CityMapGetTile(int x, int y);
void CityMapGetTypeAndTile(int x, int y, uint16_t *tile, uint16_t *type);
void CityMapGetTypeAndTileUnsafe(int x, int y, uint16_t *tile, uint16_t *type);

void CityMapDrawTile(uint16_t tile, int x, int y);
void CityMapToggleHFlip(int x, int y);
void CityMapToggleVFlip(int x, int y);
void CityMapDrawTilePreserveFlip(uint16_t tile, int x, int y);

void CityMapCheckBuildBridge(int force, int x, int y, uint16_t type,
                             int *length, int *direction);

// Builds a bridge until it finds a non TYPE_WATER (exactly) tile.
// Returns the other end of the bridge in x and y.
void CityMapBuildBridge(int force, int *x, int *y, uint16_t type,
                        int length, int direction);

void MapUpdateWater(int x, int y);

void BuildingRemoveRoadTrainPowerLines(int force, int x, int y);

void DrawCityDeleteBridge(int force, int x, int y);

#endif // ROOM_GAME_DRAW_COMMON_H__
