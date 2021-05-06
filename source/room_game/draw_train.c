// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2017-2019, 2021, Antonio Niño Díaz

#include <stdint.h>

#include <ugba/ugba.h>

#include "map_utils.h"
#include "money.h"
#include "room_game/building_info.h"
#include "room_game/draw_common.h"
#include "room_game/room_game.h"
#include "room_game/tileset_info.h"

typedef struct {
    uint8_t mask;
    uint8_t expected_result;
    uint16_t resulting_tile;
} train_mask;

static const train_mask train_mask_table[] = {
    // From more restrictive to less restrictive

    // 8 neighbours of this tile.
    //
    // 0 1 2
    // 3 . 4 <- Bit order
    // 5 6 7

    { 0b01011010, 0b01011010, T_TRAIN_TLRB },

    { 0b01011010, 0b01010010, T_TRAIN_TRB },
    { 0b01011010, 0b01011000, T_TRAIN_LRB },
    { 0b01011010, 0b01001010, T_TRAIN_TLB },
    { 0b01011010, 0b00011010, T_TRAIN_TLR },

    { 0b01011010, 0b01010000, T_TRAIN_RB },
    { 0b01011010, 0b00010010, T_TRAIN_TR },
    { 0b01011010, 0b01001000, T_TRAIN_LB },
    { 0b01011010, 0b00001010, T_TRAIN_TL },

    { 0b01011010, 0b01000000, T_TRAIN_TB },
    { 0b01011010, 0b00000010, T_TRAIN_TB },
    { 0b01011010, 0b01000010, T_TRAIN_TB },

    { 0b01011010, 0b00010000, T_TRAIN_LR },
    { 0b01011010, 0b00001000, T_TRAIN_LR },
    { 0b01011010, 0b00011000, T_TRAIN_LR },

    { 0b00000000, 0b00000000, T_TRAIN_LR }, // Default -> Always valid
};

void MapTileUpdateTrain(int x, int y)
{
    if ((x < 0) || (x >= CITY_MAP_WIDTH))
        return;
    if ((y < 0) || (y >= CITY_MAP_HEIGHT))
        return;

    // Check if this is actually a road

    uint16_t tile, tile_type;
    CityMapGetTypeAndTile(x, y, &tile, &tile_type);

    if ((tile_type & TYPE_HAS_TRAIN) == 0)
        return; // Not a train track

    if ((tile_type & TYPE_MASK) == TYPE_WATER)
        return; // This is a bridge, don't update

    // If there are roads lines and power lines, train tracks can't be built
    if ((tile_type & (TYPE_HAS_ROAD | TYPE_HAS_POWER)) ==
        (TYPE_HAS_ROAD | TYPE_HAS_POWER))
        return;

    if (tile_type & TYPE_HAS_ROAD)
    {
        if ((tile == T_ROAD_TB) || (tile == T_ROAD_TB_1) ||
            (tile == T_ROAD_TB_2) || (tile == T_ROAD_TB_3))
        {
            CityMapDrawTile(T_TRAIN_LR_ROAD, x, y);
        }
        else if ((tile == T_ROAD_LR) || (tile == T_ROAD_LR_1) ||
                 (tile == T_ROAD_LR_2) || (tile == T_ROAD_LR_3))
        {
            CityMapDrawTile(T_TRAIN_TB_ROAD, x, y);
        }

        return;
    }

    if (tile_type & TYPE_HAS_POWER)
    {
        if (tile == T_POWER_LINES_TB)
            CityMapDrawTile(T_TRAIN_LR_POWER_LINES, x, y);
        else if (tile == T_POWER_LINES_LR)
            CityMapDrawTile(T_TRAIN_TB_POWER_LINES, x, y);

        return;
    }

    // Calculate the needed tile
    // -------------------------

    // Create a byte containing the state of the 8 neighbours of this pixel.
    //
    // 1 = has train track, 0 = doesn't have train track.
    //
    // 0 1 2
    // 3 . 4 <- Bit order
    // 5 6 7

    uint8_t flags = 0;
    uint16_t aux_tile, aux_tile_type;

    CityMapGetTypeAndTile(x, y - 1, &aux_tile, &aux_tile_type);
    if (aux_tile_type & TYPE_HAS_TRAIN)
    {
        if (aux_tile != T_TRAIN_LR_BRIDGE)
            flags |= 1 << 1;
    }

    CityMapGetTypeAndTile(x - 1, y, &aux_tile, &aux_tile_type);
    if (aux_tile_type & TYPE_HAS_TRAIN)
    {
        if (aux_tile != T_TRAIN_TB_BRIDGE)
            flags |= 1 << 3;
    }

    CityMapGetTypeAndTile(x + 1, y, &aux_tile, &aux_tile_type);
    if (aux_tile_type & TYPE_HAS_TRAIN)
    {
        if (aux_tile != T_TRAIN_TB_BRIDGE)
            flags |= 1 << 4;
    }

    CityMapGetTypeAndTile(x, y + 1, &aux_tile, &aux_tile_type);
    if (aux_tile_type & TYPE_HAS_TRAIN)
    {
        if (aux_tile != T_TRAIN_LR_BRIDGE)
            flags |= 1 << 6;
    }

    // Compare with table

    const train_mask *tm = &train_mask_table[0];
    while (1)
    {
        // This loop will always end because the last element of the table will
        // always pass this check.
        if ((tm->mask & flags) == tm->expected_result)
        {
            // Draw resulting tile
            CityMapDrawTile(tm->resulting_tile, x, y);
            return;
        }

        tm++;
    }
}

void MapUpdateNeighboursTrain(int x, int y)
{
    MapTileUpdateTrain(x - 1, y - 1);
    MapTileUpdateTrain(x, y - 1);
    MapTileUpdateTrain(x + 1, y - 1);

    MapTileUpdateTrain(x - 1, y);
    MapTileUpdateTrain(x, y);
    MapTileUpdateTrain(x + 1, y);

    MapTileUpdateTrain(x - 1, y + 1);
    MapTileUpdateTrain(x, y + 1);
    MapTileUpdateTrain(x + 1, y + 1);
}

// Adds road and updates neighbours
void MapDrawTrain(int force, int x, int y)
{
    if ((x < 0) || (x >= CITY_MAP_WIDTH))
        return;
    if ((y < 0) || (y >= CITY_MAP_HEIGHT))
        return;

    // Check if it is possible to build here

    uint16_t tile, tile_type;
    CityMapGetTypeAndTile(x, y, &tile, &tile_type);

    if (tile_type & TYPE_HAS_TRAIN)
        return; // Already a train track

    if ((tile_type & TYPE_MASK) == TYPE_WATER)
    {
        // Bridge: Treat it differently
        // ----------------------------

        int length = 0;
        int direction;
        CityMapCheckBuildBridge(force, x, y, TYPE_HAS_TRAIN, &length, &direction);

        if (length == 0)
            return;

        int endx = x;
        int endy = y;
        CityMapBuildBridge(force, &endx, &endy, TYPE_HAS_TRAIN, length, direction);

        MapUpdateNeighboursTrain(x, y);
        MapTileUpdateTrain(endx, endy);

        return;
    }

    // Single train track tile
    // -----------------------

    // Check if there is enough money

    if (force == 0)
    {
        const building_info *bi = Get_Building_Info(B_Train);
        if (MoneyIsThereEnough(bi->price) == 0)
        {
            // Exit and play "not enough money" sound
            // TODO: SFX_BuildError();
            return;
        }
    }

    // If there are roads and power lines, train tracks can't be built
    if ((tile_type & (TYPE_HAS_ROAD | TYPE_HAS_POWER)) ==
        (TYPE_HAS_ROAD | TYPE_HAS_POWER))
        return;

    if (tile_type & TYPE_HAS_ROAD)
    {
        if ((tile == T_ROAD_TB) || (tile == T_ROAD_TB_1) ||
            (tile == T_ROAD_TB_2) || (tile == T_ROAD_TB_3))
        {
            CityMapDrawTile(T_TRAIN_LR_ROAD, x, y);
            goto built;
        }
        else if ((tile == T_ROAD_LR) || (tile == T_ROAD_LR_1) ||
                 (tile == T_ROAD_LR_2) || (tile == T_ROAD_LR_3))
        {
            CityMapDrawTile(T_TRAIN_TB_ROAD, x, y);
            goto built;
        }
        return;
    }

    if (tile_type & TYPE_HAS_POWER)
    {
        if (tile == T_POWER_LINES_TB)
        {
            CityMapDrawTile(T_TRAIN_LR_POWER_LINES, x, y);
            goto built;
        }
        else if (tile == T_POWER_LINES_LR)
        {
            CityMapDrawTile(T_TRAIN_TB_POWER_LINES, x, y);
            goto built;
        }
        return;
    }

    uint16_t base_type = tile_type & TYPE_MASK; // Get type without flags

    if ((base_type == TYPE_FIELD) || (base_type == TYPE_FOREST))
    {
        // Build road
        CityMapDrawTile(T_TRAIN_TB, x, y);
        goto built;
    }

    // Not built
    return;

built:

    // Update neighbours

    MapUpdateNeighboursTrain(x, y);

    // Decrease money (one tile)

    if (force == 0)
    {
        const building_info *bi = Get_Building_Info(B_Train);
        MoneyReduce(bi->price);
        // TODO: SFX_Build();
    }
}
