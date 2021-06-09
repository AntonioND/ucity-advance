// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <ugba/ugba.h>

#include "random.h"
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

    // 0 = up, 1 = up + right, 2 = right, etc
    int8_t direction;
    int8_t direction_change_countdown;

    int8_t visible;

    int8_t enabled;
} plane_info;

static plane_info planes[SIMULATION_MAX_PLANES];

#define PLANE_NUM_DIRECTIONS        8 // 4 directions + 4 diagonals
#define PLANE_TAKEOFF_DIRECTION     2 // Right

// Number of movement steps to change direction. The minimum should be the
// length of the runway of the airport so that it looks ok when a plane takes
// off.
#define PLANE_CHANGE_DIR_RANGE  128 // Power of 2
#define PLANE_CHANGE_DIR_MIN    60  // Not needed to be a power of 2

static int PlaneIsVisible(int x, int y)
{
    int minx = 0 - OUT_MARGIN;
    int maxx = 240 + OUT_MARGIN;
    int miny = 0 - OUT_MARGIN;
    int maxy = 160 + OUT_MARGIN;

    if ((x < minx) || (x > maxx) || (y < miny) || (y > maxy))
        return 0;
    return 1;
}

static void PlanesRefreshOAM(void)
{
    for (int i = 0; i < SIMULATION_MAX_PLANES; i++)
    {
        plane_info *p = &planes[i];

        if (p->enabled && p->visible)
        {
            const struct {
                int tile;
                int hflip;
                int vflip;
            } obj_info[] = {
                { PLANE_TILE_INDEX_BASE + 0, 0, 0 }, // Up
                { PLANE_TILE_INDEX_BASE + 1, 1, 0 }, // Up + Right
                { PLANE_TILE_INDEX_BASE + 2, 1, 0 }, // Right
                { PLANE_TILE_INDEX_BASE + 1, 1, 1 }, // Down + Right
                { PLANE_TILE_INDEX_BASE + 0, 0, 1 }, // Down
                { PLANE_TILE_INDEX_BASE + 1, 0, 1 }, // Down + Left
                { PLANE_TILE_INDEX_BASE + 2, 0, 0 }, // Left
                { PLANE_TILE_INDEX_BASE + 1, 0, 0 }, // Up + Left
            };

            int tile = obj_info[p->direction].tile;
            int hflip = obj_info[p->direction].hflip;
            int vflip = obj_info[p->direction].vflip;

            OBJ_RegularInit(PLANE_SPR_OBJ_BASE + i, p->spr_x, p->spr_y,
                            OBJ_SIZE_8x8, OBJ_16_COLORS, TRANSPORT_PALETTE,
                            tile);
            OBJ_PrioritySet(PLANE_SPR_OBJ_BASE + i, 2);
            OBJ_RegularHFlipSet(PLANE_SPR_OBJ_BASE + i, hflip);
            OBJ_RegularVFlipSet(PLANE_SPR_OBJ_BASE + i, vflip);
        }
        else
        {
            OBJ_RegularInit(PLANE_SPR_OBJ_BASE + i, 0, 160,
                    OBJ_SIZE_8x8, OBJ_16_COLORS, 0, 0);
            OBJ_RegularEnableSet(PLANE_SPR_OBJ_BASE + i, 0);
        }
    }
}

static void PlaneSpawn(plane_info *p)
{
    int scx, scy;
    Room_Game_GetCurrentScroll(&scx, &scy);

    int r = rand_fast();

    if (r & (1 << 2))
    {
        // Spawn at an airport

        building_count_info *info = Simulation_CountBuildingsGet();
        int airports = info->airports;

        UGBA_Assert(airports > 0);

        int the_airport = (r >> 3) % airports;

        for (int j = 0; j < CITY_MAP_HEIGHT; j++)
        {
            for (int i = 0; i < CITY_MAP_WIDTH; i++)
            {
                if (CityMapGetTile(i, j) == T_AIRPORT_RUNWAY)
                {
                    if (the_airport == 0)
                    {
                        p->x = i * 8;
                        p->y = j * 8;
                        goto airport_found;
                    }
                    the_airport--;
                }
            }
        }
airport_found:

        p->direction = PLANE_TAKEOFF_DIRECTION;
    }
    else
    {
        // Spawn at border

        // Randomize direction a bit (diagonals allowed)
        int dir = ((r >> 3) & 3) - 1;
        if (dir == 2)
            dir = 0;

        switch (r & 3)
        {
            case 0: // Up
                p->x = rand_fast() % (CITY_MAP_WIDTH * 8);
                p->y = -OUT_MARGIN + 1;
                p->direction = 4 + dir; // Down
                break;
            case 1: // Right
                p->x = (CITY_MAP_WIDTH * 8) + OUT_MARGIN - 1;
                p->y = rand_fast() % (CITY_MAP_HEIGHT * 8);
                p->direction = 6 + dir; // Left
                break;
            case 2: // Down
                p->x = rand_fast() % (CITY_MAP_WIDTH * 8);
                p->y = (CITY_MAP_HEIGHT * 8) + OUT_MARGIN - 1;
                p->direction = 0 + dir; // Up
                break;
            case 3: // Left
                p->x = -OUT_MARGIN + 1;
                p->y = rand_fast() % (CITY_MAP_HEIGHT * 8);
                p->direction = 2 + dir; // Right
                break;
            default:
                break;
        }
    }

    p->direction_change_countdown = (rand_fast() & (PLANE_CHANGE_DIR_RANGE - 1))
                                  + PLANE_CHANGE_DIR_MIN;
    p->spr_x = p->x - scx;
    p->spr_y = p->y - scy;
    p->visible = PlaneIsVisible(p->spr_x, p->spr_y);
    p->enabled = 1;
}

void PlanesReset(void)
{
    memset(planes, 0, sizeof(planes));

    PlanesRefreshOAM();
}

void PlanesHide(void)
{
    for (int i = 0; i < SIMULATION_MAX_PLANES; i++)
    {
        plane_info *p = &planes[i];

        if (p->enabled == 0)
            continue;

        p->visible = 0;
    }

    PlanesRefreshOAM();
}

void PlanesShow(void)
{
    for (int i = 0; i < SIMULATION_MAX_PLANES; i++)
    {
        plane_info *p = &planes[i];

        if (p->enabled == 0)
            continue;

        p->visible = PlaneIsVisible(p->spr_x, p->spr_y);
    }

    PlanesRefreshOAM();
}

void PlanesVBLHandle(void)
{
    for (int i = 0; i < SIMULATION_MAX_PLANES; i++)
    {
        plane_info *p = &planes[i];

        if (p->enabled == 0)
            continue;

        // Move plane

        const int8_t dir_increment[][2] = {
            {  0, -1 }, // 0 - Up
            {  1, -1 }, // 1 - Up Right
            {  1,  0 }, // 2 - Right
            {  1,  1 }, // 3 - Right Down
            {  0,  1 }, // 4 - Down
            { -1,  1 }, // 5 - Down Left
            { -1,  0 }, // 6 - Left
            { -1, -1 }, // 7 - Left Top
        };

        int deltax = dir_increment[p->direction][0];
        int deltay = dir_increment[p->direction][1];

        p->x += deltax;
        p->spr_x += deltax;
        p->y += deltay;
        p->spr_y += deltay;
    }

    PlanesRefreshOAM();
}

void PlanesHandle(void)
{
    int scx, scy;
    Room_Game_GetCurrentScroll(&scx, &scy);

    int active_planes = 0;

    for (int i = 0; i < SIMULATION_MAX_PLANES; i++)
    {
        plane_info *p = &planes[i];

        if (p->enabled == 0)
            continue;

        // Update directions

        p->direction_change_countdown--;
        if (p->direction_change_countdown == 0)
        {
            int r = rand_fast();
            p->direction_change_countdown = (r & (PLANE_CHANGE_DIR_RANGE - 1))
                                          + PLANE_CHANGE_DIR_MIN;
            p->direction += r & (1 << 16) ? 1 : -1;
        }

        // If the plane leaves the map, mark it as disabled

        int minx = 0 - OUT_MARGIN;
        int maxx = (64 * 8) + OUT_MARGIN;
        int miny = 0 - OUT_MARGIN;
        int maxy = (64 * 8) + OUT_MARGIN;

        if ((p->x < minx) || (p->x > maxx) || (p->y < miny) || (p->y > maxy))
        {
            p->enabled = 0;
        }
        else
        {
            active_planes++;

            // Refresh sprite coordinates
            p->spr_x = p->x - scx;
            p->spr_y = p->y - scy;

            p->visible = PlaneIsVisible(p->spr_x, p->spr_y);
        }
    }

    // Spawn planes until the number of enabled planes is the same as the number
    // of airports (up to a limit). Planes spawn either at the airport or the
    // edge of the map.

    building_count_info *info = Simulation_CountBuildingsGet();
    int airports = info->airports;

    if (airports > SIMULATION_MAX_PLANES)
        airports = SIMULATION_MAX_PLANES;

    for (int i = 0; i < SIMULATION_MAX_PLANES; i++)
    {
        if (active_planes >= airports)
            break;

        plane_info *p = &planes[i];

        if (p->enabled != 0)
            continue;

        PlaneSpawn(p);

        active_planes++;
    }

    // Done

    PlanesRefreshOAM();
}

void PlanesHandleScroll(int deltax, int deltay)
{
    for (int i = 0; i < SIMULATION_MAX_PLANES; i++)
    {
        plane_info *p = &planes[i];

        if (p->enabled)
        {
            p->spr_x += deltax;
            p->spr_y += deltay;
        }
    }

    PlanesRefreshOAM();
}
