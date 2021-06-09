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

    // Final destination of movement
    int32_t end_x, end_y;

    // Number of frames to wait before moving
    int32_t waitframes;

    int8_t is_waiting;

    // 0 = up, 1 = up + right, 2 = right, etc
    int8_t direction;

    int8_t visible;

    int8_t enabled;
} boat_info;

static boat_info boats[SIMULATION_MAX_BOATS];

// Max distance to travel in one go
#define BOAT_MAX_CONTINUOUS_DISTANCE    5
#define BOAT_NUM_DIRECTIONS             8 // 4 directions + 4 diagonals

// Number of handler steps to wait after stopping to start moving.
#define BOAT_MOVE_WAIT_RANGE    128 // Power of 2
#define BOAT_MOVE_WAIT_MIN      0   // Not needed to be a power of 2

static int BoatIsVisible(int x, int y)
{
    int minx = 0 - OUT_MARGIN;
    int maxx = 240 + OUT_MARGIN;
    int miny = 0 - OUT_MARGIN;
    int maxy = 160 + OUT_MARGIN;

    if ((x < minx) || (x > maxx) || (y < miny) || (y > maxy))
        return 0;
    return 1;
}

static int IsOpenWater(int x, int y)
{
    if ((x < 0) || (x >= CITY_MAP_WIDTH) || (y < 0) || (y >= CITY_MAP_HEIGHT))
        return 0;

    uint16_t tile = CityMapGetTile(x, y);

    if ((tile == T_WATER) || (tile == T_WATER_EXTRA))
        return 1;

    return 0;
}

static void BoatsRefreshOAM(void)
{
    for (int i = 0; i < SIMULATION_MAX_BOATS; i++)
    {
        boat_info *p = &boats[i];

        if (p->enabled && p->visible)
        {
            const struct {
                int tile;
                int hflip;
                int vflip;
            } obj_info[] = {
                { BOAT_TILE_INDEX_BASE + 0, 0, 0 }, // Up
                { BOAT_TILE_INDEX_BASE + 1, 1, 0 }, // Up + Right
                { BOAT_TILE_INDEX_BASE + 2, 1, 0 }, // Right
                { BOAT_TILE_INDEX_BASE + 1, 1, 1 }, // Down + Right
                { BOAT_TILE_INDEX_BASE + 0, 0, 1 }, // Down
                { BOAT_TILE_INDEX_BASE + 1, 0, 1 }, // Down + Left
                { BOAT_TILE_INDEX_BASE + 2, 0, 0 }, // Left
                { BOAT_TILE_INDEX_BASE + 1, 0, 0 }, // Up + Left
            };

            int tile = obj_info[p->direction].tile;
            int hflip = obj_info[p->direction].hflip;
            int vflip = obj_info[p->direction].vflip;

            OBJ_RegularInit(BOAT_SPR_OBJ_BASE + i, p->spr_x, p->spr_y,
                            OBJ_SIZE_8x8, OBJ_16_COLORS, TRANSPORT_PALETTE,
                            tile);
            OBJ_PrioritySet(BOAT_SPR_OBJ_BASE + i, 2);
            OBJ_RegularHFlipSet(BOAT_SPR_OBJ_BASE + i, hflip);
            OBJ_RegularVFlipSet(BOAT_SPR_OBJ_BASE + i, vflip);
        }
        else
        {
            OBJ_RegularInit(BOAT_SPR_OBJ_BASE + i, 0, 160,
                    OBJ_SIZE_8x8, OBJ_16_COLORS, 0, 0);
            OBJ_RegularEnableSet(BOAT_SPR_OBJ_BASE + i, 0);
        }
    }
}

static const int8_t boat_dir_increment[][2] = {
    {  0, -1 }, // 0 - Up
    {  1, -1 }, // 1 - Up Right
    {  1,  0 }, // 2 - Right
    {  1,  1 }, // 3 - Right Down
    {  0,  1 }, // 4 - Down
    { -1,  1 }, // 5 - Down Left
    { -1,  0 }, // 6 - Left
    { -1, -1 }, // 7 - Left Top
};

static void BoatStartMovement(boat_info *p)
{
    int startx = p->x / 8;
    int starty = p->y / 8;

    // Get a random direction for the boat

    int direction = -1;
    int tries = 0;

    while (1)
    {
        direction = rand_fast() % BOAT_NUM_DIRECTIONS;

        int deltax = boat_dir_increment[direction][0];
        int deltay = boat_dir_increment[direction][1];

        if (IsOpenWater(startx + deltax, starty + deltay))
            break;

        tries++;
        if (tries == 10)
        {
            // Give up
            return;
        }
    }

    p->direction = direction;

    // Calculate distance to advance

    int deltax = boat_dir_increment[direction][0];
    int deltay = boat_dir_increment[direction][1];

    // At least, it is possible to advance one tile
    int steps = 1;
    int endx = startx + deltax;
    int endy = starty + deltay;

    while (1)
    {
        if (IsOpenWater(endx + deltax, endy + deltay) == 0)
            break;

        endx += deltax;
        endy += deltay;
        steps++;

        if (steps == BOAT_MAX_CONTINUOUS_DISTANCE)
            break;
    }

    p->end_x = endx * 8;
    p->end_y = endy * 8;
}

static void BoatSpawn(boat_info *p)
{
    int scx, scy;
    Room_Game_GetCurrentScroll(&scx, &scy);

    int r = rand_fast();

    // Spawn at a dock

    // TODO: Maybe spawn at border of map

    building_count_info *info = Simulation_CountBuildingsGet();
    int docks = info->docks;

    UGBA_Assert(docks > 0);

    int the_dock = r % docks;

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t tile, type;
            CityMapGetTypeAndTileUnsafe(i, j, &tile, &type);

            if (type != TYPE_DOCK)
                continue;

            if (the_dock > 0)
            {
                the_dock--;
                continue;
            }

            if (tile == T_PORT_WATER_L)
            {
                if (i == 0)
                    return;
                i--;
            }
            else if (tile == T_PORT_WATER_R)
            {
                if (i == (CITY_MAP_WIDTH - 1))
                    return;
                i++;
            }
            else if (tile == T_PORT_WATER_D)
            {
                if (j == (CITY_MAP_HEIGHT - 1))
                    return;
                j++;
            }
            else if (tile == T_PORT_WATER_U)
            {
                if (j == 0)
                    return;
                j--;
            }

            tile = CityMapGetTile(i, j);

            if ((tile != T_WATER) && (tile != T_WATER_EXTRA))
                return;

            p->x = i * 8;
            p->y = j * 8;
            goto dock_found;
        }
    }
dock_found:

    // Make the boat wait for a bit before starting to move

    p->is_waiting = 1;
    p->waitframes = BOAT_MOVE_WAIT_MIN + (rand_fast() % BOAT_MOVE_WAIT_RANGE);

    // Enable it only if it has spawned correctly

    p->spr_x = p->x - scx;
    p->spr_y = p->y - scy;
    p->visible = BoatIsVisible(p->spr_x, p->spr_y);
    p->enabled = 1;
}

void BoatsReset(void)
{
    memset(boats, 0, sizeof(boats));

    BoatsRefreshOAM();
}

void BoatsHide(void)
{
    for (int i = 0; i < SIMULATION_MAX_BOATS; i++)
    {
        boat_info *p = &boats[i];

        if (p->enabled == 0)
            continue;

        p->visible = 0;
    }

    BoatsRefreshOAM();
}

void BoatsShow(void)
{
    for (int i = 0; i < SIMULATION_MAX_BOATS; i++)
    {
        boat_info *p = &boats[i];

        if (p->enabled == 0)
            continue;

        p->visible = BoatIsVisible(p->spr_x, p->spr_y);
    }

    BoatsRefreshOAM();
}

void BoatsVBLHandle(void)
{
    for (int i = 0; i < SIMULATION_MAX_BOATS; i++)
    {
        boat_info *p = &boats[i];

        if (p->enabled == 0)
            continue;

        // Move boat

        if (p->is_waiting)
            continue;

        int deltax = boat_dir_increment[p->direction][0];
        int deltay = boat_dir_increment[p->direction][1];

        p->x += deltax;
        p->spr_x += deltax;
        p->y += deltay;
        p->spr_y += deltay;

        // Stop if the destination is reached

        if ((p->x == p->end_x) && (p->y == p->end_y))
        {
            p->is_waiting = 1;
            p->waitframes = BOAT_MOVE_WAIT_MIN
                          + (rand_fast() % BOAT_MOVE_WAIT_RANGE);
        }
    }

    BoatsRefreshOAM();
}

void BoatsHandle(void)
{
    int scx, scy;
    Room_Game_GetCurrentScroll(&scx, &scy);

    int active_boats = 0;

    for (int i = 0; i < SIMULATION_MAX_BOATS; i++)
    {
        boat_info *p = &boats[i];

        if (p->enabled == 0)
            continue;

        // If the boat leaves the map, mark it as disabled

        int minx = 0 - OUT_MARGIN;
        int maxx = (64 * 8) + OUT_MARGIN;
        int miny = 0 - OUT_MARGIN;
        int maxy = (64 * 8) + OUT_MARGIN;

        if ((p->x < minx) || (p->x > maxx) || (p->y < miny) || (p->y > maxy))
        {
            p->enabled = 0;
            continue;
        }

        // If the boat isn't in the water, disable it

        if (IsOpenWater(p->x / 8, p->y / 8) == 0)
        {
            p->enabled = 0;
            continue;
        }

        // Update directions

        if (p->is_waiting)
        {
            if (p->waitframes <= 0)
            {
                BoatStartMovement(p);
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

        p->visible = BoatIsVisible(p->spr_x, p->spr_y);

        active_boats++;
    }

    // Spawn boats until the number of enabled boats is equal to half of the
    // number of docks

    building_count_info *info = Simulation_CountBuildingsGet();
    int max_boats = info->docks / 2;

    if (max_boats > SIMULATION_MAX_BOATS)
        max_boats = SIMULATION_MAX_BOATS;

    for (int i = 0; i < SIMULATION_MAX_BOATS; i++)
    {
        if (active_boats >= max_boats)
            break;

        boat_info *p = &boats[i];

        if (p->enabled != 0)
            continue;

        BoatSpawn(p);

        active_boats++;
    }

    // Done

    BoatsRefreshOAM();
}

void BoatsHandleScroll(int deltax, int deltay)
{
    for (int i = 0; i < SIMULATION_MAX_BOATS; i++)
    {
        boat_info *p = &boats[i];

        if (p->enabled)
        {
            p->spr_x += deltax;
            p->spr_y += deltay;
        }
    }

    BoatsRefreshOAM();
}
