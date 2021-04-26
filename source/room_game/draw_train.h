// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef ROOM_GAME_DRAW_TRAIN_H__
#define ROOM_GAME_DRAW_TRAIN_H__

void MapTileUpdateTrain(int x, int y);

void MapUpdateNeighboursTrain(int x, int y);

// Adds road and updates neighbours
void MapDrawTrain(int force, int x, int y);

#endif // ROOM_GAME_DRAW_TRAIN_H__
