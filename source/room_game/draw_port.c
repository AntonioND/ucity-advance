// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2017-2019, 2021, Antonio Niño Díaz

#include <stdint.h>

#include <ugba/ugba.h>

#include "map_utils.h"
#include "money.h"
#include "room_game/building_info.h"
#include "room_game/draw_building.h"
#include "room_game/draw_common.h"
#include "room_game/room_game.h"
#include "room_game/tileset_info.h"

// Returns 1 if there is water in any tile surounding this building (defined by
// its coordinates and size), 0 if not.
static int MapCheckSurroundingWater(int x, int y, int w, int h)
{
    // Top row
    if (y > 0)
    {
        for (int i = 0; i < w; i++)
        {
            if (CityMapGetType(x + i, y - 1) == TYPE_WATER)
                return 1;
        }
    }

    // Bottom row
    if (y < (CITY_MAP_HEIGHT - 1))
    {
        for (int i = 0; i < w; i++)
        {
            if (CityMapGetType(x + i, y + h) == TYPE_WATER)
                return 1;
        }
    }

    // Left column
    if (x > 0)
    {
        for (int j = 0; j < h; j++)
        {
            if (CityMapGetType(x - 1, y + j) == TYPE_WATER)
                return 1;
        }
    }

    // Right column
    if (x < (CITY_MAP_HEIGHT - 1))
    {
        for (int j = 0; j < h; j++)
        {
            if (CityMapGetType(x + w, y + j) == TYPE_WATER)
                return 1;
        }
    }

    return 0;
}

// Checks all tiles surounding a port and builds docks on the water ones.
static void MapBuildDocksSurrounding(int x, int y, int w, int h)
{
    // Top row
    if (y > 0)
    {
        for (int i = 0; i < w; i++)
        {
            if (CityMapGetType(x + i, y - 1) == TYPE_WATER)
                CityMapDrawTile(T_PORT_WATER_U, x + i, y - 1);
        }
    }

    // Bottom row
    if (y < (CITY_MAP_HEIGHT - 1))
    {
        for (int i = 0; i < w; i++)
        {
            if (CityMapGetType(x + i, y + h) == TYPE_WATER)
                CityMapDrawTile(T_PORT_WATER_D, x + i, y + h);
        }
    }

    // Left column
    if (x > 0)
    {
        for (int j = 0; j < h; j++)
        {
            if (CityMapGetType(x - 1, y + j) == TYPE_WATER)
                CityMapDrawTile(T_PORT_WATER_L, x - 1, y + j);
        }
    }

    // Right column
    if (x < (CITY_MAP_HEIGHT - 1))
    {
        for (int j = 0; j < h; j++)
        {
            if (CityMapGetType(x + w, y + j) == TYPE_WATER)
                CityMapDrawTile(T_PORT_WATER_R, x + w, y + j);
        }
    }
}

// Checks all tiles surounding a port and turns docks into water
static void MapDeleteDocksSurrounding(int x, int y, int w, int h)
{
    // Top row
    if (y > 0)
    {
        for (int i = 0; i < w; i++)
        {
            if (CityMapGetTile(x + i, y - 1) == T_PORT_WATER_U)
            {
                CityMapDrawTile(T_WATER, x + i, y - 1);
                MapUpdateWater(x + i, y - 1);
            }
        }
    }

    // Bottom row
    if (y < (CITY_MAP_HEIGHT - 1))
    {
        for (int i = 0; i < w; i++)
        {
            if (CityMapGetTile(x + i, y + h) == T_PORT_WATER_D)
            {
                CityMapDrawTile(T_WATER, x + i, y + h);
                MapUpdateWater(x + i, y + h);
            }
        }
    }

    // Left column
    if (x > 0)
    {
        for (int j = 0; j < h; j++)
        {
            if (CityMapGetTile(x - 1, y + j) == T_PORT_WATER_L)
            {
                CityMapDrawTile(T_WATER, x - 1, y + j);
                MapUpdateWater(x - 1, y + j);
            }
        }
    }

    // Right column
    if (x < (CITY_MAP_HEIGHT - 1))
    {
        for (int j = 0; j < h; j++)
        {
            if (CityMapGetTile(x + w, y + j) == T_PORT_WATER_R)
            {
                CityMapDrawTile(T_WATER, x + w, y + j);
                MapUpdateWater(x + w, y + j);
            }
        }
    }
}

// Draws a port and as many docks as possible (docks are free).
void MapDrawPort(int force, int x, int y)
{
    // Check if there is water surrounding the building

    const building_info *building = Get_Building_Info(B_Port);

    if (MapCheckSurroundingWater(x, y, building->width, building->height) == 0)
        return;

    // Build building

    if (MapDrawBuilding(force, B_Port, x, y) != 0)
        return; // Error (not enough money or something), so return

    // Build docks if everything went fine before.
    // This is free, don't check money.

    MapBuildDocksSurrounding(x, y, building->width, building->height);
}

// Delete a port and its docks
void MapDeletePort(int force, int x, int y)
{
    // Check origin of coordinates of the building and get type of building

    uint16_t tile = CityMapGetTile(x, y);

    const city_tile_info *info = City_Tileset_Entry_Info(tile);

    int ox = x + info->base_x_delta;
    int oy = y + info->base_y_delta;

    uint16_t origin_tile = CityMapGetTile(ox, oy);

    const building_info *bi = Get_BuildingFromBaseTile(origin_tile);
    int w = bi->width;
    int h = bi->height;

    // Check if there is enough money

    int32_t total_price = 0;

    if (force == 0)
    {
        // The size is needed to calculate the money to be spent.
        const building_info *delete_info = Get_Building_Info(B_Delete);
        int32_t base_price = delete_info->price;
        total_price = base_price * w * h;

        if (MoneyIsThereEnough(total_price) == 0)
        {
            // Exit and play "not enough money" sound
            // TODO: SFX_BuildError();

            // Return
            return;
        }
    }

    // Delete building and place demolished tiles

    uint16_t demolished_tile = T_DEMOLISHED;

    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
            CityMapDrawTile(demolished_tile, ox + i, oy + j);
    }

    // Remove docks if everything went fine before.
    // This is free, don't check money.

    MapDeleteDocksSurrounding(ox, oy, w, h);

    // Decrease money

    if (force == 0)
    {
        MoneyReduce(total_price);
        // TODO: SFX_Demolish();
    }
}
