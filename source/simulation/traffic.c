// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <stdint.h>
#include <string.h>

#include "map_utils.h"
#include "room_game/building_info.h"
#include "room_game/draw_common.h"
#include "room_game/room_game.h"
#include "room_game/text_messages.h"
#include "room_game/tileset_info.h"
#include "simulation/building_density.h"
#include "simulation/queue.h"
#include "simulation/building_count.h"
#include "simulation/happiness.h"

#define TRAFFIC_MAX_LEVEL       (256 / 6) // Max level of adequate traffic
#define TRAFFIC_JAM_MAX_TILES   30 // Max percent of tiles with high traffic

EWRAM_BSS static uint8_t traffic_map[CITY_MAP_HEIGHT * CITY_MAP_WIDTH];
EWRAM_BSS static uint8_t scratch_map[CITY_MAP_HEIGHT * CITY_MAP_WIDTH];

// Amount of tiles with traffic jams.
int simulation_traffic_jam_num_tiles;
int simulation_traffic_jam_num_tiles_percent;

// Remaining density in the residential building being handled at the moment.
int source_building_remaining_density;

IWRAM_CODE static int min(int a, int b)
{
    if (a < b)
        return a;
    return b;
}

int Simulation_TrafficGetTrafficJamPercent(void)
{
    return simulation_traffic_jam_num_tiles_percent;
}

uint8_t *Simulation_TrafficGetMap(void)
{
    return &traffic_map[0];
}

// Returns remaining density of a building from any tile of it.
IWRAM_CODE static uint8_t *TrafficGetBuildingiRemainingDensityPointer(int x, int y)
{
    uint16_t tile = CityMapGetTile(x, y);

    const city_tile_info *ti = City_Tileset_Entry_Info(tile);

    int ox = x + ti->base_x_delta;
    int oy = y + ti->base_y_delta;

    return &(traffic_map[oy * CITY_MAP_WIDTH + ox]);
}

// Add initial tiles that are next to a residential building. The only allowed
// destinations are road and train tracks tiles.
IWRAM_CODE static void TrafficAddStart(int x, int y)
{
    if ((x < 0) || (x >= CITY_MAP_WIDTH))
        return;

    if ((y < 0) || (y >= CITY_MAP_HEIGHT))
        return;

    uint16_t type = CityMapGetType(x, y);

    if ((type & (TYPE_HAS_ROAD | TYPE_HAS_TRAIN)) == 0)
        return;

    // Add coordinates of destination tile to the queue
    QueueAddPair(x, y);

    // Set initial cost to 1!
    scratch_map[y * CITY_MAP_WIDTH + x] = 1;
}

// Try to add a certain tile to the queue. Only non-residential buildings and
// road/train tracks are allowed.
IWRAM_CODE static void TrafficAdd(int x, int y, int accumulated_cost)
{
    // Check if it is a non-residential building. If so, add to queue
    // immediately.
    //
    // Only buildings can have density != 0, so check if it is different than
    // residential and if density != 0, it can't be field or water. If both
    // conditions are met, ignore direction check and add to queue.
    //
    // If density == 0 it means it is a road or train (if not, the tile wouldn't
    // have been added to the queue). Just check directions and add to queue if
    // the movement is allowed.
    //
    // In any case, if it is added to the queue, write the accumulated cost up
    // to this point to the tile as well if it's not a building. If it's a
    // building it means that we have reached a destination.

    uint16_t type_unmasked = CityMapGetType(x, y);
    uint16_t type = type_unmasked & TYPE_MASK;

    // Ignore residential zones and docks
    if ((type == TYPE_RESIDENTIAL) || (type == TYPE_DOCK))
        return;

    // Check if this is a building or not
    if ((type == TYPE_FIELD) || (type == TYPE_FOREST) || (type == TYPE_WATER))
    {
        // Not a building, add to queue and save accumulated cost if it is a
        // road or train track

        if ((type_unmasked & (TYPE_HAS_ROAD | TYPE_HAS_TRAIN)) == 0)
        {
            // If there are no roads or train, don't add to queue
            return;
        }

        QueueAddPair(x, y);

        scratch_map[y * CITY_MAP_WIDTH + x] = accumulated_cost;

        return;
    }

    // Building, add to queue but don't save accumulated cost.

    QueueAddPair(x, y);
}

IWRAM_CODE static void TrafficTryMoveUp(int x, int y, int accumulated_cost)
{
    // Return if this is in the top row
    if (y == 0)
        return;

    // Check if already handled. If so, check if the new cost is lower
    // than the previous one.

    int destination_cost = scratch_map[(y - 1) * CITY_MAP_WIDTH + x];

    if (destination_cost > 0)
    {
        // This has been handled before. If the cost is lower than the stored one
        // continue. If not, return.
        if (destination_cost < accumulated_cost)
            return;
    }

    TrafficAdd(x, y - 1, accumulated_cost);
}

IWRAM_CODE static void TrafficTryMoveDown(int x, int y, int accumulated_cost)
{
    // Return if this is in the bottom row
    if (y == (CITY_MAP_HEIGHT - 1))
        return;

    // Check if already handled. If so, check if the new cost is lower
    // than the previous one.

    int destination_cost = scratch_map[(y + 1) * CITY_MAP_WIDTH + x];

    if (destination_cost > 0)
    {
        // This has been handled before. If the cost is lower than the stored one
        // continue. If not, return.
        if (destination_cost < accumulated_cost)
            return;
    }

    TrafficAdd(x, y + 1, accumulated_cost);
}

IWRAM_CODE static void TrafficTryMoveLeft(int x, int y, int accumulated_cost)
{
    // Return if this is in the left column
    if (x == 0)
        return;

    // Check if already handled. If so, check if the new cost is lower
    // than the previous one.

    int destination_cost = scratch_map[y * CITY_MAP_WIDTH + (x - 1)];

    if (destination_cost > 0)
    {
        // This has been handled before. If the cost is lower than the stored one
        // continue. If not, return.
        if (destination_cost < accumulated_cost)
            return;
    }

    TrafficAdd(x - 1, y, accumulated_cost);
}

IWRAM_CODE static void TrafficTryMoveRight(int x, int y, int accumulated_cost)
{
    // Return if this is in the right column
    if (x == (CITY_MAP_WIDTH - 1))
        return;

    // Check if already handled. If so, check if the new cost is lower
    // than the previous one.

    int destination_cost = scratch_map[y * CITY_MAP_WIDTH + (x + 1)];

    if (destination_cost > 0)
    {
        // This has been handled before. If the cost is lower than the stored one
        // continue. If not, return.
        if (destination_cost < accumulated_cost)
            return;
    }

    TrafficAdd(x + 1, y, accumulated_cost);
}

// From the specified position, get the current accumulated cost, calculate the
// cost of this tile and add it. If the top cost is not reached, try to expand
// in all directions. Top cost is 255. If it goes to 256 and overflows, it is
// considered to be too far for the car/train to get there.
IWRAM_CODE static void TrafficTryExpand(int x, int y)
{
    const int TILE_TRANSPORT_INFO[] = { // Cost
        [T_ROAD_TB]                 = 12,
        [T_ROAD_TB_1]               = 12,
        [T_ROAD_TB_2]               = 12,
        [T_ROAD_TB_3]               = 12,

        [T_ROAD_LR]                 = 12,
        [T_ROAD_LR_1]               = 12,
        [T_ROAD_LR_2]               = 12,
        [T_ROAD_LR_3]               = 12,

        [T_ROAD_RB]                 = 15,
        [T_ROAD_LB]                 = 15,
        [T_ROAD_TR]                 = 15,
        [T_ROAD_TL]                 = 15,

        [T_ROAD_TRB]                = 18,
        [T_ROAD_LRB]                = 18,
        [T_ROAD_TLB]                = 18,
        [T_ROAD_TLR]                = 18,
        [T_ROAD_TLRB]               = 21,

        [T_ROAD_TB_POWER_LINES]     = 12,
        [T_ROAD_LR_POWER_LINES]     = 12,
        [T_ROAD_TB_BRIDGE]          = 15,
        [T_ROAD_LR_BRIDGE]          = 15,

        [T_TRAIN_TB]                = 6,
        [T_TRAIN_LR]                = 6,
        [T_TRAIN_RB]                = 7,
        [T_TRAIN_LB]                = 7,
        [T_TRAIN_TR]                = 7,
        [T_TRAIN_TL]                = 7,

        [T_TRAIN_TRB]               = 9,
        [T_TRAIN_LRB]               = 9,
        [T_TRAIN_TLB]               = 9,
        [T_TRAIN_TLR]               = 9,
        [T_TRAIN_TLRB]              = 10,

        [T_TRAIN_LR_ROAD]           = 22,
        [T_TRAIN_TB_ROAD]           = 22,

        [T_TRAIN_TB_POWER_LINES]    = 6,
        [T_TRAIN_LR_POWER_LINES]    = 6,
        [T_TRAIN_TB_BRIDGE]         = 7,
        [T_TRAIN_LR_BRIDGE]         = 7,
    };

    // Check if current density is small enough to fit in this road/train track.
    // If adding the density to this tile overflows 256, we can't go through
    // this tile, return.

    int current_trafic = traffic_map[y * CITY_MAP_WIDTH + x];
    int new_traffic = source_building_remaining_density + current_trafic;

    if (new_traffic > 255)
        return;

    // The current traffic level will be used to calculate the cost of going
    // through this tile

    uint16_t tile = CityMapGetTile(x, y);

    int base_cost = TILE_TRANSPORT_INFO[tile];

    // Add traffic in this tile to movement cost to penalize movement through
    // crowded streets

    int real_cost = base_cost + (current_trafic >> 4);

    int accumulated_cost = scratch_map[y * CITY_MAP_WIDTH + x] + real_cost;

    // It's too expensive to move to this tile, skip
    if (accumulated_cost > 255)
        return;

    // The functions will check inside if the tile has already been handled.

    TrafficTryMoveUp(x, y, accumulated_cost);

    TrafficTryMoveRight(x, y, accumulated_cost);

    TrafficTryMoveDown(x, y, accumulated_cost);

    TrafficTryMoveLeft(x, y, accumulated_cost);
}

// Checks bounds, returns 255 if outside the map else the accumulated cost
IWRAM_CODE static int TrafficGetAccumulatedCost(int x, int y)
{
    if ((x < 0) || (x >= CITY_MAP_WIDTH) ||
        (y < 0) || (y >= CITY_MAP_HEIGHT))
    {
        // Very high value so that there will always be a smaller one in any
        // other neighbouring tile
        return 255;
    }

    return scratch_map[y * CITY_MAP_WIDTH + x];
}

// Recursively find origin of traffic and increase traffic in traffic map.
IWRAM_CODE static void TrafficRetraceStep(int x, int y, int amount_of_traffic)
{
    if ((x < 0) || (x >= CITY_MAP_WIDTH))
        return;
    if ((y < 0) || (y >= CITY_MAP_HEIGHT))
        return;

    uint16_t type = CityMapGetType(x, y);

    if (type & (TYPE_HAS_ROAD | TYPE_HAS_TRAIN))
    {
        int result_traffic = traffic_map[y * CITY_MAP_WIDTH + x];
        result_traffic += amount_of_traffic;
        if (result_traffic > 255)
            result_traffic = 255;
        traffic_map[y * CITY_MAP_WIDTH + x] = result_traffic;
    }

    // If the cost of this tile is 1 it is the initial one, return!

    int scratch = scratch_map[y * CITY_MAP_WIDTH + x];
    if (scratch == 1)
        return;

    // This is not the start. Find neighbour with lowest cost in the scratch
    // map, but that doesn't have cost 0.

    int cost_up = TrafficGetAccumulatedCost(x, y - 1);
    int cost_down = TrafficGetAccumulatedCost(x, y + 1);
    int cost_left = TrafficGetAccumulatedCost(x - 1, y);
    int cost_right = TrafficGetAccumulatedCost(x + 1, y);

    if (cost_up == 0)
        cost_up = 255;
    if (cost_down == 0)
        cost_down = 255;
    if (cost_left == 0)
        cost_left = 255;
    if (cost_right == 0)
        cost_right = 255;

    int cost_min = min(min(cost_up, cost_down), min(cost_left, cost_right));

    int new_x;
    int new_y;

    if (cost_min == cost_up)
    {
        new_x = x;
        new_y = y - 1;
    }
    else if (cost_min == cost_down)
    {
        new_x = x;
        new_y = y + 1;
    }
    else if (cost_min == cost_left)
    {
        new_x = x - 1;
        new_y = y;
    }
    else if (cost_min == cost_right)
    {
        new_x = x + 1;
        new_y = y;
    }
    else
    {
        UGBA_Assert(0);
        return;
    }

    // Recursive call
    TrafficRetraceStep(new_x, new_y, amount_of_traffic);
}

// When calling this function for the first time in the simulation step the
// caller must ensure that every destination building has its max population
// density in its top left tile. This function will reduce them as needed so
// that the next time it is called the previous call would be taken into
// account.
//
// The coordinates given to it are the top left corner of the building.
IWRAM_CODE static void Simulation_TrafficHandleSource(int x, int y)
{
    // Get density of this building
    // ----------------------------

    // Get the density of this building (source) and save it to a variable
    // that will be decreased in the queue loop below. This doesn't need to
    // be saved to the map.

    uint16_t tile = CityMapGetTile(x, y);
    const city_tile_density_info *di = CityTileDensityInfo(tile);

    // R tiles have population 0. Actual residential buildings have population.
    if (di->population == 0)
        return;

    // Save it to a variable and start!

    source_building_remaining_density = di->population;

    // Get dimensions of this building
    // -------------------------------

    const city_tile_info *ti = City_Tileset_Entry_Info(tile);

    int ox = x + ti->base_x_delta;
    int oy = y + ti->base_y_delta;

    uint16_t origin_tile = CityMapGetTile(ox, oy);

    const building_info *bi = Get_BuildingFromBaseTile(origin_tile);
    int w = bi->width;
    int h = bi->height;

    // Flag as handled (density = 1)
    // -----------------------------

    // The top left tile will be overwritten at the end of this function with
    // the remaining population to travel (or 0 if everyone reached a valid
    // destination).

    for (int j = oy; j < (oy + h); j++)
    {
        for (int i = ox; i < (ox + w); i++)
            traffic_map[j * CITY_MAP_WIDTH + i] = 1;
    }

    // Init queue and expansion map
    // ----------------------------

    QueueInit();

    memset(scratch_map, 0, sizeof(scratch_map));

    // Add neighbours of this building source of traffic to the queue
    // --------------------------------------------------------------

    // Top row
    for (int i = ox; i < (ox + w); i++)
        TrafficAddStart(i, oy - 1);

    // Bottom row
    for (int i = ox; i < (ox + w); i++)
        TrafficAddStart(i, oy + h);

    // Left column
    for (int j = oy; j < (oy + h); j++)
        TrafficAddStart(ox - 1, j);

    // Right column
    for (int j = oy; j < (oy + h); j++)
        TrafficAddStart(ox + w, j);

    // While queue is not empty, expand
    // --------------------------------

    while (1)
    {
        // In short:
        //
        // 1) Check that there is population that needs to continue traveling.
        //
        // 2) Check that there are tiles to handle.
        //
        // 3) Get tile coordinates to handle
        //
        // 4) Read tile type.
        //
        //    - If road => expand
        //    - If building =>
        //      - Check building remaining density
        //      - Reduce the source density by that amount or reduce the
        //        destination amount (depending on which one is higher)

        // Check if remaining source density is 0. If so, exit.
        if (source_building_remaining_density == 0)
            break;

        // Check if there are tiles left to handle. If not, exit.
        if (QueueIsEmpty())
            break;

        // Get tile coordinates and type.

        uint16_t ex, ey;
        QueueGetPair(&ex, &ey);

        uint16_t type = CityMapGetType(ex, ey);

        if (type & (TYPE_HAS_ROAD | TYPE_HAS_TRAIN))
        {
            // This is a road or train tracks. Expand and continue to next tile.
            TrafficTryExpand(ex, ey);
            continue;
        }

        // If this is not a road or train tracks, it must be a building, and not
        // a residential one because the expand functions wouldn't allow that.
        //
        // Check if it has enough remaining density to accept more population.
        // If there is some population left in the tile it means that it can
        // accept more population. Reduce it as much as possible and continue in
        // next tile obtained from the queue with the remaining population.
        //
        // After that, retrace steps to increase traffic in all tiles used to
        // get to this building (using the population that has actually arrived
        // to the destination building).

        uint8_t *ptr = TrafficGetBuildingiRemainingDensityPointer(ex, ey);

        int remaining_density = *ptr;

        if (remaining_density == 0)
        {
            // Destination is full
            continue;
        }

        int spent_density =
                    min(remaining_density, source_building_remaining_density);

        // Subtract from both places

        remaining_density -= spent_density;
        source_building_remaining_density -= spent_density;

        *ptr = remaining_density;

        // Now, retrace steps to increase traffic of each tile used to get
        // to this building in the TRAFFIC map!

        TrafficRetraceStep(ex, ey, spent_density);
    }

    // If there is remaining density, restore it to the source building
    // ----------------------------------------------------------------

    // This means that the people from this building will be unhappy!

    // The same happens for other buildings, if its final density is not 0 it
    // means that this building doesn't get all the people it needs for working!

    traffic_map[oy * CITY_MAP_WIDTH + ox] = source_building_remaining_density;

    // End of this building
}

IWRAM_CODE static void Simulation_TrafficSetTileOkFlag(void)
{
    // - For roads and train, make sure that the traffic is below a certain
    //   threshold.
    //
    // - For buildings, make sure that all people could get out of residential
    //   zones, and that commercial zones and industrial zones could be reached
    //   by all people.

    simulation_traffic_jam_num_tiles = 0;

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t tile, type;
            CityMapGetTypeAndTile(i, j, &tile, &type);

            int tile_set_flag = 0;

            if (type & (TYPE_HAS_ROAD | TYPE_HAS_TRAIN))
            {
                // Road or train

                int traffic = traffic_map[j * CITY_MAP_WIDTH + i];

                if (traffic >= TRAFFIC_MAX_LEVEL)
                {
                    // Tile isn't ok

                    // Count the number of road/train tiles that have too much
                    // traffic to show warning messages to the player.

                    simulation_traffic_jam_num_tiles++;
                }
                else
                {
                    // Tile is ok
                    tile_set_flag = 1;
                }
            }
            else
            {
                type &= TYPE_MASK;

                // Check if this is a building or not. If not, set tile as ok.
                // Also, ignore docks.

                if ((type == TYPE_FIELD) || (type == TYPE_FOREST) ||
                    (type == TYPE_WATER) || (type == TYPE_DOCK))
                {
                    tile_set_flag = 1;
                }
                else
                {
                    // This is a building, check it

                    int ox, oy;
                    BuildingGetCoordinateOrigin(tile, i, j, &ox, &oy);

                    // Get remaining population of this building (that couldn't
                    // find a destination or source)
                    int value = traffic_map[oy * CITY_MAP_WIDTH + ox];

                    if (value == 0)
                        tile_set_flag = 1;
                }
            }

            if (tile_set_flag)
                Simulation_HappinessSetFlags(i, j, TILE_OK_TRAFFIC);
            else
                Simulation_HappinessResetFlags(i, j, TILE_OK_TRAFFIC);
        }
    }

    // Check if traffic is too high
    // ----------------------------

    building_count_info *bc = Simulation_CountBuildingsGet();

    unsigned int total_tiles = bc->roads + bc->train_tracks;

    if (total_tiles > 0)
    {
        simulation_traffic_jam_num_tiles_percent =
                        (simulation_traffic_jam_num_tiles * 100) / total_tiles;
    }
    else
    {
        simulation_traffic_jam_num_tiles_percent = 0;
    }

    if (simulation_traffic_jam_num_tiles_percent > TRAFFIC_JAM_MAX_TILES)
    {
        // This message is shown only once per year
        PersistentMessageShow(ID_MSG_TRAFFIC_HIGH);
    }
}

IWRAM_CODE void Simulation_Traffic(void)
{
    // Final traffic density and building handled flags go to traffic_map[],
    // temporary expansion map goes to scratch_map[].

    // Clear. Set map to 0 to flag all residential buildings as not handled
    // --------------------------------------------------------------------

    memset(traffic_map, 0, sizeof(traffic_map));
    memset(scratch_map, 0, sizeof(scratch_map));

    // Initialize each non-residential building
    // ----------------------------------------

    // Get density of each non-residential building and save it in the top left
    // tile of the building. It will be reduced as needed with each call to
    // Simulation_TrafficHandleSource().

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t tile, type;
            CityMapGetTypeAndTileUnsafe(i, j, &tile, &type);

            type &= TYPE_MASK;

            // Ignored tile types
            if ((type == TYPE_FIELD) || (type == TYPE_WATER) ||
                (type == TYPE_RESIDENTIAL) || (type == TYPE_FOREST) ||
                (type == TYPE_DOCK))
            {
                continue;
            }

            if (BuildingIsCoordinateOrigin(tile))
            {
                const city_tile_density_info *info = CityTileDensityInfo(tile);
                traffic_map[j * CITY_MAP_WIDTH + i] = info->population;
            }
        }
    }

    // For each tile check if it is a residential building
    // ---------------------------------------------------
    //
    // When a building is handled the rest of the tiles of it are flagged as
    // handled, so we will only check the top left tile of each building.
    // To flag a building as handled it is set to 1 in the TRAFFIC map
    //
    // After handling a residential building the density of population that
    // couldn't get to a valid destination will be stored in the top left tile,
    // and the rest should be flagged as 1 (handled)
    //
    // The "amount of cars" that leave a residential building is the same as the
    // TOP LEFT corner tile density. The same thing goes for the "amount of
    // cars" that can get into another building. However, all tiles of a
    // building should have the same density so that the density map makes
    // sense.

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t type = CityMapGetType(i, j);

            // If not residential, skip
            if (type != TYPE_RESIDENTIAL)
                continue;

            // Residential building = Source of traffic

            // Check if handled. If so, skip
            int val = traffic_map[j * CITY_MAP_WIDTH + i];
            if (val != 0)
                continue;

            Simulation_TrafficHandleSource(i, j);
        }
    }

    // Update tiles of the map to show the traffic level
    // -------------------------------------------------

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t tile, type;
            CityMapGetTypeAndTileUnsafe(i, j, &tile, &type);

            // Skip if not road
            if ((type & TYPE_HAS_ROAD) == 0)
                continue;

            int value = traffic_map[j * CITY_MAP_WIDTH + i];

            if (value > 127)
            {
                // Saturated
                value = 255;
            }
            else
            {
                // Offset to add more cars to the map so that it looks a bit
                // more alive. This doesn't affect pollution or anything else.
                value += 32;
            }

            // 8 bits to 2
            int traffic_level = value >> 6;

            // When drawing, preserve flip. If not, if the animation happens too
            // close to the traffic simulation, the flip will disappear, and the
            // animation steps will look irregular.
            if ((tile == T_ROAD_TB) || (tile == T_ROAD_TB_1) ||
                (tile == T_ROAD_TB_2) || (tile == T_ROAD_TB_3))
            {
                CityMapDrawTilePreserveFlip(T_ROAD_TB + traffic_level, i, j);
            }
            else if ((tile == T_ROAD_LR) || (tile == T_ROAD_LR_1) ||
                     (tile == T_ROAD_LR_2) || (tile == T_ROAD_LR_3))
            {
                CityMapDrawTilePreserveFlip(T_ROAD_LR + traffic_level, i, j);
            }
        }
    }

    // Update happiness of tiles
    // -------------------------

    Simulation_TrafficSetTileOkFlag();
}

IWRAM_CODE void Simulation_TrafficRemoveAnimationTiles(void)
{
    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t tile = CityMapGetTile(i, j);

            if ((tile == T_ROAD_TB) || (tile == T_ROAD_TB_1) ||
                (tile == T_ROAD_TB_2) || (tile == T_ROAD_TB_3))
            {
                CityMapDrawTile(T_ROAD_TB, i, j);
            }
            else if ((tile == T_ROAD_LR) || (tile == T_ROAD_LR_1) ||
                     (tile == T_ROAD_LR_2) || (tile == T_ROAD_LR_3))
            {
                CityMapDrawTile(T_ROAD_LR, i, j);
            }
        }
    }
}

IWRAM_CODE void Simulation_TrafficAnimate(void)
{
    // Animate tiles of the map with traffic animation

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t tile = CityMapGetTile(i, j);

            if ((tile == T_ROAD_TB_1) || (tile == T_ROAD_TB_2) ||
                (tile == T_ROAD_TB_3))
            {
                CityMapToggleVFlip(i, j);
            }
            else if ((tile == T_ROAD_LR_1) || (tile == T_ROAD_LR_2) ||
                     (tile == T_ROAD_LR_3))
            {
                CityMapToggleHFlip(i, j);
            }
        }
    }
}
