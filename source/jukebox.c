// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <umod/umod.h>

#include "audio.h"
#include "jukebox.h"
#include "random.h"
#include "room_game/room_game.h"
#include "simulation/calculate_stats.h"

static jukebox_room current_room = JUKEBOX_ROOM_NONE;

typedef struct {
    int is_gba;
    jukebox_room room;
    int city_level;
} jukebox_song_info;

static const jukebox_song_info song_info[] = {

    // GBC songs

    [SONG_TITLE_MOD]            = { 0, JUKEBOX_ROOM_INTRO, -1 },
    [SONG_MENU_MOD]             = { 0, JUKEBOX_ROOM_MAIN_MENU, -1 },
    [SONG_CITY_MOD]             = { 0, JUKEBOX_ROOM_GAME, -1 },

    // GBA songs

    [SONG_NO_ENERGY_MOD]        = { 1, JUKEBOX_ROOM_GAME, CLASS_VILLAGE },
    [SONG_BALTHASAR_MOD]        = { 1, JUKEBOX_ROOM_GAME, CLASS_TOWN },
    [SONG_THE_MSSING_LNK_MOD]   = { 1, JUKEBOX_ROOM_GAME, CLASS_CITY },
    [SONG_JOINT_PEOPLE_MOD]     = { 1, JUKEBOX_ROOM_GAME, CLASS_METROPOLIS },
    [SONG_IMPROV_MOD]           = { 1, JUKEBOX_ROOM_GAME, CLASS_CAPITAL },

    [SONG_TECJAZZ_MOD]          = { 1, JUKEBOX_ROOM_INTRO, -1 },

    [SONG_SCHWING_MOD]          = { 1, JUKEBOX_ROOM_MAIN_MENU, -1 },
};

static int current_index = 0;

void Jukebox_RoomSet(jukebox_room room)
{
    current_room = room;

    const size_t nelem = sizeof(song_info) / sizeof(jukebox_song_info);
    int is_gba = Room_Game_Graphics_New_Get();
    int city_level = Simulation_GetCityClass();

    uint32_t song_count = 0;

    for (size_t i = 0; i < nelem; i++)
    {
        if (song_info[i].is_gba != is_gba)
            continue;

        if (song_info[i].room != room)
            continue;

        if (song_info[i].city_level != -1)
        {
            if (song_info[i].city_level != city_level)
                continue;
        }

        song_count++;
    }

    if (song_count == 0)
        return;

    uint32_t selection = rand_fast() % song_count;

    for (size_t i = 0; i < nelem; i++)
    {
        if (song_info[i].is_gba != is_gba)
            continue;

        if (song_info[i].room != room)
            continue;

        if (song_info[i].city_level != -1)
        {
            if (song_info[i].city_level != city_level)
                continue;
        }

        if (selection == 0)
        {
            current_index = i;
            break;
        }
        selection--;
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
    int is_gba = Room_Game_Graphics_New_Get();
    int city_level = Simulation_GetCityClass();

    // Loop around all the songs starting from the one next to the current one
    for (size_t i = 0; i < nelem; i++)
    {
        size_t check_index = (i + current_index + 1) % nelem;

        if (song_info[check_index].is_gba != is_gba)
            continue;

        if (song_info[check_index].room != current_room)
            continue;

        if (song_info[check_index].city_level != -1)
        {
            if (song_info[check_index].city_level != city_level)
                continue;
        }

        current_index = check_index;
        Audio_Song_Play_Force(current_index);
        break;
    }
}
