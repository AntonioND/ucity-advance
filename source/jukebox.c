// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <umod/umod.h>

#include "audio.h"
#include "jukebox.h"
#include "random.h"

static jukebox_room current_room = JUKEBOX_ROOM_NONE;

typedef struct {
    int is_gbc;
    jukebox_room room;
} jukebox_song_info;

static const jukebox_song_info song_info[] = {
    [SONG_CITY_MOD]     = { 1, JUKEBOX_ROOM_GAME },
    [SONG_MENU_MOD]     = { 1, JUKEBOX_ROOM_MAIN_MENU },
    [SONG_TITLE_MOD]    = { 1, JUKEBOX_ROOM_INTRO },
};

static int current_index = 0;

void Jukebox_RoomSet(jukebox_room room)
{
    current_room = room;

    const size_t nelem = sizeof(song_info) / sizeof(jukebox_song_info);

    uint32_t song_count = 0;

    for (size_t i = 0; i < nelem; i++)
    {
        if (song_info[i].room == room)
            song_count++;
    }

    if (song_count == 0)
        return;

    uint32_t selection = rand_fast() % song_count;

    for (size_t i = 0; i < nelem; i++)
    {
        if (song_info[i].room == room)
        {
            if (selection == 0)
            {
                current_index = i;
                break;
            }
            selection--;
        }
    }

    Audio_Song_Play(current_index);
}

void Jukebox_Update(void)
{
    if (Audio_Enable_Get() == 0)
        return;

    if (UMOD_Song_IsPlaying())
        return;

    const size_t nelem = sizeof(song_info) / sizeof(jukebox_song_info);

    for (size_t i = 0; i < nelem; i++)
    {
        size_t check_index = (i + current_index) % nelem;

        if (song_info[check_index].room == current_room)
        {
            current_index = check_index;
            Audio_Song_Play(current_index);
            break;
        }
    }
}
