// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2017-2019, 2021, Antonio Niño Díaz

#include <stdint.h>

#include <ugba/ugba.h>

#include "map_utils.h"
#include "money.h"
#include "room_game/building_info.h"
#include "room_game/draw_common.h"
#include "room_game/draw_road.h"
#include "room_game/draw_power_lines.h"
#include "room_game/draw_train.h"
#include "room_game/room_game.h"
#include "room_game/tileset_info.h"

// ----------------------------------------------------------------------------

// The functions below can be used to guess the type of the rows and columns
// right outside the map (but out of it). They expand the type of the tile in
// the border (water or field). For example, if the last tile at row 63 is a
// forest, row 64 would have a field. If it was water, the result would be water
// too.

uint16_t CityMapGetTypeNoBoundCheck(int x, int y)
{
    void *map = (void *)CITY_MAP_BASE;
    uint16_t tile = MAP_REGULAR_TILE(read_tile_sbb(map, x, y));
    const city_tile_info *tile_info = City_Tileset_Entry_Info(tile);
    return tile_info->element_type;
}

uint16_t CityMapGetType(int x, int y)
{
    int fix = 0;

    if (x < 0)
    {
        x = 0;
        fix = 1;
    }
    else if (x >= CITY_MAP_WIDTH)
    {
        x = CITY_MAP_WIDTH - 1;
        fix = 1;
    }

    if (y < 0)
    {
        y = 0;
        fix = 1;
    }
    else if (y >= CITY_MAP_HEIGHT)
    {
        y = CITY_MAP_HEIGHT - 1;
        fix = 1;
    }

    if (fix)
    {
        if ((CityMapGetTypeNoBoundCheck(x, y) & TYPE_MASK) == TYPE_WATER)
            return TYPE_WATER;
        return TYPE_FIELD;
    }

    void *map = (void *)CITY_MAP_BASE;
    uint16_t tile = MAP_REGULAR_TILE(read_tile_sbb(map, x, y));
    const city_tile_info *tile_info = City_Tileset_Entry_Info(tile);
    return tile_info->element_type;
}

uint16_t CityMapGetTile(int x, int y)
{
    void *map = (void *)CITY_MAP_BASE;
    return MAP_REGULAR_TILE(read_tile_sbb(map, x, y));
}

void CityMapGetTypeAndTileUnsafe(int x, int y, uint16_t *tile, uint16_t *type)
{
    void *map = (void *)CITY_MAP_BASE;
    *tile = MAP_REGULAR_TILE(read_tile_sbb(map, x, y));
    const city_tile_info *tile_info = City_Tileset_Entry_Info(*tile);
    *type = tile_info->element_type;
}

void CityMapGetTypeAndTile(int x, int y, uint16_t *tile, uint16_t *type)
{
    int fix = 0;

    if (x < 0)
    {
        x = 0;
        fix = 1;
    }
    else if (x >= CITY_MAP_WIDTH)
    {
        x = CITY_MAP_WIDTH - 1;
        fix = 1;
    }

    if (y < 0)
    {
        y = 0;
        fix = 1;
    }
    else if (y >= CITY_MAP_HEIGHT)
    {
        y = CITY_MAP_HEIGHT - 1;
        fix = 1;
    }

    if (fix)
    {
        if ((CityMapGetTypeNoBoundCheck(x, y) & TYPE_MASK) == TYPE_WATER)
        {
            *tile = T_WATER;
            *type = TYPE_WATER;
        }
        else
        {
            *tile = T_GRASS;
            *type = TYPE_FIELD;
        }
    }
    else
    {
        CityMapGetTypeAndTileUnsafe(x, y, tile, type);
    }
}

// ----------------------------------------------------------------------------

void CityMapDrawTile(uint16_t tile, int x, int y)
{
    void *map = (void *)CITY_MAP_BASE;
    uint16_t vram_info = City_Tileset_VRAM_Info(tile);
    write_tile_sbb(vram_info, map, x, y);
}

void CityMapDrawTilePreserveFlip(uint16_t tile, int x, int y)
{
    void *map = (void *)CITY_MAP_BASE;
    uint16_t *ptr = get_pointer_sbb(map, x, y);

    uint16_t vram_info = City_Tileset_VRAM_Info(tile);

    uint16_t mask = MAP_REGULAR_HFLIP | MAP_REGULAR_VFLIP;

    *ptr = (*ptr & mask) | vram_info;
}

void CityMapToggleHFlip(int x, int y)
{
    void *map = (void *)CITY_MAP_BASE;
    toggle_hflip_tile_sbb(map, x, y);
}

void CityMapToggleVFlip(int x, int y)
{
    void *map = (void *)CITY_MAP_BASE;
    toggle_vflip_tile_sbb(map, x, y);
}

// Checks if a bridge of a certain type can be built. For that to be possible,
// the coordinates must point at a water tile next to the ground, but with only
// one tile of ground surounding it (or 2 at two opposite sides). It cannot
// leave the map, it must end inside of it.
//
// Returns the length and direction of a possible bridge.
void CityMapCheckBuildBridge(int force, int x, int y, uint16_t type,
                             int *length, int *direction)
{
    *length = 0;

    // Check if this point is actually water with nothing else built on it

    if (CityMapGetType(x, y) != TYPE_WATER)
        return;

    // Check if there's an item nearby like the one specified
    // ------------------------------------------------------
    //
    // Only the border of a river or the sea could have one, so that would
    // mean that this is actually the border of the water.
    //
    // Fail if:
    // - 0 neighbours (don't know how to start)
    // - 2, 3 or 4 neighbours

    int neighbours = 0;

    if (CityMapGetType(x, y - 1) == (type | TYPE_FIELD))
    {
        neighbours++;
        *direction = KEY_DOWN;
    }

    if (CityMapGetType(x - 1, y) == (type | TYPE_FIELD))
    {
        neighbours++;
        *direction = KEY_RIGHT;
    }

    if (CityMapGetType(x + 1, y) == (type | TYPE_FIELD))
    {
        neighbours++;
        *direction = KEY_LEFT;
    }

    if (CityMapGetType(x, y + 1) == (type | TYPE_FIELD))
    {
        neighbours++;
        *direction = KEY_UP;
    }

    // It's only possible to guess direction if there is only one neighbour
    if (neighbours != 1)
        return;

    // Start checking the build direction

    int curx = x;
    int cury = y;

    while (1)
    {
        *length = *length + 1;

        if (*direction == KEY_UP)
            cury--;
        else if (*direction == KEY_DOWN)
            cury++;
        else if (*direction == KEY_LEFT)
            curx--;
        else if (*direction == KEY_RIGHT)
            curx++;

        if ((curx < 0) || (curx >= CITY_MAP_WIDTH) ||
            (cury < 0) || (cury >= CITY_MAP_HEIGHT))
        {
            *length = 0;
            return;
        }

        // Check if there is an obstacle
        uint16_t type = CityMapGetType(curx, cury);
        if (type != TYPE_WATER)
        {
            type &= TYPE_MASK;

            // If bridge or dock, fail
            if ((type == TYPE_WATER) || (type == TYPE_DOCK))
            {
                *length = 0;
                return;
            }

            break;
        }
    }

    // Check if there is enough money

    int building_type;
    if (type == TYPE_HAS_ROAD)
    {
        building_type = B_Road;
    }
    else if (type == TYPE_HAS_TRAIN)
    {
        building_type = B_Train;
    }
    else if (type == TYPE_HAS_POWER)
    {
        building_type = B_PowerLines;
    }
    else
    {
        UGBA_Assert(0);
        return;
    }

    if (force == 0)
    {
        const building_info *bi = Get_Building_Info(building_type);
        if (MoneyIsThereEnough(bi->price * (*length)) == 0)
        {
            // Exit and play "not enough money" sound
            // TODO: SFX_BuildError();
            return;
        }

        // Success

        // TODO: SFX_Build()
    }

    return;
}

// Builds a bridge until it finds a non TYPE_WATER (exactly) tile.
// Returns the other end of the bridge in x and y.
void CityMapBuildBridge(int force, int *x, int *y, uint16_t type,
                        int length, int direction)
{
    // First, check which tile to draw

    int deltax = 0;
    int deltay = 0;
    uint16_t tile;

    if (type == TYPE_HAS_ROAD)
    {
        if (direction == KEY_UP)
        {
            deltay = -1;
            tile = T_ROAD_TB_BRIDGE;
        }
        else if (direction == KEY_DOWN)
        {
            deltay = +1;
            tile = T_ROAD_TB_BRIDGE;
        }
        else if (direction == KEY_LEFT)
        {
            deltax = -1;
            tile = T_ROAD_LR_BRIDGE;
        }
        else if (direction == KEY_RIGHT)
        {
            deltax = +1;
            tile = T_ROAD_LR_BRIDGE;
        }
        else
        {
            UGBA_Assert(0);
            return;
        }
    }
    else if (type == TYPE_HAS_TRAIN)
    {
        if (direction == KEY_UP)
        {
            deltay = -1;
            tile = T_TRAIN_TB_BRIDGE;
        }
        else if (direction == KEY_DOWN)
        {
            deltay = +1;
            tile = T_TRAIN_TB_BRIDGE;
        }
        else if (direction == KEY_LEFT)
        {
            deltax = -1;
            tile = T_TRAIN_LR_BRIDGE;
        }
        else if (direction == KEY_RIGHT)
        {
            deltax = +1;
            tile = T_TRAIN_LR_BRIDGE;
        }
        else
        {
            UGBA_Assert(0);
            return;
        }
    }
    else if (type == TYPE_HAS_POWER)
    {
        if (direction == KEY_UP)
        {
            deltay = -1;
            tile = T_POWER_LINES_TB_BRIDGE;
        }
        else if (direction == KEY_DOWN)
        {
            deltay = +1;
            tile = T_POWER_LINES_TB_BRIDGE;
        }
        else if (direction == KEY_LEFT)
        {
            deltax = -1;
            tile = T_POWER_LINES_LR_BRIDGE;
        }
        else if (direction == KEY_RIGHT)
        {
            deltax = +1;
            tile = T_POWER_LINES_LR_BRIDGE;
        }
        else
        {
            UGBA_Assert(0);
            return;
        }
    }
    else
    {
        UGBA_Assert(0);
        return;
    }

    // Draw bridge

    int built_length = 0;

    while (built_length < length)
    {
        uint16_t type = CityMapGetType(*x, *y);
        if (type != TYPE_WATER)
            break;

        CityMapDrawTile(tile, *x, *y);

        *x = *x + deltax;
        *y = *y + deltay;

        built_length--;
    }

    // Get money

    int building_type;
    if (type == TYPE_HAS_ROAD)
    {
        building_type = B_Road;
    }
    else if (type == TYPE_HAS_TRAIN)
    {
        building_type = B_Train;
    }
    else if (type == TYPE_HAS_POWER)
    {
        building_type = B_PowerLines;
    }
    else
    {
        UGBA_Assert(0);
        return;
    }

    if (force == 0)
    {
        const building_info *bi = Get_Building_Info(building_type);
        MoneyReduce(bi->price * length);
    }

    // Done
}

typedef struct {
    uint8_t mask;
    uint8_t expected_result;
    uint16_t resulting_tile;
} water_mask;

static const water_mask water_mask_table[] = {
    // From more restrictive to less restrictive

    // 8 neighbours of this tile.
    //
    // 0 1 2
    // 3 . 4 <- Bit order
    // 5 6 7

    { 0b01011010, 0b01010000, T_WATER__GRASS_TL },
    { 0b01011010, 0b01011000, T_WATER__GRASS_TC },
    { 0b01011010, 0b01001000, T_WATER__GRASS_TR },
    { 0b01011010, 0b01010010, T_WATER__GRASS_CL },
    { 0b01011010, 0b01001010, T_WATER__GRASS_CR },
    { 0b01011010, 0b00010010, T_WATER__GRASS_BL },
    { 0b01011010, 0b00011010, T_WATER__GRASS_BC },
    { 0b01011010, 0b00001010, T_WATER__GRASS_BR },

    { 0b01011011, 0b01011010, T_WATER__GRASS_CORNER_TL },
    { 0b01011110, 0b01011010, T_WATER__GRASS_CORNER_TR },
    { 0b01111010, 0b01011010, T_WATER__GRASS_CORNER_BL },
    { 0b11011010, 0b01011010, T_WATER__GRASS_CORNER_BR },

    { 0b00000000, 0b00000000, T_WATER }, // Default -> Always valid
};

void MapUpdateWater(int x, int y)
{
    if ((x < 0) || (x >= CITY_MAP_WIDTH))
        return;
    if ((y < 0) || (y >= CITY_MAP_HEIGHT))
        return;

    // Assume that this is water, and that a bridge here (if any) is supposed to
    // be deleted by this function. If this is a dock, delete it too.

    // Calculate the needed tile
    // -------------------------

    // Create a byte containing the state of the 8 neighbours of this pixel.
    //
    // 1 = has water, 0 = doesn't have water.
    //
    // 0 1 2
    // 3 . 4 <- Bit order
    // 5 6 7

    uint8_t flags = 0;
    uint16_t aux_type;;

    aux_type = CityMapGetType(x - 1, y - 1) & TYPE_MASK;
    if ((aux_type == TYPE_WATER) || (aux_type == TYPE_DOCK))
        flags |= 1 << 0;

    aux_type = CityMapGetType(x, y - 1) & TYPE_MASK;
    if ((aux_type == TYPE_WATER) || (aux_type == TYPE_DOCK))
        flags |= 1 << 1;

    aux_type = CityMapGetType(x + 1, y - 1) & TYPE_MASK;
    if ((aux_type == TYPE_WATER) || (aux_type == TYPE_DOCK))
        flags |= 1 << 2;

    aux_type = CityMapGetType(x - 1, y) & TYPE_MASK;
    if ((aux_type == TYPE_WATER) || (aux_type == TYPE_DOCK))
        flags |= 1 << 3;

    aux_type = CityMapGetType(x + 1, y) & TYPE_MASK;
    if ((aux_type == TYPE_WATER) || (aux_type == TYPE_DOCK))
        flags |= 1 << 4;

    aux_type = CityMapGetType(x - 1, y + 1) & TYPE_MASK;
    if ((aux_type == TYPE_WATER) || (aux_type == TYPE_DOCK))
        flags |= 1 << 5;

    aux_type = CityMapGetType(x, y + 1) & TYPE_MASK;
    if ((aux_type == TYPE_WATER) || (aux_type == TYPE_DOCK))
        flags |= 1 << 6;

    aux_type = CityMapGetType(x + 1, y + 1) & TYPE_MASK;
    if ((aux_type == TYPE_WATER) || (aux_type == TYPE_DOCK))
        flags |= 1 << 7;

    // Compare with table

    const water_mask *wm = &water_mask_table[0];
    while (1)
    {
        // This loop will always end because the last element of the table will
        // always pass this check.
        if ((wm->mask & flags) == wm->expected_result)
        {
            // Draw resulting tile
            CityMapDrawTile(wm->resulting_tile, x, y);
            return;
        }

        wm++;
    }
}

// It deletes one tile of road, train or power lines, but it doesn't update
// neighbours, that has to be done by the caller. It doesn't work to demolish
// bridges. Returns 0 on success.
int MapDeleteRoadTrainPowerlines(int force, int x, int y)
{
    // Check if there is enough money

    if (force == 0)
    {
        const building_info *bi = Get_Building_Info(B_Delete);
        if (MoneyIsThereEnough(bi->price) == 0)
        {
            // Exit and play "not enough money" sound
            // TODO: SFX_BuildError();

            // Return
            return 1;
        }
    }

    // Delete

    CityMapDrawTile(T_DEMOLISHED, x, y);

    // Decrease money

    if (force == 0)
    {
        const building_info *bi = Get_Building_Info(B_Delete);
        MoneyReduce(bi->price);
        // TODO: SFX_Demolish();
    }

    return 0;
}

// Roads, train tracks and power lines require special handling because it is
// needed to handle bridges in a different way.
void BuildingRemoveRoadTrainPowerLines(int force, int x, int y)
{
    // Check if this is a bridge

    uint16_t tile, type;
    CityMapGetTypeAndTile(x, y, &tile, &type);

    if ((type & TYPE_MASK) == TYPE_WATER)
    {
        DrawCityDeleteBridge(force, x, y);
        return;
    }

    // Delete tile

    if (MapDeleteRoadTrainPowerlines(force, x, y) != 0)
        return; // Exit if it failed

    // Update suroundings according to the elements that it had

    if (type & TYPE_HAS_ROAD)
        MapUpdateNeighboursRoad(x, y);

    if (type & TYPE_HAS_TRAIN)
        MapUpdateNeighboursTrain(x, y);

    if (type & TYPE_HAS_POWER)
        MapUpdateNeighboursPowerLines(x, y);
}

typedef struct {
    uint16_t tile;
    uint16_t type;
    int is_vertical;
} bridge_info;

static const bridge_info bridge_tile_info[] = {
    { T_ROAD_TB_BRIDGE, TYPE_HAS_ROAD, 1 },
    { T_ROAD_LR_BRIDGE, TYPE_HAS_ROAD, 0 },

    { T_TRAIN_TB_BRIDGE, TYPE_HAS_TRAIN, 1 },
    { T_TRAIN_LR_BRIDGE, TYPE_HAS_TRAIN, 0 },

    { T_POWER_LINES_TB_BRIDGE, TYPE_HAS_POWER, 1 },
    { T_POWER_LINES_LR_BRIDGE, TYPE_HAS_POWER, 0 },

    { 0, 0, 0 }, // End
};

// Checks length of the bridge to see if there is money to delete. If so, it
// calls DrawCityDeleteBridgeForce and reduces the money. The money check
// can be disabled. If plays SFX.
void DrawCityDeleteBridge(int force, int x, int y)
{
    // Check tile to see which direction to go (up or left)

    uint16_t tile = CityMapGetTile(x, y);

    int is_vertical;
    int type;

    const bridge_info *bti = &bridge_tile_info[0];
    while (1)
    {
        if (bti->tile == tile)
        {
            is_vertical = bti->is_vertical;
            type = bti->type;
            break;
        }

        bti++;

        if (bti->type == 0)
            return;
    }

    // Go to the top of the bridge or the left depending on orientation

    int ox = x;
    int oy = y;

    while (1)
    {
        if ((ox < 0) || (oy < 0))
            return;

        if (is_vertical)
            oy--;
        else
            ox--;

        uint16_t type = CityMapGetType(ox, oy) & TYPE_MASK;
        if (type != TYPE_WATER)
        {
            if (is_vertical)
                oy++;
            else
                ox++;
            break;
        }
    }

    // Calculate how many tiles long is this bridge

    int ex = ox;
    int ey = oy;
    int length = 1;

    while (1)
    {
        if ((ex >= CITY_MAP_WIDTH) || (ey >= CITY_MAP_HEIGHT))
            return;

        if (is_vertical)
            ey++;
        else
            ex++;

        uint16_t type = CityMapGetType(ex, ey) & TYPE_MASK;
        if (type != TYPE_WATER)
        {
            if (is_vertical)
                ey--;
            else
                ex--;
            break;
        }

        length++;
    }

    // Check if there is enough money

    int32_t total_price = 0;

    if (force == 0)
    {
        // The size is needed to calculate the money to be spent.
        const building_info *delete_info = Get_Building_Info(B_Delete);
        int32_t base_price = delete_info->price;
        total_price = base_price * length;

        if (MoneyIsThereEnough(total_price) == 0)
        {
            // Exit and play "not enough money" sound
            // TODO: SFX_BuildError();

            // Return
            return;
        }
    }

    // Delete bridge

    x = ox;
    y = oy;

    for (int l = 0; l < length; l++)
    {
        MapUpdateWater(x, y);

        if (is_vertical)
        {
            y++;
            if (y > ey)
                break;
        }
        else
        {
            x++;
            if (x > ex)
                break;
        }
    }

    // Update both ends of the bridge

    // Because of how mixing road, trains and power line works, it's NOT
    // impossible for them to be together at one of the ends of the bridge,
    // but in that case there is no point in doing a refresh because it can't
    // change the tile.

    if (is_vertical)
    {
        oy--;
        ey++;
    }
    else
    {
        ox--;
        ex++;
    }

    if (type == TYPE_HAS_ROAD)
    {
        MapUpdateNeighboursRoad(ox, oy);
        MapUpdateNeighboursRoad(ex, ey);
    }
    else if (type == TYPE_HAS_TRAIN)
    {
        MapUpdateNeighboursTrain(ox, oy);
        MapUpdateNeighboursTrain(ex, ey);
    }
    else if (type == TYPE_HAS_POWER)
    {
        MapUpdateNeighboursPowerLines(ox, oy);
        MapUpdateNeighboursPowerLines(ex, ey);
    }

    // Decrease money

    if (force == 0)
    {
        MoneyReduce(total_price);
        // TODO: SFX_Demolish();
    }

    // Done
}
