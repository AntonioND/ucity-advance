// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <ugba/ugba.h>

#include "room_game/draw_common.h"
#include "room_game/room_game.h"
#include "room_game/tileset_info.h"
#include "simulation/simulation_building_count.h"
#include "simulation/simulation_transport_anims.h"

#define OUT_MARGIN  8

typedef struct {
    // Coordinates in the map
    int32_t x, y;

    // Coordinates on screen
    int32_t spr_x, spr_y;

    // Final destination of movement
    int32_t end_x, end_y;

    // Number of frames to wait before moving
    int32_t waitframes;

    int8_t is_waiting;

    // 0 = up, 1 = right, 2 = down, 3 = left
    int8_t direction;

    int8_t visible;

    int8_t enabled;
} train_info;

static train_info trains[SIMULATION_MAX_TRAINS];

#define TRAIN_NUM_DIRECTIONS            4

#define TRAIN_DIRECTION_INVALID         -1

static int TrainIsVisible(int x, int y)
{
    int minx = 0 - OUT_MARGIN;
    int maxx = 240 + OUT_MARGIN;
    int miny = 0 - OUT_MARGIN;
    int maxy = 160 + OUT_MARGIN;

    if ((x < minx) || (x > maxx) || (y < miny) || (y > maxy))
        return 0;
    return 1;
}

static int IsTrainTrack(int x, int y)
{
    if ((x < 0) || (x >= CITY_MAP_WIDTH) || (y < 0) || (y >= CITY_MAP_HEIGHT))
        return 0;

    uint16_t type = CityMapGetTypeNoBoundCheck(x, y);

    if (type & TYPE_HAS_TRAIN)
        return 1;

    return 0;
}

static int IsTrainTrackCrossing(int x, int y)
{
    UGBA_Assert((x >= 0) || (x < CITY_MAP_WIDTH) ||
                (y >= 0) || (y < CITY_MAP_HEIGHT));

    uint16_t tile = CityMapGetTile(x, y);

    if ((tile == T_TRAIN_TRB) || (tile == T_TRAIN_LRB) ||
        (tile == T_TRAIN_TLB) || (tile == T_TRAIN_TLR) ||
        (tile == T_TRAIN_TLRB))
    {
        return 1;
    }

    return 0;
}

static void TrainsRefreshOAM(void)
{
    for (int i = 0; i < SIMULATION_MAX_TRAINS; i++)
    {
        train_info *p = &trains[i];

        if (p->enabled && p->visible)
        {
            const struct {
                int tile;
                int hflip;
                int vflip;
            } obj_info[] = {
                { TRAIN_TILE_INDEX_BASE + 1, 0, 0 }, // Up
                { TRAIN_TILE_INDEX_BASE + 0, 1, 0 }, // Right
                { TRAIN_TILE_INDEX_BASE + 1, 0, 1 }, // Down
                { TRAIN_TILE_INDEX_BASE + 0, 0, 0 }, // Left
            };

            int tile = obj_info[p->direction].tile;
            int hflip = obj_info[p->direction].hflip;
            int vflip = obj_info[p->direction].vflip;

            OBJ_RegularInit(TRAIN_SPR_OBJ_BASE + i, p->spr_x, p->spr_y,
                            OBJ_SIZE_8x8, OBJ_16_COLORS, TRANSPORT_PALETTE,
                            tile);
            OBJ_PrioritySet(TRAIN_SPR_OBJ_BASE + i, 2);
            OBJ_RegularHFlipSet(TRAIN_SPR_OBJ_BASE + i, hflip);
            OBJ_RegularVFlipSet(TRAIN_SPR_OBJ_BASE + i, vflip);
        }
        else
        {
            OBJ_RegularInit(TRAIN_SPR_OBJ_BASE + i, 0, 160,
                    OBJ_SIZE_8x8, OBJ_16_COLORS, 0, 0);
            OBJ_RegularEnableSet(TRAIN_SPR_OBJ_BASE + i, 0);
        }
    }
}

static const int8_t train_dir_increment[][2] = {
    {  0, -1 }, // 0 - Up
    {  1,  0 }, // 2 - Right
    {  0,  1 }, // 4 - Down
    { -1,  0 }, // 6 - Left
};

static void TrainStartMovement(train_info *p)
{
    int startx = p->x / 8;
    int starty = p->y / 8;

    // Try to get a new random direction for the train, one different from going
    // backwards. If that's not possible, go backwards.

    int direction = -1;
    int tries = 0;

    while (1)
    {
        direction = rand() % TRAIN_NUM_DIRECTIONS;

        // This can't be optimized to the following because the value
        // TRAIN_DIRECTION_INVALID would also be a valid value in the formula:
        //   (direction == (p->direction + 2) % 4)
        if ((direction == 0) && (p->direction == 2))
            continue;
        if ((direction == 1) && (p->direction == 3))
            continue;
        if ((direction == 2) && (p->direction == 0))
            continue;
        if ((direction == 3) && (p->direction == 1))
            continue;

        int deltax = train_dir_increment[direction][0];
        int deltay = train_dir_increment[direction][1];

        if (IsTrainTrack(startx + deltax, starty + deltay))
            break;

        tries++;
        if (tries == 10)
        {
            // Give up. If the train hasn't found a valid direction, just go
            // backwards
            direction = (p->direction + 2) % 4;
            break;
        }
    }

    p->direction = direction;

    // Calculate distance to advance

    int deltax = train_dir_increment[direction][0];
    int deltay = train_dir_increment[direction][1];

    // At least, it is possible to advance one tile
    int steps = 1;
    int endx = startx + deltax;
    int endy = starty + deltay;

    while (1)
    {
        if (IsTrainTrack(endx + deltax, endy + deltay) == 0)
            break;

        endx += deltax;
        endy += deltay;
        steps++;

        if (IsTrainTrackCrossing(endx, endy))
        {
            // This is a crossing, stop so that the train can change directions
            break;
        }
    }

    p->end_x = endx * 8;
    p->end_y = endy * 8;
}

static void TrainSpawn(train_info *p)
{
    int scx, scy;
    Room_Game_GetCurrentScroll(&scx, &scy);

    int r = rand();

    // Spawn at a train track

    building_count_info *info = Simulation_CountBuildingsGet();
    int train_tracks = info->train_tracks;

    if (train_tracks < 0)
        return;

    int the_track = r % train_tracks;

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t tile, type;
            CityMapGetTypeAndTileUnsafe(i, j, &tile, &type);

            if ((type & TYPE_HAS_TRAIN) == 0)
                continue;

            if (the_track > 0)
            {
                the_track--;
                continue;
            }

            p->x = i * 8;
            p->y = j * 8;
            goto track_found;
        }
    }
track_found:

    // Make the train wait for a bit before starting to move

    p->is_waiting = 1;
    p->waitframes = 0;

    // Enable it only if it has spawned correctly

    p->spr_x = p->x - scx;
    p->spr_y = p->y - scy;
    p->visible = TrainIsVisible(p->spr_x, p->spr_y);
    p->enabled = 1;
}

void TrainsReset(void)
{
    memset(trains, 0, sizeof(trains));

    TrainsRefreshOAM();
}

void TrainsHide(void)
{
    for (int i = 0; i < SIMULATION_MAX_TRAINS; i++)
    {
        train_info *p = &trains[i];

        if (p->enabled == 0)
            continue;

        p->visible = 0;
    }

    TrainsRefreshOAM();
}

void TrainsShow(void)
{
    for (int i = 0; i < SIMULATION_MAX_TRAINS; i++)
    {
        train_info *p = &trains[i];

        if (p->enabled == 0)
            continue;

        p->visible = TrainIsVisible(p->spr_x, p->spr_y);
    }

    TrainsRefreshOAM();
}

void TrainsVBLHandle(void)
{
    for (int i = 0; i < SIMULATION_MAX_TRAINS; i++)
    {
        train_info *p = &trains[i];

        if (p->enabled == 0)
            continue;

        // Move train

        if (p->is_waiting)
            continue;

        int deltax = train_dir_increment[p->direction][0];
        int deltay = train_dir_increment[p->direction][1];

        p->x += deltax;
        p->spr_x += deltax;
        p->y += deltay;
        p->spr_y += deltay;

        // Stop if the destination is reached

        if ((p->x == p->end_x) && (p->y == p->end_y))
        {
            p->is_waiting = 1;
            p->waitframes = 0;
        }
    }

    TrainsRefreshOAM();
}

void TrainsHandle(void)
{
    int scx, scy;
    Room_Game_GetCurrentScroll(&scx, &scy);

    int active_trains = 0;

    for (int i = 0; i < SIMULATION_MAX_TRAINS; i++)
    {
        train_info *p = &trains[i];

        if (p->enabled == 0)
            continue;

        // If the train leaves the map, mark it as disabled

        int minx = 0 - OUT_MARGIN;
        int maxx = (64 * 8) + OUT_MARGIN;
        int miny = 0 - OUT_MARGIN;
        int maxy = (64 * 8) + OUT_MARGIN;

        if ((p->x < minx) || (p->x > maxx) || (p->y < miny) || (p->y > maxy))
        {
            p->enabled = 0;
            continue;
        }

        // If the train isn't in a train track, disable it

        if (IsTrainTrack(p->x / 8, p->y / 8) == 0)
        {
            p->enabled = 0;
            continue;
        }

        // Update directions

        if (p->is_waiting)
        {
            if (p->waitframes <= 0)
            {
                TrainStartMovement(p);
                p->is_waiting = 0;
            }
            else
            {
                p->waitframes--;
            }
        }

        // Refresh sprite coordinates
        p->spr_x = p->x - scx;
        p->spr_y = p->y - scy;

        p->visible = TrainIsVisible(p->spr_x, p->spr_y);

        active_trains++;
    }

    // Spawn trains until the number of enabled trains is equal to the number of
    // train tracks divided by 32

    building_count_info *info = Simulation_CountBuildingsGet();
    int max_trains = info->train_tracks / 64;

    if (max_trains > SIMULATION_MAX_TRAINS)
        max_trains = SIMULATION_MAX_TRAINS;

    for (int i = 0; i < SIMULATION_MAX_TRAINS; i++)
    {
        if (active_trains >= max_trains)
            break;

        train_info *p = &trains[i];

        if (p->enabled != 0)
            continue;

        // When the train is created, allow any direction
        p->direction = TRAIN_DIRECTION_INVALID;

        TrainSpawn(p);

        active_trains++;
    }

    // Done

    TrainsRefreshOAM();
}

void TrainsHandleScroll(int deltax, int deltay)
{
    for (int i = 0; i < SIMULATION_MAX_TRAINS; i++)
    {
        train_info *p = &trains[i];

        if (p->enabled)
        {
            p->spr_x += deltax;
            p->spr_y += deltay;
        }
    }

    TrainsRefreshOAM();
}
