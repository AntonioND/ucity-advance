// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#ifndef JUKEBOX_H__
#define JUKEBOX_H__

typedef enum {
    JUKEBOX_ROOM_NONE = 0,

    JUKEBOX_ROOM_INTRO,
    JUKEBOX_ROOM_MAIN_MENU,
    JUKEBOX_ROOM_GAME,
} jukebox_room;

void Jukebox_RoomSet(jukebox_room room);
void Jukebox_Update(void);

#endif // JUKEBOX_H__
