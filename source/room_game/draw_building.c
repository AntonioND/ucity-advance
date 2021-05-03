// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2017-2019, 2021, Antonio Niño Díaz

#include <assert.h>
#include <stdint.h>

#include <ugba/ugba.h>

#include "map_utils.h"
#include "money.h"
#include "room_game/building_info.h"
#include "room_game/draw_common.h"
#include "room_game/draw_port.h"
#include "room_game/draw_power_lines.h"
#include "room_game/draw_train.h"
#include "room_game/draw_road.h"
#include "room_game/room_game.h"
#include "room_game/tileset_info.h"

void MapUpdateBuildingSuroundingPowerLines(int x, int y, int h, int w)
{
    // Top and bottom rows
    for (int i = 0; i < w; i++)
    {
        MapTileUpdatePowerLines(x + i, y - 1);
        MapTileUpdatePowerLines(x + i, y + h);
    }

    // Left and right columns
    for (int j = 0; j < h; j++)
    {
        MapTileUpdatePowerLines(x - 1, y + j);
        MapTileUpdatePowerLines(x + w, y + j);
    }
}

// Returns 0 on success
int MapDrawBuilding(int forced, int type, int x, int y)
{
    if (forced == 0)
    {
        // Check if there is enough money

        building_info *bi = Get_Building_Info(type);
        if (MoneyIsThereEnough(bi->price) == 0)
        {
            // Exit and play "not enough money" sound
            // TODO: SFX_BuildError();
            return 1;
        }

        // Check if it is possible to build here (it is empty)

        int w = bi->width;
        int h = bi->height;

        for (int j = 0; j < h; j++)
        {
            for (int i = 0; i < w; i++)
            {
                uint16_t type = CityMapGetType(x + i, y + j);

                // Valid types: Field, forest and power lines

                if ((type != TYPE_FIELD) &&
                    (type != TYPE_FOREST) &&
                    (type != (TYPE_FIELD | TYPE_HAS_POWER)))
                {
                    // Exit and play error sound
                    // TODO: SFX_BuildError();
                    return 1;
                }
            }
        }
    }

    // Build building

    building_info *building = Get_Building_Info(type);

    int w = building->width;
    int h = building->height;
    uint16_t base_tile = building->base_tile;

    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            CityMapDrawTile(base_tile, x + i, y + j);
            base_tile++;
        }
    }

    // Update power lines around it

    MapUpdateBuildingSuroundingPowerLines(x, y, h, w);

    // Decrease money

    if (forced == 0)
    {
        building_info *bi = Get_Building_Info(type);
        MoneyReduce(bi->price);
        // TODO: SFX_Build()
    }

    // Return success

    return 0;
}


// Returns 0 if it could remove the building, 1 on error.
int MapDeleteBuilding(int forced, int x, int y)
{
    // Determine what's the resulting tile after demolishing the building

    uint16_t tile, tile_type;
    CityMapGetTypeAndTile(x, y, &tile, &tile_type);

    uint16_t demolished_tile = T_DEMOLISHED;

    // If this is a RCI tile set the resulting tile to T_DEMOLISHED
    if ((tile == T_RESIDENTIAL) || (tile == T_COMMERCIAL) ||
        (tile == T_INDUSTRIAL))
    {
        demolished_tile = T_DEMOLISHED;
    }
    else
    {
        // If this is a RCI building, demolish to RCI tile
        if (tile_type == TYPE_RESIDENTIAL)
            demolished_tile = T_RESIDENTIAL;
        if (tile_type == TYPE_COMMERCIAL)
            demolished_tile = T_COMMERCIAL;
        if (tile_type == TYPE_INDUSTRIAL)
            demolished_tile = T_INDUSTRIAL;
    }

    // Check origin of coordinates of the building and get type of building

    city_tile_info *info = City_Tileset_Entry_Info(tile);

    int ox = x + info->base_x_delta;
    int oy = y + info->base_y_delta;

    uint16_t origin_tile = CityMapGetTile(ox, oy);

    building_info *bi = Get_BuildingFromBaseTile(origin_tile);
    int w = bi->width;
    int h = bi->height;

    // Check if there is enough money

    int32_t total_price = 0;

    if (forced == 0)
    {
        // The size is needed to calculate the money to be spent.
        building_info *delete_info = Get_Building_Info(B_Delete);
        int32_t base_price = delete_info->price;
        total_price = base_price * w * h;

        if (MoneyIsThereEnough(total_price) == 0)
        {
            // Exit and play "not enough money" sound
            // TODO: SFX_BuildError();

            // Return error
            return 1;
        }
    }

    // Delete building and place demolished tiles

    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
            CityMapDrawTile(demolished_tile, ox + i, oy + j);
    }

    // Update power lines around

    MapUpdateBuildingSuroundingPowerLines(ox, oy, h, w);

    // Decrease money

    if (forced == 0)
    {
        MoneyReduce(total_price);
        // TODO: SFX_Demolish();
    }

    // Return success

    return 0;
}

// Transform demolished into field. Checks money.
void MapClearDemolishedTile(int forced, int x, int y)
{
    // Check if enough money

    if (forced == 0)
    {
        building_info *bi = Get_Building_Info(B_Delete);
        if (MoneyIsThereEnough(bi->price) == 0)
        {
            // Exit and play "not enough money" sound
            // TODO: SFX_BuildError();
            return;
        }
    }

    // Delete

    CityMapDrawTile(T_GRASS, x, y);

    // Decrease money

    if (forced == 0)
    {
        building_info *bi = Get_Building_Info(B_Delete);
        MoneyReduce(bi->price);
        // TODO: SFX_Clear()
    }
}

void Building_Remove(int force, int x, int y)
{
    // Check which kind of element has to be removed

    uint16_t tile, type;
    CityMapGetTypeAndTile(x, y, &tile, &type);

    if ((type & (TYPE_HAS_ROAD | TYPE_HAS_TRAIN | TYPE_HAS_POWER)) != 0)
    {
        BuildingRemoveRoadTrainPowerLines(force, x, y);
        return;
    }

    if ((type & TYPE_MASK) == TYPE_PORT)
    {
        MapDeletePort(force, x, y);
        return;
    }

    // Check for types that can't be deleted
    if ((type == TYPE_WATER) || (type == TYPE_FIRE) || (type == TYPE_RADIATION))
        return;

    if (type == TYPE_FOREST)
    {
        MapClearDemolishedTile(force, x, y);
        return;
    }

    if (tile == T_DEMOLISHED)
    {
        MapClearDemolishedTile(force, x, y);
        return;
    }

    // Check if it's a building
    if ((type == TYPE_RESIDENTIAL) || (type == TYPE_INDUSTRIAL) ||
        (type == TYPE_COMMERCIAL) || (type == TYPE_POLICE_DEPT) ||
        (type == TYPE_FIRE_DEPT) || (type == TYPE_HOSPITAL) ||
        (type == TYPE_SCHOOL) || (type == TYPE_HIGH_SCHOOL) ||
        (type == TYPE_UNIVERSITY) || (type == TYPE_MUSEUM) ||
        (type == TYPE_LIBRARY) || (type == TYPE_STADIUM) ||
        (type == TYPE_AIRPORT) || (type == TYPE_PARK) ||
        (type == TYPE_POWER_PLANT))
    {
        MapDeleteBuilding(force, x, y);
        return;
    }

    // Docks can't be deleted manually
    if (type == TYPE_DOCK)
        return;

    UGBA_Assert(0);
}

// Top level building function
void Building_Build(int force, int type, int x, int y)
{
    if (type == B_Delete)
    {
        // Delete building
        // ---------------

        Building_Remove(force, x, y);

        return;
    }

    if (type < B_BuildingMax)
    {
        // Regular building
        // ----------------

        MapDrawBuilding(force, type, x, y);

        return;
    }

    // Meta buildings
    // --------------

    if (type == B_None)
    {
        // Return, this is a dummy building type
        return;
    }
    else if (type == B_Road)
    {
        MapDrawRoad(force, x, y);
        return;
    }
    else if (type == B_Train)
    {
        MapDrawTrain(force, x, y);
        return;
    }
    else if (type == B_PowerLines)
    {
        MapDrawPowerLines(force, x, y);
        return;
    }
    else if (type == B_Port)
    {
        MapDrawPort(force, x, y);
        return;
    }

    UGBA_Assert(0);
}
