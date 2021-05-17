// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include "money.h"
#include "room_game/draw_common.h"
#include "room_game/room_game.h"
#include "room_game/text_messages.h"
#include "room_game/tileset_info.h"

// Max amount of money per tile * tiles in map = 99 * 64 * 64 = 175890

static int32_t taxes_rci; // Residential, commercial, industrial
static int32_t taxes_other; // Stadium, airport, seaport
static int32_t budget_police;
static int32_t budget_firemen;
static int32_t budget_healthcare; // Hospital, park
static int32_t budget_education; // Schools, university, museum, library
static int32_t budget_transport; // Road, train tracks

static int32_t budget_result;

static uint32_t tax_percentage;

// Number of quarters with negative budgets and negative funds
static int negative_budget_count;

// Cost and income is per-tile. The amount can be 0-99
// Depending on the tile, cost or income.
static int city_tile_money_cost[] = {
    [T_GRASS__FOREST_TL]        = 0,
    [T_GRASS__FOREST_TC]        = 0,
    [T_GRASS__FOREST_TR]        = 0,
    [T_GRASS__FOREST_CL]        = 0,
    [T_GRASS]                   = 0,
    [T_GRASS__FOREST_CR]        = 0,
    [T_GRASS__FOREST_BL]        = 0,
    [T_GRASS__FOREST_BC]        = 0,
    [T_GRASS__FOREST_BR]        = 0,
    [T_GRASS__FOREST_CORNER_TL] = 0,
    [T_GRASS__FOREST_CORNER_TR] = 0,
    [T_GRASS__FOREST_CORNER_BL] = 0,
    [T_GRASS__FOREST_CORNER_BR] = 0,
    [T_FOREST]                  = 0,
    [T_GRASS_EXTRA]             = 0,
    [T_FOREST_EXTRA]            = 0,

    [T_WATER__GRASS_TL]         = 0,
    [T_WATER__GRASS_TC]         = 0,
    [T_WATER__GRASS_TR]         = 0,
    [T_WATER__GRASS_CL]         = 0,
    [T_WATER]                   = 0,
    [T_WATER__GRASS_CR]         = 0,
    [T_WATER__GRASS_BL]         = 0,
    [T_WATER__GRASS_BC]         = 0,
    [T_WATER__GRASS_BR]         = 0,
    [T_WATER__GRASS_CORNER_TL]  = 0,
    [T_WATER__GRASS_CORNER_TR]  = 0,
    [T_WATER__GRASS_CORNER_BL]  = 0,
    [T_WATER__GRASS_CORNER_BR]  = 0,
    [T_WATER_EXTRA]             = 0,

    [T_RESIDENTIAL]             = 1,
    [T_COMMERCIAL]              = 1,
    [T_INDUSTRIAL]              = 1,
    [T_DEMOLISHED]              = 0,

#define ROAD_MAINTENANCE        1
#define TRAIN_MAINTENANCE       2
#define POWER_MAINTENANCE       1

    [T_ROAD_TB]                 = ROAD_MAINTENANCE,
    [T_ROAD_TB_1]               = ROAD_MAINTENANCE,
    [T_ROAD_TB_2]               = ROAD_MAINTENANCE,
    [T_ROAD_TB_3]               = ROAD_MAINTENANCE,
    [T_ROAD_LR]                 = ROAD_MAINTENANCE,
    [T_ROAD_LR_1]               = ROAD_MAINTENANCE,
    [T_ROAD_LR_2]               = ROAD_MAINTENANCE,
    [T_ROAD_LR_3]               = ROAD_MAINTENANCE,
    [T_ROAD_RB]                 = ROAD_MAINTENANCE,
    [T_ROAD_LB]                 = ROAD_MAINTENANCE,
    [T_ROAD_TR]                 = ROAD_MAINTENANCE,
    [T_ROAD_TL]                 = ROAD_MAINTENANCE,
    [T_ROAD_TRB]                = ROAD_MAINTENANCE,
    [T_ROAD_LRB]                = ROAD_MAINTENANCE,
    [T_ROAD_TLB]                = ROAD_MAINTENANCE,
    [T_ROAD_TLR]                = ROAD_MAINTENANCE,
    [T_ROAD_TLRB]               = ROAD_MAINTENANCE,
    [T_ROAD_TB_POWER_LINES]     = ROAD_MAINTENANCE + POWER_MAINTENANCE,
    [T_ROAD_LR_POWER_LINES]     = ROAD_MAINTENANCE + POWER_MAINTENANCE,
    [T_ROAD_TB_BRIDGE]          = ROAD_MAINTENANCE * 2,
    [T_ROAD_LR_BRIDGE]          = ROAD_MAINTENANCE * 2,

    [T_TRAIN_TB]                = TRAIN_MAINTENANCE,
    [T_TRAIN_LR]                = TRAIN_MAINTENANCE,
    [T_TRAIN_RB]                = TRAIN_MAINTENANCE,
    [T_TRAIN_LB]                = TRAIN_MAINTENANCE,
    [T_TRAIN_TR]                = TRAIN_MAINTENANCE,
    [T_TRAIN_TL]                = TRAIN_MAINTENANCE,
    [T_TRAIN_TRB]               = TRAIN_MAINTENANCE,
    [T_TRAIN_LRB]               = TRAIN_MAINTENANCE,
    [T_TRAIN_TLB]               = TRAIN_MAINTENANCE,
    [T_TRAIN_TLR]               = TRAIN_MAINTENANCE,
    [T_TRAIN_TLRB]              = TRAIN_MAINTENANCE,
    [T_TRAIN_LR_ROAD]           = TRAIN_MAINTENANCE + ROAD_MAINTENANCE,
    [T_TRAIN_TB_ROAD]           = TRAIN_MAINTENANCE + ROAD_MAINTENANCE,
    [T_TRAIN_TB_POWER_LINES]    = TRAIN_MAINTENANCE + POWER_MAINTENANCE,
    [T_TRAIN_LR_POWER_LINES]    = TRAIN_MAINTENANCE + POWER_MAINTENANCE,
    [T_TRAIN_TB_BRIDGE]         = TRAIN_MAINTENANCE * 2,
    [T_TRAIN_LR_BRIDGE]         = TRAIN_MAINTENANCE * 2,

    [T_POWER_LINES_TB]          = POWER_MAINTENANCE,
    [T_POWER_LINES_LR]          = POWER_MAINTENANCE,
    [T_POWER_LINES_RB]          = POWER_MAINTENANCE,
    [T_POWER_LINES_LB]          = POWER_MAINTENANCE,
    [T_POWER_LINES_TR]          = POWER_MAINTENANCE,
    [T_POWER_LINES_TL]          = POWER_MAINTENANCE,
    [T_POWER_LINES_TRB]         = POWER_MAINTENANCE,
    [T_POWER_LINES_LRB]         = POWER_MAINTENANCE,
    [T_POWER_LINES_TLB]         = POWER_MAINTENANCE,
    [T_POWER_LINES_TLR]         = POWER_MAINTENANCE,
    [T_POWER_LINES_TLRB]        = POWER_MAINTENANCE,
    [T_POWER_LINES_TB_BRIDGE]   = POWER_MAINTENANCE * 2,
    [T_POWER_LINES_LR_BRIDGE]   = POWER_MAINTENANCE * 2,

    [T_POLICE_DEPT + 0]         = 10,
    [T_POLICE_DEPT + 1]         = 10,
    [T_POLICE_DEPT + 2]         = 10,
    [T_POLICE_DEPT + 3]         = 10,
    [T_POLICE_DEPT + 4]         = 10,
    [T_POLICE_DEPT + 5]         = 10,
    [T_POLICE_DEPT + 6]         = 10,
    [T_POLICE_DEPT + 7]         = 10,
    [T_POLICE_DEPT + 8]         = 10,

    [T_FIRE_DEPT + 0]           = 10,
    [T_FIRE_DEPT + 1]           = 10,
    [T_FIRE_DEPT + 2]           = 10,
    [T_FIRE_DEPT + 3]           = 10,
    [T_FIRE_DEPT + 4]           = 10,
    [T_FIRE_DEPT + 5]           = 10,
    [T_FIRE_DEPT + 6]           = 10,
    [T_FIRE_DEPT + 7]           = 10,
    [T_FIRE_DEPT + 8]           = 10,

    [T_HOSPITAL + 0]            = 20,
    [T_HOSPITAL + 1]            = 20,
    [T_HOSPITAL + 2]            = 20,
    [T_HOSPITAL + 3]            = 20,
    [T_HOSPITAL + 4]            = 20,
    [T_HOSPITAL + 5]            = 20,
    [T_HOSPITAL + 6]            = 20,
    [T_HOSPITAL + 7]            = 20,
    [T_HOSPITAL + 8]            = 20,

    [T_PARK_SMALL + 0]          = 5, // Cost

    [T_PARK_BIG + 0]            = 5, // Cost
    [T_PARK_BIG + 1]            = 5,
    [T_PARK_BIG + 2]            = 5,
    [T_PARK_BIG + 3]            = 5,
    [T_PARK_BIG + 4]            = 5,
    [T_PARK_BIG + 5]            = 5,
    [T_PARK_BIG + 6]            = 5,
    [T_PARK_BIG + 7]            = 5,
    [T_PARK_BIG + 8]            = 5,

    [T_STADIUM + 0]             = 10, // Income
    [T_STADIUM + 1]             = 10,
    [T_STADIUM + 2]             = 10,
    [T_STADIUM + 3]             = 10,
    [T_STADIUM + 4]             = 10,
    [T_STADIUM + 5]             = 10,
    [T_STADIUM + 6]             = 10,
    [T_STADIUM + 7]             = 10,
    [T_STADIUM + 8]             = 10,
    [T_STADIUM + 9]             = 10,
    [T_STADIUM + 10]            = 10,
    [T_STADIUM + 11]            = 10,
    [T_STADIUM + 12]            = 10,
    [T_STADIUM + 13]            = 10,
    [T_STADIUM + 14]            = 10,
    [T_STADIUM + 15]            = 10,
    [T_STADIUM + 16]            = 10,
    [T_STADIUM + 17]            = 10,
    [T_STADIUM + 18]            = 10,
    [T_STADIUM + 19]            = 10,

    [T_SCHOOL + 0]              = 5, // Cost
    [T_SCHOOL + 1]              = 5,
    [T_SCHOOL + 2]              = 5,
    [T_SCHOOL + 3]              = 5,
    [T_SCHOOL + 4]              = 5,
    [T_SCHOOL + 5]              = 5,

    [T_HIGH_SCHOOL + 0]         = 10, // Cost
    [T_HIGH_SCHOOL + 1]         = 10,
    [T_HIGH_SCHOOL + 2]         = 10,
    [T_HIGH_SCHOOL + 3]         = 10,
    [T_HIGH_SCHOOL + 4]         = 10,
    [T_HIGH_SCHOOL + 5]         = 10,
    [T_HIGH_SCHOOL + 6]         = 10,
    [T_HIGH_SCHOOL + 7]         = 10,
    [T_HIGH_SCHOOL + 8]         = 10,

    [T_UNIVERSITY + 0]          = 20, // Cost
    [T_UNIVERSITY + 1]          = 20,
    [T_UNIVERSITY + 2]          = 20,
    [T_UNIVERSITY + 3]          = 20,
    [T_UNIVERSITY + 4]          = 20,
    [T_UNIVERSITY + 5]          = 20,
    [T_UNIVERSITY + 6]          = 20,
    [T_UNIVERSITY + 7]          = 20,
    [T_UNIVERSITY + 8]          = 20,
    [T_UNIVERSITY + 9]          = 20,
    [T_UNIVERSITY + 10]         = 20,
    [T_UNIVERSITY + 11]         = 20,
    [T_UNIVERSITY + 12]         = 20,
    [T_UNIVERSITY + 13]         = 20,
    [T_UNIVERSITY + 14]         = 20,
    [T_UNIVERSITY + 15]         = 20,
    [T_UNIVERSITY + 16]         = 20,
    [T_UNIVERSITY + 17]         = 20,
    [T_UNIVERSITY + 18]         = 20,
    [T_UNIVERSITY + 19]         = 20,
    [T_UNIVERSITY + 20]         = 20,
    [T_UNIVERSITY + 21]         = 20,
    [T_UNIVERSITY + 22]         = 20,
    [T_UNIVERSITY + 23]         = 20,
    [T_UNIVERSITY + 24]         = 20,

    [T_MUSEUM + 0]              = 7, // Cost
    [T_MUSEUM + 1]              = 7,
    [T_MUSEUM + 2]              = 7,
    [T_MUSEUM + 3]              = 7,
    [T_MUSEUM + 4]              = 7,
    [T_MUSEUM + 5]              = 7,
    [T_MUSEUM + 6]              = 7,
    [T_MUSEUM + 7]              = 7,
    [T_MUSEUM + 8]              = 7,
    [T_MUSEUM + 9]              = 7,
    [T_MUSEUM + 10]             = 7,
    [T_MUSEUM + 11]             = 7,

    [T_LIBRARY + 0]             = 6, // Cost
    [T_LIBRARY + 1]             = 6,
    [T_LIBRARY + 2]             = 6,
    [T_LIBRARY + 3]             = 6,
    [T_LIBRARY + 4]             = 6,
    [T_LIBRARY + 5]             = 6,

    [T_AIRPORT + 0]             = 25, // Income
    [T_AIRPORT + 1]             = 25,
    [T_AIRPORT + 2]             = 25,
    [T_AIRPORT + 3]             = 25,
    [T_AIRPORT + 4]             = 25,
    [T_AIRPORT + 5]             = 25,
    [T_AIRPORT + 6]             = 25,
    [T_AIRPORT + 7]             = 25,
    [T_AIRPORT + 8]             = 25,
    [T_AIRPORT + 9]             = 25,
    [T_AIRPORT + 10]            = 25,
    [T_AIRPORT + 11]            = 25,
    [T_AIRPORT + 12]            = 25,
    [T_AIRPORT + 13]            = 25,
    [T_AIRPORT + 14]            = 25,

    [T_PORT + 0]                = 30, // Income
    [T_PORT + 1]                = 30,
    [T_PORT + 2]                = 30,
    [T_PORT + 3]                = 30,
    [T_PORT + 4]                = 30,
    [T_PORT + 5]                = 30,
    [T_PORT + 6]                = 30,
    [T_PORT + 7]                = 30,
    [T_PORT + 8]                = 30,

    [T_PORT_WATER_L]            = 0,
    [T_PORT_WATER_R]            = 0,
    [T_PORT_WATER_D]            = 0,
    [T_PORT_WATER_U]            = 0,

    [T_POWER_PLANT_COAL + 0]    = 0,
    [T_POWER_PLANT_COAL + 1]    = 0,
    [T_POWER_PLANT_COAL + 2]    = 0,
    [T_POWER_PLANT_COAL + 3]    = 0,
    [T_POWER_PLANT_COAL + 4]    = 0,
    [T_POWER_PLANT_COAL + 5]    = 0,
    [T_POWER_PLANT_COAL + 6]    = 0,
    [T_POWER_PLANT_COAL + 7]    = 0,
    [T_POWER_PLANT_COAL + 8]    = 0,
    [T_POWER_PLANT_COAL + 9]    = 0,
    [T_POWER_PLANT_COAL + 10]   = 0,
    [T_POWER_PLANT_COAL + 11]   = 0,
    [T_POWER_PLANT_COAL + 12]   = 0,
    [T_POWER_PLANT_COAL + 13]   = 0,
    [T_POWER_PLANT_COAL + 14]   = 0,
    [T_POWER_PLANT_COAL + 15]   = 0,

    [T_POWER_PLANT_OIL + 0]     = 0,
    [T_POWER_PLANT_OIL + 1]     = 0,
    [T_POWER_PLANT_OIL + 2]     = 0,
    [T_POWER_PLANT_OIL + 3]     = 0,
    [T_POWER_PLANT_OIL + 4]     = 0,
    [T_POWER_PLANT_OIL + 5]     = 0,
    [T_POWER_PLANT_OIL + 6]     = 0,
    [T_POWER_PLANT_OIL + 7]     = 0,
    [T_POWER_PLANT_OIL + 8]     = 0,
    [T_POWER_PLANT_OIL + 9]     = 0,
    [T_POWER_PLANT_OIL + 10]    = 0,
    [T_POWER_PLANT_OIL + 11]    = 0,
    [T_POWER_PLANT_OIL + 12]    = 0,
    [T_POWER_PLANT_OIL + 13]    = 0,
    [T_POWER_PLANT_OIL + 14]    = 0,
    [T_POWER_PLANT_OIL + 15]    = 0,

    [T_POWER_PLANT_WIND + 0]    = 0,
    [T_POWER_PLANT_WIND + 1]    = 0,
    [T_POWER_PLANT_WIND + 2]    = 0,
    [T_POWER_PLANT_WIND + 3]    = 0,

    [T_POWER_PLANT_SOLAR + 0]   = 0,
    [T_POWER_PLANT_SOLAR + 1]   = 0,
    [T_POWER_PLANT_SOLAR + 2]   = 0,
    [T_POWER_PLANT_SOLAR + 3]   = 0,
    [T_POWER_PLANT_SOLAR + 4]   = 0,
    [T_POWER_PLANT_SOLAR + 5]   = 0,
    [T_POWER_PLANT_SOLAR + 6]   = 0,
    [T_POWER_PLANT_SOLAR + 7]   = 0,
    [T_POWER_PLANT_SOLAR + 8]   = 0,
    [T_POWER_PLANT_SOLAR + 9]   = 0,
    [T_POWER_PLANT_SOLAR + 10]  = 0,
    [T_POWER_PLANT_SOLAR + 11]  = 0,
    [T_POWER_PLANT_SOLAR + 12]  = 0,
    [T_POWER_PLANT_SOLAR + 13]  = 0,
    [T_POWER_PLANT_SOLAR + 14]  = 0,
    [T_POWER_PLANT_SOLAR + 15]  = 0,

    [T_POWER_PLANT_NUCLEAR + 0] = 0,
    [T_POWER_PLANT_NUCLEAR + 1] = 0,
    [T_POWER_PLANT_NUCLEAR + 2] = 0,
    [T_POWER_PLANT_NUCLEAR + 3] = 0,
    [T_POWER_PLANT_NUCLEAR + 4] = 0,
    [T_POWER_PLANT_NUCLEAR + 5] = 0,
    [T_POWER_PLANT_NUCLEAR + 6] = 0,
    [T_POWER_PLANT_NUCLEAR + 7] = 0,
    [T_POWER_PLANT_NUCLEAR + 8] = 0,
    [T_POWER_PLANT_NUCLEAR + 9] = 0,
    [T_POWER_PLANT_NUCLEAR + 10] = 0,
    [T_POWER_PLANT_NUCLEAR + 11] = 0,
    [T_POWER_PLANT_NUCLEAR + 12] = 0,
    [T_POWER_PLANT_NUCLEAR + 13] = 0,
    [T_POWER_PLANT_NUCLEAR + 14] = 0,
    [T_POWER_PLANT_NUCLEAR + 15] = 0,

    [T_POWER_PLANT_FUSION + 0]  = 0,
    [T_POWER_PLANT_FUSION + 1]  = 0,
    [T_POWER_PLANT_FUSION + 2]  = 0,
    [T_POWER_PLANT_FUSION + 3]  = 0,
    [T_POWER_PLANT_FUSION + 4]  = 0,
    [T_POWER_PLANT_FUSION + 5]  = 0,
    [T_POWER_PLANT_FUSION + 6]  = 0,
    [T_POWER_PLANT_FUSION + 7]  = 0,
    [T_POWER_PLANT_FUSION + 8]  = 0,
    [T_POWER_PLANT_FUSION + 9]  = 0,
    [T_POWER_PLANT_FUSION + 10]  = 0,
    [T_POWER_PLANT_FUSION + 11]  = 0,
    [T_POWER_PLANT_FUSION + 12]  = 0,
    [T_POWER_PLANT_FUSION + 13]  = 0,
    [T_POWER_PLANT_FUSION + 14]  = 0,
    [T_POWER_PLANT_FUSION + 15]  = 0,

    [T_RESIDENTIAL_S1_A]        = 6, // Income
    [T_RESIDENTIAL_S1_B]        = 7,
    [T_RESIDENTIAL_S1_C]        = 8,
    [T_RESIDENTIAL_S1_D]        = 8,

    [T_RESIDENTIAL_S2_A + 0]    = 10,
    [T_RESIDENTIAL_S2_A + 1]    = 10,
    [T_RESIDENTIAL_S2_A + 2]    = 10,
    [T_RESIDENTIAL_S2_A + 3]    = 10,

    [T_RESIDENTIAL_S2_B + 0]    = 12,
    [T_RESIDENTIAL_S2_B + 1]    = 12,
    [T_RESIDENTIAL_S2_B + 2]    = 12,
    [T_RESIDENTIAL_S2_B + 3]    = 12,

    [T_RESIDENTIAL_S2_C + 0]    = 13,
    [T_RESIDENTIAL_S2_C + 1]    = 13,
    [T_RESIDENTIAL_S2_C + 2]    = 13,
    [T_RESIDENTIAL_S2_C + 3]    = 13,

    [T_RESIDENTIAL_S2_D + 0]    = 15,
    [T_RESIDENTIAL_S2_D + 1]    = 15,
    [T_RESIDENTIAL_S2_D + 2]    = 15,
    [T_RESIDENTIAL_S2_D + 3]    = 15,

    [T_RESIDENTIAL_S3_A + 0]    = 20,
    [T_RESIDENTIAL_S3_A + 1]    = 20,
    [T_RESIDENTIAL_S3_A + 2]    = 20,
    [T_RESIDENTIAL_S3_A + 3]    = 20,
    [T_RESIDENTIAL_S3_A + 4]    = 20,
    [T_RESIDENTIAL_S3_A + 5]    = 20,
    [T_RESIDENTIAL_S3_A + 6]    = 20,
    [T_RESIDENTIAL_S3_A + 7]    = 20,
    [T_RESIDENTIAL_S3_A + 8]    = 20,

    [T_RESIDENTIAL_S3_B + 0]    = 21,
    [T_RESIDENTIAL_S3_B + 1]    = 21,
    [T_RESIDENTIAL_S3_B + 2]    = 21,
    [T_RESIDENTIAL_S3_B + 3]    = 21,
    [T_RESIDENTIAL_S3_B + 4]    = 21,
    [T_RESIDENTIAL_S3_B + 5]    = 21,
    [T_RESIDENTIAL_S3_B + 6]    = 21,
    [T_RESIDENTIAL_S3_B + 7]    = 21,
    [T_RESIDENTIAL_S3_B + 8]    = 21,

    [T_RESIDENTIAL_S3_C + 0]    = 22,
    [T_RESIDENTIAL_S3_C + 1]    = 22,
    [T_RESIDENTIAL_S3_C + 2]    = 22,
    [T_RESIDENTIAL_S3_C + 3]    = 22,
    [T_RESIDENTIAL_S3_C + 4]    = 22,
    [T_RESIDENTIAL_S3_C + 5]    = 22,
    [T_RESIDENTIAL_S3_C + 6]    = 22,
    [T_RESIDENTIAL_S3_C + 7]    = 22,
    [T_RESIDENTIAL_S3_C + 8]    = 22,

    [T_RESIDENTIAL_S3_D + 0]    = 24,
    [T_RESIDENTIAL_S3_D + 1]    = 24,
    [T_RESIDENTIAL_S3_D + 2]    = 24,
    [T_RESIDENTIAL_S3_D + 3]    = 24,
    [T_RESIDENTIAL_S3_D + 4]    = 24,
    [T_RESIDENTIAL_S3_D + 5]    = 24,
    [T_RESIDENTIAL_S3_D + 6]    = 24,
    [T_RESIDENTIAL_S3_D + 7]    = 24,
    [T_RESIDENTIAL_S3_D + 8]    = 24,

    [T_COMMERCIAL_S1_A]         = 8,
    [T_COMMERCIAL_S1_B]         = 8,
    [T_COMMERCIAL_S1_C]         = 9,
    [T_COMMERCIAL_S1_D]         = 10,

    [T_COMMERCIAL_S2_A + 0]     = 10,
    [T_COMMERCIAL_S2_A + 1]     = 10,
    [T_COMMERCIAL_S2_A + 2]     = 10,
    [T_COMMERCIAL_S2_A + 3]     = 10,

    [T_COMMERCIAL_S2_B + 0]     = 12,
    [T_COMMERCIAL_S2_B + 1]     = 12,
    [T_COMMERCIAL_S2_B + 2]     = 12,
    [T_COMMERCIAL_S2_B + 3]     = 12,

    [T_COMMERCIAL_S2_C + 0]     = 14,
    [T_COMMERCIAL_S2_C + 1]     = 14,
    [T_COMMERCIAL_S2_C + 2]     = 14,
    [T_COMMERCIAL_S2_C + 3]     = 14,

    [T_COMMERCIAL_S2_D + 0]     = 16,
    [T_COMMERCIAL_S2_D + 1]     = 16,
    [T_COMMERCIAL_S2_D + 2]     = 16,
    [T_COMMERCIAL_S2_D + 3]     = 16,

    [T_COMMERCIAL_S3_A + 0]     = 23,
    [T_COMMERCIAL_S3_A + 1]     = 23,
    [T_COMMERCIAL_S3_A + 2]     = 23,
    [T_COMMERCIAL_S3_A + 3]     = 23,
    [T_COMMERCIAL_S3_A + 4]     = 23,
    [T_COMMERCIAL_S3_A + 5]     = 23,
    [T_COMMERCIAL_S3_A + 6]     = 23,
    [T_COMMERCIAL_S3_A + 7]     = 23,
    [T_COMMERCIAL_S3_A + 8]     = 23,

    [T_COMMERCIAL_S3_B + 0]     = 24,
    [T_COMMERCIAL_S3_B + 1]     = 24,
    [T_COMMERCIAL_S3_B + 2]     = 24,
    [T_COMMERCIAL_S3_B + 3]     = 24,
    [T_COMMERCIAL_S3_B + 4]     = 24,
    [T_COMMERCIAL_S3_B + 5]     = 24,
    [T_COMMERCIAL_S3_B + 6]     = 24,
    [T_COMMERCIAL_S3_B + 7]     = 24,
    [T_COMMERCIAL_S3_B + 8]     = 24,

    [T_COMMERCIAL_S3_C + 0]     = 25,
    [T_COMMERCIAL_S3_C + 1]     = 25,
    [T_COMMERCIAL_S3_C + 2]     = 25,
    [T_COMMERCIAL_S3_C + 3]     = 25,
    [T_COMMERCIAL_S3_C + 4]     = 25,
    [T_COMMERCIAL_S3_C + 5]     = 25,
    [T_COMMERCIAL_S3_C + 6]     = 25,
    [T_COMMERCIAL_S3_C + 7]     = 25,
    [T_COMMERCIAL_S3_C + 8]     = 25,

    [T_COMMERCIAL_S3_D + 0]     = 27,
    [T_COMMERCIAL_S3_D + 1]     = 27,
    [T_COMMERCIAL_S3_D + 2]     = 27,
    [T_COMMERCIAL_S3_D + 3]     = 27,
    [T_COMMERCIAL_S3_D + 4]     = 27,
    [T_COMMERCIAL_S3_D + 5]     = 27,
    [T_COMMERCIAL_S3_D + 6]     = 27,
    [T_COMMERCIAL_S3_D + 7]     = 27,
    [T_COMMERCIAL_S3_D + 8]     = 27,

    [T_INDUSTRIAL_S1_A]         = 9,
    [T_INDUSTRIAL_S1_B]         = 9,
    [T_INDUSTRIAL_S1_C]         = 10,
    [T_INDUSTRIAL_S1_D]         = 11,

    [T_INDUSTRIAL_S2_A + 0]     = 14,
    [T_INDUSTRIAL_S2_A + 1]     = 14,
    [T_INDUSTRIAL_S2_A + 2]     = 14,
    [T_INDUSTRIAL_S2_A + 3]     = 14,

    [T_INDUSTRIAL_S2_B + 0]     = 15,
    [T_INDUSTRIAL_S2_B + 1]     = 15,
    [T_INDUSTRIAL_S2_B + 2]     = 15,
    [T_INDUSTRIAL_S2_B + 3]     = 15,

    [T_INDUSTRIAL_S2_C + 0]     = 17,
    [T_INDUSTRIAL_S2_C + 1]     = 17,
    [T_INDUSTRIAL_S2_C + 2]     = 17,
    [T_INDUSTRIAL_S2_C + 3]     = 17,

    [T_INDUSTRIAL_S2_D + 0]     = 18,
    [T_INDUSTRIAL_S2_D + 1]     = 18,
    [T_INDUSTRIAL_S2_D + 2]     = 18,
    [T_INDUSTRIAL_S2_D + 3]     = 18,

    [T_INDUSTRIAL_S3_A + 0]     = 24,
    [T_INDUSTRIAL_S3_A + 1]     = 24,
    [T_INDUSTRIAL_S3_A + 2]     = 24,
    [T_INDUSTRIAL_S3_A + 3]     = 24,
    [T_INDUSTRIAL_S3_A + 4]     = 24,
    [T_INDUSTRIAL_S3_A + 5]     = 24,
    [T_INDUSTRIAL_S3_A + 6]     = 24,
    [T_INDUSTRIAL_S3_A + 7]     = 24,
    [T_INDUSTRIAL_S3_A + 8]     = 24,

    [T_INDUSTRIAL_S3_B + 0]     = 26,
    [T_INDUSTRIAL_S3_B + 1]     = 26,
    [T_INDUSTRIAL_S3_B + 2]     = 26,
    [T_INDUSTRIAL_S3_B + 3]     = 26,
    [T_INDUSTRIAL_S3_B + 4]     = 26,
    [T_INDUSTRIAL_S3_B + 5]     = 26,
    [T_INDUSTRIAL_S3_B + 6]     = 26,
    [T_INDUSTRIAL_S3_B + 7]     = 26,
    [T_INDUSTRIAL_S3_B + 8]     = 26,

    [T_INDUSTRIAL_S3_C + 0]     = 27,
    [T_INDUSTRIAL_S3_C + 1]     = 27,
    [T_INDUSTRIAL_S3_C + 2]     = 27,
    [T_INDUSTRIAL_S3_C + 3]     = 27,
    [T_INDUSTRIAL_S3_C + 4]     = 27,
    [T_INDUSTRIAL_S3_C + 5]     = 27,
    [T_INDUSTRIAL_S3_C + 6]     = 27,
    [T_INDUSTRIAL_S3_C + 7]     = 27,
    [T_INDUSTRIAL_S3_C + 8]     = 27,

    [T_INDUSTRIAL_S3_D + 0]     = 30,
    [T_INDUSTRIAL_S3_D + 1]     = 30,
    [T_INDUSTRIAL_S3_D + 2]     = 30,
    [T_INDUSTRIAL_S3_D + 3]     = 30,
    [T_INDUSTRIAL_S3_D + 4]     = 30,
    [T_INDUSTRIAL_S3_D + 5]     = 30,
    [T_INDUSTRIAL_S3_D + 6]     = 30,
    [T_INDUSTRIAL_S3_D + 7]     = 30,
    [T_INDUSTRIAL_S3_D + 8]     = 30,

    [T_FIRE_1]                  = 0,
    [T_FIRE_2]                  = 0,

    [T_RADIATION_GROUND]        = 0,
    [T_RADIATION_WATER]         = 0,
};

static int32_t *tile_money_destination[] = {
    [TYPE_FIELD] = &budget_transport,   // Roads, train tracks, power lines
    [TYPE_FOREST] = &taxes_other,       // Placeholder, unused
    [TYPE_WATER] = &budget_transport,   // Bridges
    [TYPE_RESIDENTIAL] = &taxes_rci,
    [TYPE_INDUSTRIAL] = &taxes_rci,
    [TYPE_COMMERCIAL] = &taxes_rci,
    [TYPE_POLICE_DEPT] = &budget_police,
    [TYPE_FIRE_DEPT] = &budget_firemen,
    [TYPE_HOSPITAL] = &budget_healthcare,
    [TYPE_PARK] = &budget_healthcare,
    [TYPE_STADIUM] = &taxes_other,
    [TYPE_SCHOOL] = &budget_education,
    [TYPE_HIGH_SCHOOL] = &budget_education,
    [TYPE_UNIVERSITY] = &budget_education,
    [TYPE_MUSEUM] = &budget_education,
    [TYPE_LIBRARY] = &budget_education,
    [TYPE_AIRPORT] = &taxes_other,
    [TYPE_PORT] = &taxes_other,
    [TYPE_DOCK] = &taxes_other,
    [TYPE_POWER_PLANT] = &taxes_other,      // Placeholder, unused
    [TYPE_FIRE] = &taxes_other, // Placeholder, unused
    [TYPE_RADIATION] = &taxes_other, // Placeholder, unused
};

void Simulation_CalculateBudgetAndTaxes(void)
{
    // Clear variables
    // ---------------

    taxes_rci = 0;
    taxes_other = 0;
    budget_police = 0;
    budget_firemen = 0;
    budget_healthcare = 0;
    budget_education = 0;
    budget_transport = 0;

    // Calculate taxes and budget
    // --------------------------

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t tile, type;
            CityMapGetTypeAndTileUnsafe(i, j, &tile, &type);

            type &= TYPE_MASK;

            int32_t cost = city_tile_money_cost[tile];
            int32_t *dst_ptr = tile_money_destination[type];

            *dst_ptr += cost;
        }
    }

    // Adjust taxes according to the tax percentage
    // --------------------------------------------

    // 10% = base cost => Final cost = base cost * tax / 10

    taxes_rci = (taxes_rci * tax_percentage) / 20;
    taxes_other = (taxes_other * tax_percentage) / 20;

    // Calculate total budget
    // ----------------------

    budget_result =
        // Add RCI and other taxes
        + taxes_rci
        + taxes_other
        // Pay police, firemen, healthcare, education and transport
        - budget_police
        - budget_firemen
        - budget_healthcare
        - budget_education
        - budget_transport;


    // Pay loans

    // TODO
    //if (loan_remaining_payments > 0)
    //    budget_result -= loan_payments_amount;
}


void Simulation_ApplyBudgetAndTaxes(void)
{
    // Add temp variable to original amount of money

    MoneyAdd(budget_result);

    // Reduce number of remaining loan payments

    // TODO
    //if (loan_remaining_payments > 0)
    //{
    //    loan_remaining_payments--;
    //    if (loan_remaining_payments == 0)
    //    {
    //        MessageQueueAdd(ID_MSG_FINISHED_LOAN);
    //    }
    //}

    // Check if money is negative to warn the user

    if (MoneyGet() > 0)
    {
        negative_budget_count = 0;
    }
    else
    {
        if (budget_result > 0)
        {
            negative_budget_count = 0;
        }
        else
        {
            negative_budget_count++;

            if (negative_budget_count >= 4)
            {
                MessageQueueAdd(ID_MSG_GAME_OVER_1);
                MessageQueueAdd(ID_MSG_GAME_OVER_2);
                // TODO: game_over = 1;

                return;
            }
            else
            {
#if 0
                // TODO: Enable this
                if (loan_remaining_payments > 0)
                {
                    PersistentMessageShow(ID_MSG_MONEY_NEGATIVE_CANT_LOAN);
                }
                else
                {
                    PersistentMessageShow(ID_MSG_MONEY_NEGATIVE_CAN_LOAN);
                }
#endif
            }
        }
    }
}
