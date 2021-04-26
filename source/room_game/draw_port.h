// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef ROOM_GAME_DRAW_PORT_H__
#define ROOM_GAME_DRAW_PORT_H__

// Draws a port and as many docks as possible (docks are free).
void MapDrawPort(int force, int x, int y);

void MapDeletePort(int force, int x, int y);

#endif // ROOM_GAME_DRAW_PORT_H__
