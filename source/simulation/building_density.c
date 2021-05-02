// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include "simulation/building_density.h"
#include "room_game/tileset_info.h"

static city_tile_density_info CITY_TILE_DENSITY[] = {
    [T_GRASS__FOREST_TL]        = { 0, 0, 0, 12 },
    [T_GRASS__FOREST_TC]        = { 0, 0, 0, 12 },
    [T_GRASS__FOREST_TR]        = { 0, 0, 0, 12 },
    [T_GRASS__FOREST_CL]        = { 0, 0, 0, 12 },
    [T_GRASS]                   = { 0, 0, 0, 0 },
    [T_GRASS__FOREST_CR]        = { 0, 0, 0, 12 },
    [T_GRASS__FOREST_BL]        = { 0, 0, 0, 12 },
    [T_GRASS__FOREST_BC]        = { 0, 0, 0, 12 },
    [T_GRASS__FOREST_BR]        = { 0, 0, 0, 12 },
    [T_GRASS__FOREST_CORNER_TL] = { 0, 0, 0, 12 },
    [T_GRASS__FOREST_CORNER_TR] = { 0, 0, 0, 12 },
    [T_GRASS__FOREST_CORNER_BL] = { 0, 0, 0, 12 },
    [T_GRASS__FOREST_CORNER_BR] = { 0, 0, 0, 12 },
    [T_FOREST]                  = { 0, 0, 0, 12 },
    [T_GRASS_EXTRA]             = { 0, 0, 0, 0 },
    [T_FOREST_EXTRA]            = { 0, 0, 0, 12 },

    [T_WATER__GRASS_TL]         = { 0, 0, 0, 0 },
    [T_WATER__GRASS_TC]         = { 0, 0, 0, 0 },
    [T_WATER__GRASS_TR]         = { 0, 0, 0, 0 },
    [T_WATER__GRASS_CL]         = { 0, 0, 0, 0 },
    [T_WATER]                   = { 0, 0, 0, 0 },
    [T_WATER__GRASS_CR]         = { 0, 0, 0, 0 },
    [T_WATER__GRASS_BL]         = { 0, 0, 0, 0 },
    [T_WATER__GRASS_BC]         = { 0, 0, 0, 0 },
    [T_WATER__GRASS_BR]         = { 0, 0, 0, 0 },
    [T_WATER__GRASS_CORNER_TL]  = { 0, 0, 0, 0 },
    [T_WATER__GRASS_CORNER_TR]  = { 0, 0, 0, 0 },
    [T_WATER__GRASS_CORNER_BL]  = { 0, 0, 0, 0 },
    [T_WATER__GRASS_CORNER_BR]  = { 0, 0, 0, 0 },
    [T_WATER_EXTRA]             = { 0, 0, 0, 0 },

    [T_RESIDENTIAL]             = { 0, 1, 0, 12 },
    [T_COMMERCIAL]              = { 0, 1, 0, 12 },
    [T_INDUSTRIAL]              = { 0, 1, 0, 12 },
    [T_DEMOLISHED]              = { 0, 0, 0, 0 },

    [T_ROAD_TB]                 = { 0, 0, 0, 0 }, // Road pollution is 0,
    [T_ROAD_TB_1]               = { 0, 0, 0, 0 }, // it is calculated from
    [T_ROAD_TB_2]               = { 0, 0, 0, 0 }, // the traffic level.
    [T_ROAD_TB_3]               = { 0, 0, 0, 0 },
    [T_ROAD_LR]                 = { 0, 0, 0, 0 },
    [T_ROAD_LR_1]               = { 0, 0, 0, 0 },
    [T_ROAD_LR_2]               = { 0, 0, 0, 0 },
    [T_ROAD_LR_3]               = { 0, 0, 0, 0 },
    [T_ROAD_RB]                 = { 0, 0, 0, 0 },
    [T_ROAD_LB]                 = { 0, 0, 0, 0 },
    [T_ROAD_TR]                 = { 0, 0, 0, 0 },
    [T_ROAD_TL]                 = { 0, 0, 0, 0 },
    [T_ROAD_TRB]                = { 0, 0, 0, 0 },
    [T_ROAD_LRB]                = { 0, 0, 0, 0 },
    [T_ROAD_TLB]                = { 0, 0, 0, 0 },
    [T_ROAD_TLR]                = { 0, 0, 0, 0 },
    [T_ROAD_TLRB]               = { 0, 0, 0, 0 },
    [T_ROAD_TB_POWER_LINES]     = { 0, 1, 0, 12 },
    [T_ROAD_LR_POWER_LINES]     = { 0, 1, 0, 12 },
    [T_ROAD_TB_BRIDGE]          = { 0, 0, 0, 0 },
    [T_ROAD_LR_BRIDGE]          = { 0, 0, 0, 0 },

    [T_TRAIN_TB]                = { 0, 0, 0, 0 },
    [T_TRAIN_LR]                = { 0, 0, 0, 0 },
    [T_TRAIN_RB]                = { 0, 0, 0, 0 },
    [T_TRAIN_LB]                = { 0, 0, 0, 0 },
    [T_TRAIN_TR]                = { 0, 0, 0, 0 },
    [T_TRAIN_TL]                = { 0, 0, 0, 0 },
    [T_TRAIN_TRB]               = { 0, 0, 0, 0 },
    [T_TRAIN_LRB]               = { 0, 0, 0, 0 },
    [T_TRAIN_TLB]               = { 0, 0, 0, 0 },
    [T_TRAIN_TLR]               = { 0, 0, 0, 0 },
    [T_TRAIN_TLRB]              = { 0, 0, 0, 0 },
    [T_TRAIN_LR_ROAD]           = { 0, 0, 0, 0 },
    [T_TRAIN_TB_ROAD]           = { 0, 0, 0, 0 },
    [T_TRAIN_TB_POWER_LINES]    = { 0, 1, 0, 12 },
    [T_TRAIN_LR_POWER_LINES]    = { 0, 1, 0, 12 },
    [T_TRAIN_TB_BRIDGE]         = { 0, 0, 0, 0 },
    [T_TRAIN_LR_BRIDGE]         = { 0, 0, 0, 0 },

    [T_POWER_LINES_TB]          = { 0, 1, 0, 12 },
    [T_POWER_LINES_LR]          = { 0, 1, 0, 12 },
    [T_POWER_LINES_RB]          = { 0, 1, 0, 12 },
    [T_POWER_LINES_LB]          = { 0, 1, 0, 12 },
    [T_POWER_LINES_TR]          = { 0, 1, 0, 12 },
    [T_POWER_LINES_TL]          = { 0, 1, 0, 12 },
    [T_POWER_LINES_TRB]         = { 0, 1, 0, 12 },
    [T_POWER_LINES_LRB]         = { 0, 1, 0, 12 },
    [T_POWER_LINES_TLB]         = { 0, 1, 0, 12 },
    [T_POWER_LINES_TLR]         = { 0, 1, 0, 12 },
    [T_POWER_LINES_TLRB]        = { 0, 1, 0, 12 },
    [T_POWER_LINES_TB_BRIDGE]   = { 0, 1, 0, 12 }, // This is the only bridge
    [T_POWER_LINES_LR_BRIDGE]   = { 0, 1, 0, 12 }, // that can burn.

    [T_POLICE_DEPT]             = { 1 * 9, 1, 0, 12 },
    [T_FIRE_DEPT]               = { 1 * 9, 1, 0, 6 },
    [T_HOSPITAL]                = { 2 * 9, 1, 0, 12 },

    [T_PARK_SMALL]              = { 2 * 1, 1, 0, 12 },
    [T_PARK_BIG]                = { 2 * 9, 1, 0, 12 },
    [T_STADIUM]                 = { 3 * 15, 20, 0, 32 },

    [T_SCHOOL]                  = { 2 * 6, 5, 0, 12 },
    [T_HIGH_SCHOOL]             = { 2 * 9, 6, 0, 12 },
    [T_UNIVERSITY]              = { 2 * 25, 7, 0, 12 },
    [T_MUSEUM]                  = { 1 * 12, 6, 0, 12 },
    [T_LIBRARY]                 = { 1 * 6, 5, 0, 12 },

    [T_AIRPORT]                 = { 2 * 15, 10, 128, 20 },
    [T_PORT]                    = { 1 * 9, 8, 128, 20 },
    [T_PORT_WATER_L]            = { 0, 0, 32, 0 }, // Docks don't burn, only the
    [T_PORT_WATER_R]            = { 0, 0, 32, 0 }, // port. The docks are
    [T_PORT_WATER_D]            = { 0, 0, 32, 0 }, // destroyed when the port is
    [T_PORT_WATER_U]            = { 0, 0, 32, 0 }, // destroyed.

    [T_POWER_PLANT_COAL]        = { 1 * 16, 0, 255, 24 }, // No energetic cost,
    [T_POWER_PLANT_OIL]         = { 1 * 16, 0, 232, 24 }, // power plants are
    [T_POWER_PLANT_WIND]        = { 1 * 4, 0, 0, 4 },     // generators.
    [T_POWER_PLANT_SOLAR]       = { 1 * 16, 0, 0, 4 },
    [T_POWER_PLANT_NUCLEAR]     = { 2 * 16, 0, 0, 4 },
    [T_POWER_PLANT_FUSION]      = { 3 * 16, 0, 0, 4 },

    [T_RESIDENTIAL_S1_A]        = { 6 * 1, 2, 0, 6 },
    [T_RESIDENTIAL_S1_B]        = { 7 * 1, 2, 0, 6 },
    [T_RESIDENTIAL_S1_C]        = { 7 * 1, 2, 0, 6 },
    [T_RESIDENTIAL_S1_D]        = { 8 * 1, 2, 0, 6 },

    [T_RESIDENTIAL_S2_A]        = {  9 * 4, 3, 0, 8 },
    [T_RESIDENTIAL_S2_B]        = { 10 * 4, 3, 0, 8 },
    [T_RESIDENTIAL_S2_C]        = { 10 * 4, 3, 0, 8 },
    [T_RESIDENTIAL_S2_D]        = { 10 * 4, 3, 0, 8 },

    [T_RESIDENTIAL_S3_A]        = { 11 * 9, 5, 0, 12 },
    [T_RESIDENTIAL_S3_B]        = { 11 * 9, 5, 0, 12 },
    [T_RESIDENTIAL_S3_C]        = { 11 * 9, 5, 0, 12 },
    [T_RESIDENTIAL_S3_D]        = { 12 * 9, 5, 0, 12 },

    [T_COMMERCIAL_S1_A]         = { 1 * 1, 2, 0, 8 },
    [T_COMMERCIAL_S1_B]         = { 1 * 1, 2, 0, 8 },
    [T_COMMERCIAL_S1_C]         = { 2 * 1, 2, 0, 8 },
    [T_COMMERCIAL_S1_D]         = { 2 * 1, 2, 0, 8 },

    [T_COMMERCIAL_S2_A]         = { 2 * 4, 3, 0, 12 },
    [T_COMMERCIAL_S2_B]         = { 2 * 4, 3, 0, 12 },
    [T_COMMERCIAL_S2_C]         = { 3 * 4, 3, 0, 12 },
    [T_COMMERCIAL_S2_D]         = { 3 * 4, 3, 0, 12 },

    [T_COMMERCIAL_S3_A]         = { 4 * 9, 5, 0, 16 },
    [T_COMMERCIAL_S3_B]         = { 4 * 9, 5, 0, 16 },
    [T_COMMERCIAL_S3_C]         = { 5 * 9, 5, 0, 16 },
    [T_COMMERCIAL_S3_D]         = { 5 * 9, 5, 0, 16 },

    [T_INDUSTRIAL_S1_A]         = { 1 * 1, 2, 128, 12 },
    [T_INDUSTRIAL_S1_B]         = { 2 * 1, 2, 128, 12 },
    [T_INDUSTRIAL_S1_C]         = { 2 * 1, 2, 128, 12 },
    [T_INDUSTRIAL_S1_D]         = { 2 * 1, 2, 128, 12 },

    [T_INDUSTRIAL_S2_A]         = { 3 * 4, 6, 192, 16 },
    [T_INDUSTRIAL_S2_B]         = { 3 * 4, 6, 192, 16 },
    [T_INDUSTRIAL_S2_C]         = { 4 * 4, 6, 192, 16 },
    [T_INDUSTRIAL_S2_D]         = { 4 * 4, 6, 192, 16 },

    [T_INDUSTRIAL_S3_A]         = { 5 * 9, 10, 255, 20 },
    [T_INDUSTRIAL_S3_B]         = { 5 * 9, 10, 255, 20 },
    [T_INDUSTRIAL_S3_C]         = { 5 * 9, 10, 255, 20 },
    [T_INDUSTRIAL_S3_D]         = { 6 * 9, 10, 255, 20 },

    // 1) Pollution isn't simulated in disaster mode.
    // 2) Fire can't catch fire!
    [T_FIRE_1]                  = { 0, 0, 0, 0 },
    [T_FIRE_2]                  = { 0, 0, 0, 0 },

    [T_RADIATION_GROUND]        = { 0, 0, 0, 0 }, // Radiation can't catch fire!
    [T_RADIATION_WATER]         = { 0, 0, 0, 0 },
};

city_tile_density_info *CityTileDensityInfo(uint16_t tile)
{
    return &CITY_TILE_DENSITY[tile];
}
