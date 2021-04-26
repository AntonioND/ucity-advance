// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2017-2019, 2021, Antonio Niño Díaz

#include <stdint.h>

#include <ugba/ugba.h>

#include "map_utils.h"
#include "room_game/room_game.h"
#include "room_game/tileset_info.h"

// ----------------------------------------------------------------------------

#include "maps/city_tileset_map.h"

// This takes a define from the list in "tileset_info.h" and returns the
// combined tile number and palette to be written to VRAM in the background.
uint16_t City_Tileset_VRAM_Info(uint16_t city_index)
{
    UGBA_Assert(city_index < city_tileset_map_map_height);
    return city_tileset_map_map[city_index];
}

// ----------------------------------------------------------------------------

static city_tile_info city_tileset_info[] = { // TILESET_INFO
    [T_GRASS__FOREST_TL] = { TYPE_FOREST, 0, 0, "Forest" },
    [T_GRASS__FOREST_TC] = { TYPE_FOREST, 0, 0, "Forest" },
    [T_GRASS__FOREST_TR] = { TYPE_FOREST, 0, 0, "Forest" },
    [T_GRASS__FOREST_CL] = { TYPE_FOREST, 0, 0, "Forest" },
    [T_GRASS] = { TYPE_FIELD, 0, 0, "Field" },
    [T_GRASS__FOREST_CR] = { TYPE_FOREST, 0, 0, "Forest" },
    [T_GRASS__FOREST_BL] = { TYPE_FOREST, 0, 0, "Forest" },
    [T_GRASS__FOREST_BC] = { TYPE_FOREST, 0, 0, "Forest" },
    [T_GRASS__FOREST_BR] = { TYPE_FOREST, 0, 0, "Forest" },
    [T_GRASS__FOREST_CORNER_TL] = { TYPE_FOREST, 0, 0, "Field" },
    [T_GRASS__FOREST_CORNER_TR] = { TYPE_FOREST, 0, 0, "Field" },
    [T_GRASS__FOREST_CORNER_BL] = { TYPE_FOREST, 0, 0, "Field" },
    [T_GRASS__FOREST_CORNER_BR] = { TYPE_FOREST, 0, 0, "Field" },
    [T_FOREST] = { TYPE_FOREST, 0, 0, "Forest" },
    [T_GRASS_EXTRA] = { TYPE_FIELD, 0, 0, "Field" },
    [T_FOREST_EXTRA] = { TYPE_FOREST, 0, 0, "Forest" },

    [T_WATER__GRASS_TL] = { TYPE_WATER, 0, 0, "Water" },
    [T_WATER__GRASS_TC] = { TYPE_WATER, 0, 0, "Water" },
    [T_WATER__GRASS_TR] = { TYPE_WATER, 0, 0, "Water" },
    [T_WATER__GRASS_CL] = { TYPE_WATER, 0, 0, "Water" },
    [T_WATER] = { TYPE_WATER, 0, 0, "Water" },
    [T_WATER__GRASS_CR] = { TYPE_WATER, 0, 0, "Water" },
    [T_WATER__GRASS_BL] = { TYPE_WATER, 0, 0, "Water" },
    [T_WATER__GRASS_BC] = { TYPE_WATER, 0, 0, "Water" },
    [T_WATER__GRASS_BR] = { TYPE_WATER, 0, 0, "Water" },
    [T_WATER__GRASS_CORNER_TL] = { TYPE_WATER, 0, 0, "Water" },
    [T_WATER__GRASS_CORNER_TR] = { TYPE_WATER, 0, 0, "Water" },
    [T_WATER__GRASS_CORNER_BL] = { TYPE_WATER, 0, 0, "Water" },
    [T_WATER__GRASS_CORNER_BR] = { TYPE_WATER, 0, 0, "Water" },
    [T_WATER_EXTRA] = { TYPE_WATER, 0, 0, "Water" },

    [T_RESIDENTIAL] = { TYPE_RESIDENTIAL, 0, 0, "Residential" },
    [T_COMMERCIAL] = { TYPE_COMMERCIAL, 0, 0, "Commercial" },
    [T_INDUSTRIAL] = { TYPE_INDUSTRIAL, 0, 0, "Industrial" },
    [T_DEMOLISHED] = { TYPE_FIELD, 0, 0, "Demolished" },

    [T_ROAD_TB] = { TYPE_FIELD | TYPE_HAS_ROAD, 0, 0, "Road" },
    [T_ROAD_TB_1] = { TYPE_FIELD | TYPE_HAS_ROAD, 0, 0, "Road" },
    [T_ROAD_TB_2] = { TYPE_FIELD | TYPE_HAS_ROAD, 0, 0, "Road" },
    [T_ROAD_TB_3] = { TYPE_FIELD | TYPE_HAS_ROAD, 0, 0, "Road" },
    [T_ROAD_LR] = { TYPE_FIELD | TYPE_HAS_ROAD, 0, 0, "Road" },
    [T_ROAD_LR_1] = { TYPE_FIELD | TYPE_HAS_ROAD, 0, 0, "Road" },
    [T_ROAD_LR_2] = { TYPE_FIELD | TYPE_HAS_ROAD, 0, 0, "Road" },
    [T_ROAD_LR_3] = { TYPE_FIELD | TYPE_HAS_ROAD, 0, 0, "Road" },
    [T_ROAD_RB] = { TYPE_FIELD | TYPE_HAS_ROAD, 0, 0, "Road" },
    [T_ROAD_LB] = { TYPE_FIELD | TYPE_HAS_ROAD, 0, 0, "Road" },
    [T_ROAD_TR] = { TYPE_FIELD | TYPE_HAS_ROAD, 0, 0, "Road" },
    [T_ROAD_TL] = { TYPE_FIELD | TYPE_HAS_ROAD, 0, 0, "Road" },
    [T_ROAD_TRB] = { TYPE_FIELD | TYPE_HAS_ROAD, 0, 0, "Road" },
    [T_ROAD_LRB] = { TYPE_FIELD | TYPE_HAS_ROAD, 0, 0, "Road" },
    [T_ROAD_TLB] = { TYPE_FIELD | TYPE_HAS_ROAD, 0, 0, "Road" },
    [T_ROAD_TLR] = { TYPE_FIELD | TYPE_HAS_ROAD, 0, 0, "Road" },
    [T_ROAD_TLRB] = { TYPE_FIELD | TYPE_HAS_ROAD, 0, 0, "Road" },
    [T_ROAD_TB_POWER_LINES] = { TYPE_FIELD | TYPE_HAS_ROAD | TYPE_HAS_POWER, 0, 0, "Road + Power Lines" },
    [T_ROAD_LR_POWER_LINES] = { TYPE_FIELD | TYPE_HAS_ROAD | TYPE_HAS_POWER, 0, 0, "Road + Power Lines" },
    [T_ROAD_TB_BRIDGE] = { TYPE_WATER | TYPE_HAS_ROAD, 0, 0, "Road Bridge" },
    [T_ROAD_LR_BRIDGE] = { TYPE_WATER | TYPE_HAS_ROAD, 0, 0, "Road Bridge" },

    [T_TRAIN_TB] = { TYPE_FIELD | TYPE_HAS_TRAIN, 0, 0, "Train" },
    [T_TRAIN_LR] = { TYPE_FIELD | TYPE_HAS_TRAIN, 0, 0, "Train" },
    [T_TRAIN_RB] = { TYPE_FIELD | TYPE_HAS_TRAIN, 0, 0, "Train" },
    [T_TRAIN_LB] = { TYPE_FIELD | TYPE_HAS_TRAIN, 0, 0, "Train" },
    [T_TRAIN_TR] = { TYPE_FIELD | TYPE_HAS_TRAIN, 0, 0, "Train" },
    [T_TRAIN_TL] = { TYPE_FIELD | TYPE_HAS_TRAIN, 0, 0, "Train" },
    [T_TRAIN_TRB] = { TYPE_FIELD | TYPE_HAS_TRAIN, 0, 0, "Train" },
    [T_TRAIN_LRB] = { TYPE_FIELD | TYPE_HAS_TRAIN, 0, 0, "Train" },
    [T_TRAIN_TLB] = { TYPE_FIELD | TYPE_HAS_TRAIN, 0, 0, "Train" },
    [T_TRAIN_TLR] = { TYPE_FIELD | TYPE_HAS_TRAIN, 0, 0, "Train" },
    [T_TRAIN_TLRB] = { TYPE_FIELD | TYPE_HAS_TRAIN, 0, 0, "Train" },
    [T_TRAIN_LR_ROAD] = { TYPE_FIELD | TYPE_HAS_TRAIN | TYPE_HAS_ROAD, 0, 0, "Road + Train" },
    [T_TRAIN_TB_ROAD] = { TYPE_FIELD | TYPE_HAS_TRAIN | TYPE_HAS_ROAD, 0, 0, "Road + Train" },
    [T_TRAIN_TB_POWER_LINES] = { TYPE_FIELD | TYPE_HAS_TRAIN | TYPE_HAS_POWER, 0, 0, "Train + Power Lines" },
    [T_TRAIN_LR_POWER_LINES] = { TYPE_FIELD | TYPE_HAS_TRAIN | TYPE_HAS_POWER, 0, 0, "Train + Power Lines" },
    [T_TRAIN_TB_BRIDGE] = { TYPE_WATER | TYPE_HAS_TRAIN, 0, 0, "Train Bridge" },
    [T_TRAIN_LR_BRIDGE] = { TYPE_WATER | TYPE_HAS_TRAIN, 0, 0, "Train Bridge" },

    [T_POWER_LINES_TB] = { TYPE_FIELD | TYPE_HAS_POWER, 0, 0, "Power Lines" },
    [T_POWER_LINES_LR] = { TYPE_FIELD | TYPE_HAS_POWER, 0, 0, "Power Lines" },
    [T_POWER_LINES_RB] = { TYPE_FIELD | TYPE_HAS_POWER, 0, 0, "Power Lines" },
    [T_POWER_LINES_LB] = { TYPE_FIELD | TYPE_HAS_POWER, 0, 0, "Power Lines" },
    [T_POWER_LINES_TR] = { TYPE_FIELD | TYPE_HAS_POWER, 0, 0, "Power Lines" },
    [T_POWER_LINES_TL] = { TYPE_FIELD | TYPE_HAS_POWER, 0, 0, "Power Lines" },
    [T_POWER_LINES_TRB] = { TYPE_FIELD | TYPE_HAS_POWER, 0, 0, "Power Lines" },
    [T_POWER_LINES_LRB] = { TYPE_FIELD | TYPE_HAS_POWER, 0, 0, "Power Lines" },
    [T_POWER_LINES_TLB] = { TYPE_FIELD | TYPE_HAS_POWER, 0, 0, "Power Lines" },
    [T_POWER_LINES_TLR] = { TYPE_FIELD | TYPE_HAS_POWER, 0, 0, "Power Lines" },
    [T_POWER_LINES_TLRB] = { TYPE_FIELD | TYPE_HAS_POWER, 0, 0, "Power Lines" },
    [T_POWER_LINES_TB_BRIDGE] = { TYPE_WATER | TYPE_HAS_POWER, 0, 0, "Power Lines Bridge" },
    [T_POWER_LINES_LR_BRIDGE] = { TYPE_WATER | TYPE_HAS_POWER, 0, 0, "Power Lines Bridge" },

    [T_POLICE_DEPT + 0] = { TYPE_POLICE_DEPT,  0, 0, "Police Department" },
    [T_POLICE_DEPT + 1] = { TYPE_POLICE_DEPT, -1, 0, "Police Department" },
    [T_POLICE_DEPT + 2] = { TYPE_POLICE_DEPT, -2, 0, "Police Department" },
    [T_POLICE_DEPT + 3] = { TYPE_POLICE_DEPT,  0,-1, "Police Department" },
    [T_POLICE_DEPT + 4] = { TYPE_POLICE_DEPT, -1,-1, "Police Department" },
    [T_POLICE_DEPT + 5] = { TYPE_POLICE_DEPT, -2,-1, "Police Department" },
    [T_POLICE_DEPT + 6] = { TYPE_POLICE_DEPT,  0,-2, "Police Department" },
    [T_POLICE_DEPT + 7] = { TYPE_POLICE_DEPT, -1,-2, "Police Department" },
    [T_POLICE_DEPT + 8] = { TYPE_POLICE_DEPT, -2,-2, "Police Department" },

    [T_FIRE_DEPT + 0] = { TYPE_FIRE_DEPT,  0, 0, "Fire Department" },
    [T_FIRE_DEPT + 1] = { TYPE_FIRE_DEPT, -1, 0, "Fire Department" },
    [T_FIRE_DEPT + 2] = { TYPE_FIRE_DEPT, -2, 0, "Fire Department" },
    [T_FIRE_DEPT + 3] = { TYPE_FIRE_DEPT,  0,-1, "Fire Department" },
    [T_FIRE_DEPT + 4] = { TYPE_FIRE_DEPT, -1,-1, "Fire Department" },
    [T_FIRE_DEPT + 5] = { TYPE_FIRE_DEPT, -2,-1, "Fire Department" },
    [T_FIRE_DEPT + 6] = { TYPE_FIRE_DEPT,  0,-2, "Fire Department" },
    [T_FIRE_DEPT + 7] = { TYPE_FIRE_DEPT, -1,-2, "Fire Department" },
    [T_FIRE_DEPT + 8] = { TYPE_FIRE_DEPT, -2,-2, "Fire Department" },

    [T_HOSPITAL + 0] = { TYPE_HOSPITAL,  0, 0, "Hospital" },
    [T_HOSPITAL + 1] = { TYPE_HOSPITAL, -1, 0, "Hospital" },
    [T_HOSPITAL + 2] = { TYPE_HOSPITAL, -2, 0, "Hospital" },
    [T_HOSPITAL + 3] = { TYPE_HOSPITAL,  0,-1, "Hospital" },
    [T_HOSPITAL + 4] = { TYPE_HOSPITAL, -1,-1, "Hospital" },
    [T_HOSPITAL + 5] = { TYPE_HOSPITAL, -2,-1, "Hospital" },
    [T_HOSPITAL + 6] = { TYPE_HOSPITAL,  0,-2, "Hospital" },
    [T_HOSPITAL + 7] = { TYPE_HOSPITAL, -1,-2, "Hospital" },
    [T_HOSPITAL + 8] = { TYPE_HOSPITAL, -2,-2, "Hospital" },

    [T_PARK_SMALL + 0] = { TYPE_PARK, 0, 0, "Small Park" },

    [T_PARK_BIG + 0] = { TYPE_PARK,  0, 0, "Big Park" },
    [T_PARK_BIG + 1] = { TYPE_PARK, -1, 0, "Big Park" },
    [T_PARK_BIG + 2] = { TYPE_PARK, -2, 0, "Big Park" },
    [T_PARK_BIG + 3] = { TYPE_PARK,  0,-1, "Big Park" },
    [T_PARK_BIG + 4] = { TYPE_PARK, -1,-1, "Big Park" },
    [T_PARK_BIG + 5] = { TYPE_PARK, -2,-1, "Big Park" },
    [T_PARK_BIG + 6] = { TYPE_PARK,  0,-2, "Big Park" },
    [T_PARK_BIG + 7] = { TYPE_PARK, -1,-2, "Big Park" },
    [T_PARK_BIG + 8] = { TYPE_PARK, -2,-2, "Big Park" },

    [T_STADIUM + 0] = { TYPE_STADIUM,  0, 0, "Stadium" },
    [T_STADIUM + 1] = { TYPE_STADIUM, -1, 0, "Stadium" },
    [T_STADIUM + 2] = { TYPE_STADIUM, -2, 0, "Stadium" },
    [T_STADIUM + 3] = { TYPE_STADIUM, -3, 0, "Stadium" },
    [T_STADIUM + 4] = { TYPE_STADIUM, -4, 0, "Stadium" },
    [T_STADIUM + 5] = { TYPE_STADIUM,  0,-1, "Stadium" },
    [T_STADIUM + 6] = { TYPE_STADIUM, -1,-1, "Stadium" },
    [T_STADIUM + 7] = { TYPE_STADIUM, -2,-1, "Stadium" },
    [T_STADIUM + 8] = { TYPE_STADIUM, -3,-1, "Stadium" },
    [T_STADIUM + 9] = { TYPE_STADIUM, -4,-1, "Stadium" },
    [T_STADIUM + 10] = { TYPE_STADIUM,  0,-2, "Stadium" },
    [T_STADIUM + 11] = { TYPE_STADIUM, -1,-2, "Stadium" },
    [T_STADIUM + 12] = { TYPE_STADIUM, -2,-2, "Stadium" },
    [T_STADIUM + 13] = { TYPE_STADIUM, -3,-2, "Stadium" },
    [T_STADIUM + 14] = { TYPE_STADIUM, -4,-2, "Stadium" },
    [T_STADIUM + 15] = { TYPE_STADIUM,  0,-3, "Stadium" },
    [T_STADIUM + 16] = { TYPE_STADIUM, -1,-3, "Stadium" },
    [T_STADIUM + 17] = { TYPE_STADIUM, -2,-3, "Stadium" },
    [T_STADIUM + 18] = { TYPE_STADIUM, -3,-3, "Stadium" },
    [T_STADIUM + 19] = { TYPE_STADIUM, -4,-3, "Stadium" },

    [T_SCHOOL + 0] = { TYPE_SCHOOL,  0, 0, "School" },
    [T_SCHOOL + 1] = { TYPE_SCHOOL, -1, 0, "School" },
    [T_SCHOOL + 2] = { TYPE_SCHOOL, -2, 0, "School" },
    [T_SCHOOL + 3] = { TYPE_SCHOOL,  0,-1, "School" },
    [T_SCHOOL + 4] = { TYPE_SCHOOL, -1,-1, "School" },
    [T_SCHOOL + 5] = { TYPE_SCHOOL, -2,-1, "School" },

    [T_HIGH_SCHOOL + 0] = { TYPE_HIGH_SCHOOL,  0, 0, "High School" },
    [T_HIGH_SCHOOL + 1] = { TYPE_HIGH_SCHOOL, -1, 0, "High School" },
    [T_HIGH_SCHOOL + 2] = { TYPE_HIGH_SCHOOL, -2, 0, "High School" },
    [T_HIGH_SCHOOL + 3] = { TYPE_HIGH_SCHOOL,  0,-1, "High School" },
    [T_HIGH_SCHOOL + 4] = { TYPE_HIGH_SCHOOL, -1,-1, "High School" },
    [T_HIGH_SCHOOL + 5] = { TYPE_HIGH_SCHOOL, -2,-1, "High School" },
    [T_HIGH_SCHOOL + 6] = { TYPE_HIGH_SCHOOL,  0,-2, "High School" },
    [T_HIGH_SCHOOL + 7] = { TYPE_HIGH_SCHOOL, -1,-2, "High School" },
    [T_HIGH_SCHOOL + 8] = { TYPE_HIGH_SCHOOL, -2,-2, "High School" },

    [T_UNIVERSITY + 0] = { TYPE_UNIVERSITY,  0, 0, "University" },
    [T_UNIVERSITY + 1] = { TYPE_UNIVERSITY, -1, 0, "University" },
    [T_UNIVERSITY + 2] = { TYPE_UNIVERSITY, -2, 0, "University" },
    [T_UNIVERSITY + 3] = { TYPE_UNIVERSITY, -3, 0, "University" },
    [T_UNIVERSITY + 4] = { TYPE_UNIVERSITY, -4, 0, "University" },
    [T_UNIVERSITY + 5] = { TYPE_UNIVERSITY,  0,-1, "University" },
    [T_UNIVERSITY + 6] = { TYPE_UNIVERSITY, -1,-1, "University" },
    [T_UNIVERSITY + 7] = { TYPE_UNIVERSITY, -2,-1, "University" },
    [T_UNIVERSITY + 8] = { TYPE_UNIVERSITY, -3,-1, "University" },
    [T_UNIVERSITY + 9] = { TYPE_UNIVERSITY, -4,-1, "University" },
    [T_UNIVERSITY + 10] = { TYPE_UNIVERSITY,  0,-2, "University" },
    [T_UNIVERSITY + 11] = { TYPE_UNIVERSITY, -1,-2, "University" },
    [T_UNIVERSITY + 12] = { TYPE_UNIVERSITY, -2,-2, "University" },
    [T_UNIVERSITY + 13] = { TYPE_UNIVERSITY, -3,-2, "University" },
    [T_UNIVERSITY + 14] = { TYPE_UNIVERSITY, -4,-2, "University" },
    [T_UNIVERSITY + 15] = { TYPE_UNIVERSITY,  0,-3, "University" },
    [T_UNIVERSITY + 16] = { TYPE_UNIVERSITY, -1,-3, "University" },
    [T_UNIVERSITY + 17] = { TYPE_UNIVERSITY, -2,-3, "University" },
    [T_UNIVERSITY + 18] = { TYPE_UNIVERSITY, -3,-3, "University" },
    [T_UNIVERSITY + 19] = { TYPE_UNIVERSITY, -4,-3, "University" },
    [T_UNIVERSITY + 20] = { TYPE_UNIVERSITY,  0,-4, "University" },
    [T_UNIVERSITY + 21] = { TYPE_UNIVERSITY, -1,-4, "University" },
    [T_UNIVERSITY + 22] = { TYPE_UNIVERSITY, -2,-4, "University" },
    [T_UNIVERSITY + 23] = { TYPE_UNIVERSITY, -3,-4, "University" },
    [T_UNIVERSITY + 24] = { TYPE_UNIVERSITY, -4,-4, "University" },

    [T_MUSEUM + 0] = { TYPE_MUSEUM,  0, 0, "Museum" },
    [T_MUSEUM + 1] = { TYPE_MUSEUM, -1, 0, "Museum" },
    [T_MUSEUM + 2] = { TYPE_MUSEUM, -2, 0, "Museum" },
    [T_MUSEUM + 3] = { TYPE_MUSEUM, -3, 0, "Museum" },
    [T_MUSEUM + 4] = { TYPE_MUSEUM,  0,-1, "Museum" },
    [T_MUSEUM + 5] = { TYPE_MUSEUM, -1,-1, "Museum" },
    [T_MUSEUM + 6] = { TYPE_MUSEUM, -2,-1, "Museum" },
    [T_MUSEUM + 7] = { TYPE_MUSEUM, -3,-1, "Museum" },
    [T_MUSEUM + 8] = { TYPE_MUSEUM,  0,-2, "Museum" },
    [T_MUSEUM + 9] = { TYPE_MUSEUM, -1,-2, "Museum" },
    [T_MUSEUM + 10] = { TYPE_MUSEUM, -2,-2, "Museum" },
    [T_MUSEUM + 11] = { TYPE_MUSEUM, -3,-2, "Museum" },

    [T_LIBRARY + 0] = { TYPE_LIBRARY,  0, 0, "Library" },
    [T_LIBRARY + 1] = { TYPE_LIBRARY, -1, 0, "Library" },
    [T_LIBRARY + 2] = { TYPE_LIBRARY, -2, 0, "Library" },
    [T_LIBRARY + 3] = { TYPE_LIBRARY,  0,-1, "Library" },
    [T_LIBRARY + 4] = { TYPE_LIBRARY, -1,-1, "Library" },
    [T_LIBRARY + 5] = { TYPE_LIBRARY, -2,-1, "Library" },

    [T_AIRPORT + 0] = { TYPE_AIRPORT,  0, 0, "Airport" },
    [T_AIRPORT + 1] = { TYPE_AIRPORT, -1, 0, "Airport" },
    [T_AIRPORT + 2] = { TYPE_AIRPORT, -2, 0, "Airport" },
    [T_AIRPORT + 3] = { TYPE_AIRPORT, -3, 0, "Airport" },
    [T_AIRPORT + 4] = { TYPE_AIRPORT, -4, 0, "Airport" },
    [T_AIRPORT + 5] = { TYPE_AIRPORT,  0,-1, "Airport" },
    [T_AIRPORT + 6] = { TYPE_AIRPORT, -1,-1, "Airport" },
    [T_AIRPORT + 7] = { TYPE_AIRPORT, -2,-1, "Airport" },
    [T_AIRPORT + 8] = { TYPE_AIRPORT, -3,-1, "Airport" },
    [T_AIRPORT + 9] = { TYPE_AIRPORT, -4,-1, "Airport" },
    [T_AIRPORT + 10] = { TYPE_AIRPORT,  0,-2, "Airport" },
    [T_AIRPORT + 11] = { TYPE_AIRPORT, -1,-2, "Airport" },
    [T_AIRPORT + 12] = { TYPE_AIRPORT, -2,-2, "Airport" },
    [T_AIRPORT + 13] = { TYPE_AIRPORT, -3,-2, "Airport" },
    [T_AIRPORT + 14] = { TYPE_AIRPORT, -4,-2, "Airport" },

    [T_PORT + 0] = { TYPE_PORT,  0, 0, "Port" },
    [T_PORT + 1] = { TYPE_PORT, -1, 0, "Port" },
    [T_PORT + 2] = { TYPE_PORT, -2, 0, "Port" },
    [T_PORT + 3] = { TYPE_PORT,  0,-1, "Port" },
    [T_PORT + 4] = { TYPE_PORT, -1,-1, "Port" },
    [T_PORT + 5] = { TYPE_PORT, -2,-1, "Port" },
    [T_PORT + 6] = { TYPE_PORT,  0,-2, "Port" },
    [T_PORT + 7] = { TYPE_PORT, -1,-2, "Port" },
    [T_PORT + 8] = { TYPE_PORT, -2,-2, "Port" },

    [T_PORT_WATER_L] = { TYPE_DOCK,  0, 0, "Dock" },
    [T_PORT_WATER_R] = { TYPE_DOCK,  0, 0, "Dock" },
    [T_PORT_WATER_D] = { TYPE_DOCK,  0, 0, "Dock" },
    [T_PORT_WATER_U] = { TYPE_DOCK,  0, 0, "Dock" },

    [T_POWER_PLANT_COAL + 0] = { TYPE_POWER_PLANT,  0, 0, "Coal Power" },
    [T_POWER_PLANT_COAL + 1] = { TYPE_POWER_PLANT, -1, 0, "Coal Power" },
    [T_POWER_PLANT_COAL + 2] = { TYPE_POWER_PLANT, -2, 0, "Coal Power" },
    [T_POWER_PLANT_COAL + 3] = { TYPE_POWER_PLANT, -3, 0, "Coal Power" },
    [T_POWER_PLANT_COAL + 4] = { TYPE_POWER_PLANT,  0,-1, "Coal Power" },
    [T_POWER_PLANT_COAL + 5] = { TYPE_POWER_PLANT, -1,-1, "Coal Power" },
    [T_POWER_PLANT_COAL + 6] = { TYPE_POWER_PLANT, -2,-1, "Coal Power" },
    [T_POWER_PLANT_COAL + 7] = { TYPE_POWER_PLANT, -3,-1, "Coal Power" },
    [T_POWER_PLANT_COAL + 8] = { TYPE_POWER_PLANT,  0,-2, "Coal Power" },
    [T_POWER_PLANT_COAL + 9] = { TYPE_POWER_PLANT, -1,-2, "Coal Power" },
    [T_POWER_PLANT_COAL + 10] = { TYPE_POWER_PLANT, -2,-2, "Coal Power" },
    [T_POWER_PLANT_COAL + 11] = { TYPE_POWER_PLANT, -3,-2, "Coal Power" },
    [T_POWER_PLANT_COAL + 12] = { TYPE_POWER_PLANT,  0,-3, "Coal Power" },
    [T_POWER_PLANT_COAL + 13] = { TYPE_POWER_PLANT, -1,-3, "Coal Power" },
    [T_POWER_PLANT_COAL + 14] = { TYPE_POWER_PLANT, -2,-3, "Coal Power" },
    [T_POWER_PLANT_COAL + 15] = { TYPE_POWER_PLANT, -3,-3, "Coal Power" },

    [T_POWER_PLANT_OIL + 0] = { TYPE_POWER_PLANT,  0, 0, "Oil Power" },
    [T_POWER_PLANT_OIL + 1] = { TYPE_POWER_PLANT, -1, 0, "Oil Power" },
    [T_POWER_PLANT_OIL + 2] = { TYPE_POWER_PLANT, -2, 0, "Oil Power" },
    [T_POWER_PLANT_OIL + 3] = { TYPE_POWER_PLANT, -3, 0, "Oil Power" },
    [T_POWER_PLANT_OIL + 4] = { TYPE_POWER_PLANT,  0,-1, "Oil Power" },
    [T_POWER_PLANT_OIL + 5] = { TYPE_POWER_PLANT, -1,-1, "Oil Power" },
    [T_POWER_PLANT_OIL + 6] = { TYPE_POWER_PLANT, -2,-1, "Oil Power" },
    [T_POWER_PLANT_OIL + 7] = { TYPE_POWER_PLANT, -3,-1, "Oil Power" },
    [T_POWER_PLANT_OIL + 8] = { TYPE_POWER_PLANT,  0,-2, "Oil Power" },
    [T_POWER_PLANT_OIL + 9] = { TYPE_POWER_PLANT, -1,-2, "Oil Power" },
    [T_POWER_PLANT_OIL + 10] = { TYPE_POWER_PLANT, -2,-2, "Oil Power" },
    [T_POWER_PLANT_OIL + 11] = { TYPE_POWER_PLANT, -3,-2, "Oil Power" },
    [T_POWER_PLANT_OIL + 12] = { TYPE_POWER_PLANT,  0,-3, "Oil Power" },
    [T_POWER_PLANT_OIL + 13] = { TYPE_POWER_PLANT, -1,-3, "Oil Power" },
    [T_POWER_PLANT_OIL + 14] = { TYPE_POWER_PLANT, -2,-3, "Oil Power" },
    [T_POWER_PLANT_OIL + 15] = { TYPE_POWER_PLANT, -3,-3, "Oil Power" },

    [T_POWER_PLANT_WIND + 0] = { TYPE_POWER_PLANT,  0, 0, "Wind Power" },
    [T_POWER_PLANT_WIND + 1] = { TYPE_POWER_PLANT, -1, 0, "Wind Power" },
    [T_POWER_PLANT_WIND + 2] = { TYPE_POWER_PLANT,  0,-1, "Wind Power" },
    [T_POWER_PLANT_WIND + 3] = { TYPE_POWER_PLANT, -1,-1, "Wind Power" },

    [T_POWER_PLANT_SOLAR + 0] = { TYPE_POWER_PLANT,  0, 0, "Solar Power" },
    [T_POWER_PLANT_SOLAR + 1] = { TYPE_POWER_PLANT, -1, 0, "Solar Power" },
    [T_POWER_PLANT_SOLAR + 2] = { TYPE_POWER_PLANT, -2, 0, "Solar Power" },
    [T_POWER_PLANT_SOLAR + 3] = { TYPE_POWER_PLANT, -3, 0, "Solar Power" },
    [T_POWER_PLANT_SOLAR + 4] = { TYPE_POWER_PLANT,  0,-1, "Solar Power" },
    [T_POWER_PLANT_SOLAR + 5] = { TYPE_POWER_PLANT, -1,-1, "Solar Power" },
    [T_POWER_PLANT_SOLAR + 6] = { TYPE_POWER_PLANT, -2,-1, "Solar Power" },
    [T_POWER_PLANT_SOLAR + 7] = { TYPE_POWER_PLANT, -3,-1, "Solar Power" },
    [T_POWER_PLANT_SOLAR + 8] = { TYPE_POWER_PLANT,  0,-2, "Solar Power" },
    [T_POWER_PLANT_SOLAR + 9] = { TYPE_POWER_PLANT, -1,-2, "Solar Power" },
    [T_POWER_PLANT_SOLAR + 10] = { TYPE_POWER_PLANT, -2,-2, "Solar Power" },
    [T_POWER_PLANT_SOLAR + 11] = { TYPE_POWER_PLANT, -3,-2, "Solar Power" },
    [T_POWER_PLANT_SOLAR + 12] = { TYPE_POWER_PLANT,  0,-3, "Solar Power" },
    [T_POWER_PLANT_SOLAR + 13] = { TYPE_POWER_PLANT, -1,-3, "Solar Power" },
    [T_POWER_PLANT_SOLAR + 14] = { TYPE_POWER_PLANT, -2,-3, "Solar Power" },
    [T_POWER_PLANT_SOLAR + 15] = { TYPE_POWER_PLANT, -3,-3, "Solar Power" },

    [T_POWER_PLANT_NUCLEAR + 0] = { TYPE_POWER_PLANT,  0, 0, "Nuclear Power" },
    [T_POWER_PLANT_NUCLEAR + 1] = { TYPE_POWER_PLANT, -1, 0, "Nuclear Power" },
    [T_POWER_PLANT_NUCLEAR + 2] = { TYPE_POWER_PLANT, -2, 0, "Nuclear Power" },
    [T_POWER_PLANT_NUCLEAR + 3] = { TYPE_POWER_PLANT, -3, 0, "Nuclear Power" },
    [T_POWER_PLANT_NUCLEAR + 4] = { TYPE_POWER_PLANT,  0,-1, "Nuclear Power" },
    [T_POWER_PLANT_NUCLEAR + 5] = { TYPE_POWER_PLANT, -1,-1, "Nuclear Power" },
    [T_POWER_PLANT_NUCLEAR + 6] = { TYPE_POWER_PLANT, -2,-1, "Nuclear Power" },
    [T_POWER_PLANT_NUCLEAR + 7] = { TYPE_POWER_PLANT, -3,-1, "Nuclear Power" },
    [T_POWER_PLANT_NUCLEAR + 8] = { TYPE_POWER_PLANT,  0,-2, "Nuclear Power" },
    [T_POWER_PLANT_NUCLEAR + 9] = { TYPE_POWER_PLANT, -1,-2, "Nuclear Power" },
    [T_POWER_PLANT_NUCLEAR + 10] = { TYPE_POWER_PLANT, -2,-2, "Nuclear Power" },
    [T_POWER_PLANT_NUCLEAR + 11] = { TYPE_POWER_PLANT, -3,-2, "Nuclear Power" },
    [T_POWER_PLANT_NUCLEAR + 12] = { TYPE_POWER_PLANT,  0,-3, "Nuclear Power" },
    [T_POWER_PLANT_NUCLEAR + 13] = { TYPE_POWER_PLANT, -1,-3, "Nuclear Power" },
    [T_POWER_PLANT_NUCLEAR + 14] = { TYPE_POWER_PLANT, -2,-3, "Nuclear Power" },
    [T_POWER_PLANT_NUCLEAR + 15] = { TYPE_POWER_PLANT, -3,-3, "Nuclear Power" },

    [T_POWER_PLANT_FUSION + 0] = { TYPE_POWER_PLANT,  0, 0, "Fusion Power" },
    [T_POWER_PLANT_FUSION + 1] = { TYPE_POWER_PLANT, -1, 0, "Fusion Power" },
    [T_POWER_PLANT_FUSION + 2] = { TYPE_POWER_PLANT, -2, 0, "Fusion Power" },
    [T_POWER_PLANT_FUSION + 3] = { TYPE_POWER_PLANT, -3, 0, "Fusion Power" },
    [T_POWER_PLANT_FUSION + 4] = { TYPE_POWER_PLANT,  0,-1, "Fusion Power" },
    [T_POWER_PLANT_FUSION + 5] = { TYPE_POWER_PLANT, -1,-1, "Fusion Power" },
    [T_POWER_PLANT_FUSION + 6] = { TYPE_POWER_PLANT, -2,-1, "Fusion Power" },
    [T_POWER_PLANT_FUSION + 7] = { TYPE_POWER_PLANT, -3,-1, "Fusion Power" },
    [T_POWER_PLANT_FUSION + 8] = { TYPE_POWER_PLANT,  0,-2, "Fusion Power" },
    [T_POWER_PLANT_FUSION + 9] = { TYPE_POWER_PLANT, -1,-2, "Fusion Power" },
    [T_POWER_PLANT_FUSION + 10] = { TYPE_POWER_PLANT, -2,-2, "Fusion Power" },
    [T_POWER_PLANT_FUSION + 11] = { TYPE_POWER_PLANT, -3,-2, "Fusion Power" },
    [T_POWER_PLANT_FUSION + 12] = { TYPE_POWER_PLANT,  0,-3, "Fusion Power" },
    [T_POWER_PLANT_FUSION + 13] = { TYPE_POWER_PLANT, -1,-3, "Fusion Power" },
    [T_POWER_PLANT_FUSION + 14] = { TYPE_POWER_PLANT, -2,-3, "Fusion Power" },
    [T_POWER_PLANT_FUSION + 15] = { TYPE_POWER_PLANT, -3,-3, "Fusion Power" },

    [T_RESIDENTIAL_S1_A] = { TYPE_RESIDENTIAL,  0, 0, "Light Residential" },
    [T_RESIDENTIAL_S1_B] = { TYPE_RESIDENTIAL,  0, 0, "Light Residential" },
    [T_RESIDENTIAL_S1_C] = { TYPE_RESIDENTIAL,  0, 0, "Light Residential" },
    [T_RESIDENTIAL_S1_D] = { TYPE_RESIDENTIAL,  0, 0, "Light Residential" },

    [T_RESIDENTIAL_S2_A + 0] = { TYPE_RESIDENTIAL,  0, 0, "Medium Residential" },
    [T_RESIDENTIAL_S2_A + 1] = { TYPE_RESIDENTIAL, -1, 0, "Medium Residential" },
    [T_RESIDENTIAL_S2_A + 2] = { TYPE_RESIDENTIAL,  0,-1, "Medium Residential" },
    [T_RESIDENTIAL_S2_A + 3] = { TYPE_RESIDENTIAL, -1,-1, "Medium Residential" },

    [T_RESIDENTIAL_S2_B + 0] = { TYPE_RESIDENTIAL,  0, 0, "Medium Residential" },
    [T_RESIDENTIAL_S2_B + 1] = { TYPE_RESIDENTIAL, -1, 0, "Medium Residential" },
    [T_RESIDENTIAL_S2_B + 2] = { TYPE_RESIDENTIAL,  0,-1, "Medium Residential" },
    [T_RESIDENTIAL_S2_B + 3] = { TYPE_RESIDENTIAL, -1,-1, "Medium Residential" },

    [T_RESIDENTIAL_S2_C + 0] = { TYPE_RESIDENTIAL,  0, 0, "Medium Residential" },
    [T_RESIDENTIAL_S2_C + 1] = { TYPE_RESIDENTIAL, -1, 0, "Medium Residential" },
    [T_RESIDENTIAL_S2_C + 2] = { TYPE_RESIDENTIAL,  0,-1, "Medium Residential" },
    [T_RESIDENTIAL_S2_C + 3] = { TYPE_RESIDENTIAL, -1,-1, "Medium Residential" },

    [T_RESIDENTIAL_S2_D + 0] = { TYPE_RESIDENTIAL,  0, 0, "Medium Residential" },
    [T_RESIDENTIAL_S2_D + 1] = { TYPE_RESIDENTIAL, -1, 0, "Medium Residential" },
    [T_RESIDENTIAL_S2_D + 2] = { TYPE_RESIDENTIAL,  0,-1, "Medium Residential" },
    [T_RESIDENTIAL_S2_D + 3] = { TYPE_RESIDENTIAL, -1,-1, "Medium Residential" },

    [T_RESIDENTIAL_S3_A + 0] = { TYPE_RESIDENTIAL,  0, 0, "Dense Residential" },
    [T_RESIDENTIAL_S3_A + 1] = { TYPE_RESIDENTIAL, -1, 0, "Dense Residential" },
    [T_RESIDENTIAL_S3_A + 2] = { TYPE_RESIDENTIAL, -2, 0, "Dense Residential" },
    [T_RESIDENTIAL_S3_A + 3] = { TYPE_RESIDENTIAL,  0,-1, "Dense Residential" },
    [T_RESIDENTIAL_S3_A + 4] = { TYPE_RESIDENTIAL, -1,-1, "Dense Residential" },
    [T_RESIDENTIAL_S3_A + 5] = { TYPE_RESIDENTIAL, -2,-1, "Dense Residential" },
    [T_RESIDENTIAL_S3_A + 6] = { TYPE_RESIDENTIAL,  0,-2, "Dense Residential" },
    [T_RESIDENTIAL_S3_A + 7] = { TYPE_RESIDENTIAL, -1,-2, "Dense Residential" },
    [T_RESIDENTIAL_S3_A + 8] = { TYPE_RESIDENTIAL, -2,-2, "Dense Residential" },

    [T_RESIDENTIAL_S3_B + 0] = { TYPE_RESIDENTIAL,  0, 0, "Dense Residential" },
    [T_RESIDENTIAL_S3_B + 1] = { TYPE_RESIDENTIAL, -1, 0, "Dense Residential" },
    [T_RESIDENTIAL_S3_B + 2] = { TYPE_RESIDENTIAL, -2, 0, "Dense Residential" },
    [T_RESIDENTIAL_S3_B + 3] = { TYPE_RESIDENTIAL,  0,-1, "Dense Residential" },
    [T_RESIDENTIAL_S3_B + 4] = { TYPE_RESIDENTIAL, -1,-1, "Dense Residential" },
    [T_RESIDENTIAL_S3_B + 5] = { TYPE_RESIDENTIAL, -2,-1, "Dense Residential" },
    [T_RESIDENTIAL_S3_B + 6] = { TYPE_RESIDENTIAL,  0,-2, "Dense Residential" },
    [T_RESIDENTIAL_S3_B + 7] = { TYPE_RESIDENTIAL, -1,-2, "Dense Residential" },
    [T_RESIDENTIAL_S3_B + 8] = { TYPE_RESIDENTIAL, -2,-2, "Dense Residential" },

    [T_RESIDENTIAL_S3_C + 0] = { TYPE_RESIDENTIAL,  0, 0, "Dense Residential" },
    [T_RESIDENTIAL_S3_C + 1] = { TYPE_RESIDENTIAL, -1, 0, "Dense Residential" },
    [T_RESIDENTIAL_S3_C + 2] = { TYPE_RESIDENTIAL, -2, 0, "Dense Residential" },
    [T_RESIDENTIAL_S3_C + 3] = { TYPE_RESIDENTIAL,  0,-1, "Dense Residential" },
    [T_RESIDENTIAL_S3_C + 4] = { TYPE_RESIDENTIAL, -1,-1, "Dense Residential" },
    [T_RESIDENTIAL_S3_C + 5] = { TYPE_RESIDENTIAL, -2,-1, "Dense Residential" },
    [T_RESIDENTIAL_S3_C + 6] = { TYPE_RESIDENTIAL,  0,-2, "Dense Residential" },
    [T_RESIDENTIAL_S3_C + 7] = { TYPE_RESIDENTIAL, -1,-2, "Dense Residential" },
    [T_RESIDENTIAL_S3_C + 8] = { TYPE_RESIDENTIAL, -2,-2, "Dense Residential" },

    [T_RESIDENTIAL_S3_D + 0] = { TYPE_RESIDENTIAL,  0, 0, "Dense Residential" },
    [T_RESIDENTIAL_S3_D + 1] = { TYPE_RESIDENTIAL, -1, 0, "Dense Residential" },
    [T_RESIDENTIAL_S3_D + 2] = { TYPE_RESIDENTIAL, -2, 0, "Dense Residential" },
    [T_RESIDENTIAL_S3_D + 3] = { TYPE_RESIDENTIAL,  0,-1, "Dense Residential" },
    [T_RESIDENTIAL_S3_D + 4] = { TYPE_RESIDENTIAL, -1,-1, "Dense Residential" },
    [T_RESIDENTIAL_S3_D + 5] = { TYPE_RESIDENTIAL, -2,-1, "Dense Residential" },
    [T_RESIDENTIAL_S3_D + 6] = { TYPE_RESIDENTIAL,  0,-2, "Dense Residential" },
    [T_RESIDENTIAL_S3_D + 7] = { TYPE_RESIDENTIAL, -1,-2, "Dense Residential" },
    [T_RESIDENTIAL_S3_D + 8] = { TYPE_RESIDENTIAL, -2,-2, "Dense Residential" },

    [T_COMMERCIAL_S1_A] = { TYPE_COMMERCIAL,  0, 0, "Light Commercial" },
    [T_COMMERCIAL_S1_B] = { TYPE_COMMERCIAL,  0, 0, "Light Commercial" },
    [T_COMMERCIAL_S1_C] = { TYPE_COMMERCIAL,  0, 0, "Light Commercial" },
    [T_COMMERCIAL_S1_D] = { TYPE_COMMERCIAL,  0, 0, "Light Commercial" },

    [T_COMMERCIAL_S2_A + 0] = { TYPE_COMMERCIAL,  0, 0, "Medium Commercial" },
    [T_COMMERCIAL_S2_A + 1] = { TYPE_COMMERCIAL, -1, 0, "Medium Commercial" },
    [T_COMMERCIAL_S2_A + 2] = { TYPE_COMMERCIAL,  0,-1, "Medium Commercial" },
    [T_COMMERCIAL_S2_A + 3] = { TYPE_COMMERCIAL, -1,-1, "Medium Commercial" },

    [T_COMMERCIAL_S2_B + 0] = { TYPE_COMMERCIAL,  0, 0, "Medium Commercial" },
    [T_COMMERCIAL_S2_B + 1] = { TYPE_COMMERCIAL, -1, 0, "Medium Commercial" },
    [T_COMMERCIAL_S2_B + 2] = { TYPE_COMMERCIAL,  0,-1, "Medium Commercial" },
    [T_COMMERCIAL_S2_B + 3] = { TYPE_COMMERCIAL, -1,-1, "Medium Commercial" },

    [T_COMMERCIAL_S2_C + 0] = { TYPE_COMMERCIAL,  0, 0, "Medium Commercial" },
    [T_COMMERCIAL_S2_C + 1] = { TYPE_COMMERCIAL, -1, 0, "Medium Commercial" },
    [T_COMMERCIAL_S2_C + 2] = { TYPE_COMMERCIAL,  0,-1, "Medium Commercial" },
    [T_COMMERCIAL_S2_C + 3] = { TYPE_COMMERCIAL, -1,-1, "Medium Commercial" },

    [T_COMMERCIAL_S2_D + 0] = { TYPE_COMMERCIAL,  0, 0, "Medium Commercial" },
    [T_COMMERCIAL_S2_D + 1] = { TYPE_COMMERCIAL, -1, 0, "Medium Commercial" },
    [T_COMMERCIAL_S2_D + 2] = { TYPE_COMMERCIAL,  0,-1, "Medium Commercial" },
    [T_COMMERCIAL_S2_D + 3] = { TYPE_COMMERCIAL, -1,-1, "Medium Commercial" },

    [T_COMMERCIAL_S3_A + 0] = { TYPE_COMMERCIAL,  0, 0, "Dense Commercial" },
    [T_COMMERCIAL_S3_A + 1] = { TYPE_COMMERCIAL, -1, 0, "Dense Commercial" },
    [T_COMMERCIAL_S3_A + 2] = { TYPE_COMMERCIAL, -2, 0, "Dense Commercial" },
    [T_COMMERCIAL_S3_A + 3] = { TYPE_COMMERCIAL,  0,-1, "Dense Commercial" },
    [T_COMMERCIAL_S3_A + 4] = { TYPE_COMMERCIAL, -1,-1, "Dense Commercial" },
    [T_COMMERCIAL_S3_A + 5] = { TYPE_COMMERCIAL, -2,-1, "Dense Commercial" },
    [T_COMMERCIAL_S3_A + 6] = { TYPE_COMMERCIAL,  0,-2, "Dense Commercial" },
    [T_COMMERCIAL_S3_A + 7] = { TYPE_COMMERCIAL, -1,-2, "Dense Commercial" },
    [T_COMMERCIAL_S3_A + 8] = { TYPE_COMMERCIAL, -2,-2, "Dense Commercial" },

    [T_COMMERCIAL_S3_B + 0] = { TYPE_COMMERCIAL,  0, 0, "Dense Commercial" },
    [T_COMMERCIAL_S3_B + 1] = { TYPE_COMMERCIAL, -1, 0, "Dense Commercial" },
    [T_COMMERCIAL_S3_B + 2] = { TYPE_COMMERCIAL, -2, 0, "Dense Commercial" },
    [T_COMMERCIAL_S3_B + 3] = { TYPE_COMMERCIAL,  0,-1, "Dense Commercial" },
    [T_COMMERCIAL_S3_B + 4] = { TYPE_COMMERCIAL, -1,-1, "Dense Commercial" },
    [T_COMMERCIAL_S3_B + 5] = { TYPE_COMMERCIAL, -2,-1, "Dense Commercial" },
    [T_COMMERCIAL_S3_B + 6] = { TYPE_COMMERCIAL,  0,-2, "Dense Commercial" },
    [T_COMMERCIAL_S3_B + 7] = { TYPE_COMMERCIAL, -1,-2, "Dense Commercial" },
    [T_COMMERCIAL_S3_B + 8] = { TYPE_COMMERCIAL, -2,-2, "Dense Commercial" },

    [T_COMMERCIAL_S3_C + 0] = { TYPE_COMMERCIAL,  0, 0, "Dense Commercial" },
    [T_COMMERCIAL_S3_C + 1] = { TYPE_COMMERCIAL, -1, 0, "Dense Commercial" },
    [T_COMMERCIAL_S3_C + 2] = { TYPE_COMMERCIAL, -2, 0, "Dense Commercial" },
    [T_COMMERCIAL_S3_C + 3] = { TYPE_COMMERCIAL,  0,-1, "Dense Commercial" },
    [T_COMMERCIAL_S3_C + 4] = { TYPE_COMMERCIAL, -1,-1, "Dense Commercial" },
    [T_COMMERCIAL_S3_C + 5] = { TYPE_COMMERCIAL, -2,-1, "Dense Commercial" },
    [T_COMMERCIAL_S3_C + 6] = { TYPE_COMMERCIAL,  0,-2, "Dense Commercial" },
    [T_COMMERCIAL_S3_C + 7] = { TYPE_COMMERCIAL, -1,-2, "Dense Commercial" },
    [T_COMMERCIAL_S3_C + 8] = { TYPE_COMMERCIAL, -2,-2, "Dense Commercial" },

    [T_COMMERCIAL_S3_D + 0] = { TYPE_COMMERCIAL,  0, 0, "Dense Commercial" },
    [T_COMMERCIAL_S3_D + 1] = { TYPE_COMMERCIAL, -1, 0, "Dense Commercial" },
    [T_COMMERCIAL_S3_D + 2] = { TYPE_COMMERCIAL, -2, 0, "Dense Commercial" },
    [T_COMMERCIAL_S3_D + 3] = { TYPE_COMMERCIAL,  0,-1, "Dense Commercial" },
    [T_COMMERCIAL_S3_D + 4] = { TYPE_COMMERCIAL, -1,-1, "Dense Commercial" },
    [T_COMMERCIAL_S3_D + 5] = { TYPE_COMMERCIAL, -2,-1, "Dense Commercial" },
    [T_COMMERCIAL_S3_D + 6] = { TYPE_COMMERCIAL,  0,-2, "Dense Commercial" },
    [T_COMMERCIAL_S3_D + 7] = { TYPE_COMMERCIAL, -1,-2, "Dense Commercial" },
    [T_COMMERCIAL_S3_D + 8] = { TYPE_COMMERCIAL, -2,-2, "Dense Commercial" },

    [T_INDUSTRIAL_S1_A] = { TYPE_INDUSTRIAL,  0, 0, "Light Industrial" },
    [T_INDUSTRIAL_S1_B] = { TYPE_INDUSTRIAL,  0, 0, "Light Industrial" },
    [T_INDUSTRIAL_S1_C] = { TYPE_INDUSTRIAL,  0, 0, "Light Industrial" },
    [T_INDUSTRIAL_S1_D] = { TYPE_INDUSTRIAL,  0, 0, "Light Industrial" },

    [T_INDUSTRIAL_S2_A + 0] = { TYPE_INDUSTRIAL,  0, 0, "Medium Industrial" },
    [T_INDUSTRIAL_S2_A + 1] = { TYPE_INDUSTRIAL, -1, 0, "Medium Industrial" },
    [T_INDUSTRIAL_S2_A + 2] = { TYPE_INDUSTRIAL,  0,-1, "Medium Industrial" },
    [T_INDUSTRIAL_S2_A + 3] = { TYPE_INDUSTRIAL, -1,-1, "Medium Industrial" },

    [T_INDUSTRIAL_S2_B + 0] = { TYPE_INDUSTRIAL,  0, 0, "Medium Industrial" },
    [T_INDUSTRIAL_S2_B + 1] = { TYPE_INDUSTRIAL, -1, 0, "Medium Industrial" },
    [T_INDUSTRIAL_S2_B + 2] = { TYPE_INDUSTRIAL,  0,-1, "Medium Industrial" },
    [T_INDUSTRIAL_S2_B + 3] = { TYPE_INDUSTRIAL, -1,-1, "Medium Industrial" },

    [T_INDUSTRIAL_S2_C + 0] = { TYPE_INDUSTRIAL,  0, 0, "Medium Industrial" },
    [T_INDUSTRIAL_S2_C + 1] = { TYPE_INDUSTRIAL, -1, 0, "Medium Industrial" },
    [T_INDUSTRIAL_S2_C + 2] = { TYPE_INDUSTRIAL,  0,-1, "Medium Industrial" },
    [T_INDUSTRIAL_S2_C + 3] = { TYPE_INDUSTRIAL, -1,-1, "Medium Industrial" },

    [T_INDUSTRIAL_S2_D + 0] = { TYPE_INDUSTRIAL,  0, 0, "Medium Industrial" },
    [T_INDUSTRIAL_S2_D + 1] = { TYPE_INDUSTRIAL, -1, 0, "Medium Industrial" },
    [T_INDUSTRIAL_S2_D + 2] = { TYPE_INDUSTRIAL,  0,-1, "Medium Industrial" },
    [T_INDUSTRIAL_S2_D + 3] = { TYPE_INDUSTRIAL, -1,-1, "Medium Industrial" },

    [T_INDUSTRIAL_S3_A + 0] = { TYPE_INDUSTRIAL,  0, 0, "Dense Industrial" },
    [T_INDUSTRIAL_S3_A + 1] = { TYPE_INDUSTRIAL, -1, 0, "Dense Industrial" },
    [T_INDUSTRIAL_S3_A + 2] = { TYPE_INDUSTRIAL, -2, 0, "Dense Industrial" },
    [T_INDUSTRIAL_S3_A + 3] = { TYPE_INDUSTRIAL,  0,-1, "Dense Industrial" },
    [T_INDUSTRIAL_S3_A + 4] = { TYPE_INDUSTRIAL, -1,-1, "Dense Industrial" },
    [T_INDUSTRIAL_S3_A + 5] = { TYPE_INDUSTRIAL, -2,-1, "Dense Industrial" },
    [T_INDUSTRIAL_S3_A + 6] = { TYPE_INDUSTRIAL,  0,-2, "Dense Industrial" },
    [T_INDUSTRIAL_S3_A + 7] = { TYPE_INDUSTRIAL, -1,-2, "Dense Industrial" },
    [T_INDUSTRIAL_S3_A + 8] = { TYPE_INDUSTRIAL, -2,-2, "Dense Industrial" },

    [T_INDUSTRIAL_S3_B + 0] = { TYPE_INDUSTRIAL,  0, 0, "Dense Industrial" },
    [T_INDUSTRIAL_S3_B + 1] = { TYPE_INDUSTRIAL, -1, 0, "Dense Industrial" },
    [T_INDUSTRIAL_S3_B + 2] = { TYPE_INDUSTRIAL, -2, 0, "Dense Industrial" },
    [T_INDUSTRIAL_S3_B + 3] = { TYPE_INDUSTRIAL,  0,-1, "Dense Industrial" },
    [T_INDUSTRIAL_S3_B + 4] = { TYPE_INDUSTRIAL, -1,-1, "Dense Industrial" },
    [T_INDUSTRIAL_S3_B + 5] = { TYPE_INDUSTRIAL, -2,-1, "Dense Industrial" },
    [T_INDUSTRIAL_S3_B + 6] = { TYPE_INDUSTRIAL,  0,-2, "Dense Industrial" },
    [T_INDUSTRIAL_S3_B + 7] = { TYPE_INDUSTRIAL, -1,-2, "Dense Industrial" },
    [T_INDUSTRIAL_S3_B + 8] = { TYPE_INDUSTRIAL, -2,-2, "Dense Industrial" },

    [T_INDUSTRIAL_S3_C + 0] = { TYPE_INDUSTRIAL,  0, 0, "Dense Industrial" },
    [T_INDUSTRIAL_S3_C + 1] = { TYPE_INDUSTRIAL, -1, 0, "Dense Industrial" },
    [T_INDUSTRIAL_S3_C + 2] = { TYPE_INDUSTRIAL, -2, 0, "Dense Industrial" },
    [T_INDUSTRIAL_S3_C + 3] = { TYPE_INDUSTRIAL,  0,-1, "Dense Industrial" },
    [T_INDUSTRIAL_S3_C + 4] = { TYPE_INDUSTRIAL, -1,-1, "Dense Industrial" },
    [T_INDUSTRIAL_S3_C + 5] = { TYPE_INDUSTRIAL, -2,-1, "Dense Industrial" },
    [T_INDUSTRIAL_S3_C + 6] = { TYPE_INDUSTRIAL,  0,-2, "Dense Industrial" },
    [T_INDUSTRIAL_S3_C + 7] = { TYPE_INDUSTRIAL, -1,-2, "Dense Industrial" },
    [T_INDUSTRIAL_S3_C + 8] = { TYPE_INDUSTRIAL, -2,-2, "Dense Industrial" },

    [T_INDUSTRIAL_S3_D + 0] = { TYPE_INDUSTRIAL,  0, 0, "Dense Industrial" },
    [T_INDUSTRIAL_S3_D + 1] = { TYPE_INDUSTRIAL, -1, 0, "Dense Industrial" },
    [T_INDUSTRIAL_S3_D + 2] = { TYPE_INDUSTRIAL, -2, 0, "Dense Industrial" },
    [T_INDUSTRIAL_S3_D + 3] = { TYPE_INDUSTRIAL,  0,-1, "Dense Industrial" },
    [T_INDUSTRIAL_S3_D + 4] = { TYPE_INDUSTRIAL, -1,-1, "Dense Industrial" },
    [T_INDUSTRIAL_S3_D + 5] = { TYPE_INDUSTRIAL, -2,-1, "Dense Industrial" },
    [T_INDUSTRIAL_S3_D + 6] = { TYPE_INDUSTRIAL,  0,-2, "Dense Industrial" },
    [T_INDUSTRIAL_S3_D + 7] = { TYPE_INDUSTRIAL, -1,-2, "Dense Industrial" },
    [T_INDUSTRIAL_S3_D + 8] = { TYPE_INDUSTRIAL, -2,-2, "Dense Industrial" },

    [T_FIRE_1] = { TYPE_FIRE, 0, 0, "Fire" },
    [T_FIRE_2] = { TYPE_FIRE, 0, 0, "Fire" },

    [T_RADIATION_GROUND] = { TYPE_RADIATION, 0, 0, "Radiation Ground" },
    [T_RADIATION_WATER] = { TYPE_RADIATION, 0, 0, "Radiation Water" },
};

city_tile_info *City_Tileset_Entry_Info(uint16_t city_index)
{
    UGBA_Assert(city_index < city_tileset_map_map_height);
    return &city_tileset_info[city_index];
}
