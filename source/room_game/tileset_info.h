// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2017-2019, 2021, Antonio Niño Díaz

#ifndef ROOM_GAME_TILESET_INFO_H__
#define ROOM_GAME_TILESET_INFO_H__

#include <stdint.h>

// ----------------------------------------------------------------------------

// This takes a define from the list in "tileset_info.h" and returns the
// combined tile number and palette to be written to VRAM in the background.
uint16_t City_Tileset_VRAM_Info(uint16_t city_index);

// ----------------------------------------------------------------------------

typedef struct {
    uint16_t element_type;

    // Used to identify the origin of coordinates of a building if it is
    // composed by more than one tile.
    int8_t base_x_delta;
    int8_t base_y_delta;

    char *name;
} city_tile_info;

const city_tile_info *City_Tileset_Entry_Info(uint16_t city_index);

void CityMapDrawTerrainTile(uint16_t city_tile, void *map, int x, int y);

int BuildingIsCoordinateOrigin(uint16_t tile);

void BuildingGetCoordinateOrigin(uint16_t tile, int x, int y, int *ox, int *oy);

// ----------------------------------------------------------------------------

// List of tiles
// -------------

// Note: All bridge tiles should be < 256

// Grass surrounded by forest
#define T_GRASS__FOREST_TL          0
#define T_GRASS__FOREST_TC          1
#define T_GRASS__FOREST_TR          2
#define T_GRASS__FOREST_CL          3
#define T_GRASS                     4
#define T_GRASS__FOREST_CR          5
#define T_GRASS__FOREST_BL          6
#define T_GRASS__FOREST_BC          7
#define T_GRASS__FOREST_BR          8
#define T_GRASS__FOREST_CORNER_TL   9
#define T_GRASS__FOREST_CORNER_TR   10
#define T_GRASS__FOREST_CORNER_BL   11
#define T_GRASS__FOREST_CORNER_BR   12
#define T_FOREST                    13
#define T_GRASS_EXTRA               14
#define T_FOREST_EXTRA              15

// Water surrounded by grass
#define T_WATER__GRASS_TL           16
#define T_WATER__GRASS_TC           17
#define T_WATER__GRASS_TR           18
#define T_WATER__GRASS_CL           19
#define T_WATER                     20
#define T_WATER__GRASS_CR           21
#define T_WATER__GRASS_BL           22
#define T_WATER__GRASS_BC           23
#define T_WATER__GRASS_BR           24
#define T_WATER__GRASS_CORNER_TL    25
#define T_WATER__GRASS_CORNER_TR    26
#define T_WATER__GRASS_CORNER_BL    27
#define T_WATER__GRASS_CORNER_BR    28
#define T_WATER_EXTRA               29

// Terrains
#define T_RESIDENTIAL               30 // Industrial must be the highest of the
#define T_COMMERCIAL                31 // 3 and should be placed before all the
#define T_INDUSTRIAL                32 // RCI buildings. Also, the 3 of them
#define T_DEMOLISHED                33 // must be lower than 256.

// Road
#define T_ROAD_TB                   34 // Joins top and bottom (vertical)
#define T_ROAD_TB_1                 35
#define T_ROAD_TB_2                 36
#define T_ROAD_TB_3                 37
#define T_ROAD_LR                   38 // Joins left and right (horizontal)
#define T_ROAD_LR_1                 39
#define T_ROAD_LR_2                 40
#define T_ROAD_LR_3                 41
#define T_ROAD_RB                   42
#define T_ROAD_LB                   43
#define T_ROAD_TR                   44
#define T_ROAD_TL                   45
#define T_ROAD_TRB                  46
#define T_ROAD_LRB                  47
#define T_ROAD_TLB                  48
#define T_ROAD_TLR                  49
#define T_ROAD_TLRB                 50 // 2 roads crossing
#define T_ROAD_TB_POWER_LINES       51 // Road + power lines
#define T_ROAD_LR_POWER_LINES       52
#define T_ROAD_TB_BRIDGE            53 // Bridge
#define T_ROAD_LR_BRIDGE            54

// Train
#define T_TRAIN_TB                  55 // Joins top and bottom (vertical)
#define T_TRAIN_LR                  56 // Joins left and right (horizontal)
#define T_TRAIN_RB                  57
#define T_TRAIN_LB                  58
#define T_TRAIN_TR                  59
#define T_TRAIN_TL                  60
#define T_TRAIN_TRB                 61
#define T_TRAIN_LRB                 62
#define T_TRAIN_TLB                 63
#define T_TRAIN_TLR                 64
#define T_TRAIN_TLRB                65 // Crossing
#define T_TRAIN_LR_ROAD             66 // Train + road
#define T_TRAIN_TB_ROAD             67
#define T_TRAIN_TB_POWER_LINES      68 // Train + power lines
#define T_TRAIN_LR_POWER_LINES      69
#define T_TRAIN_TB_BRIDGE           70 // Bridge
#define T_TRAIN_LR_BRIDGE           71

// Power lines
#define T_POWER_LINES_TB            72 // Joins top and bottom (vertical)
#define T_POWER_LINES_LR            73 // Joins left and right (horizontal)
#define T_POWER_LINES_RB            74
#define T_POWER_LINES_LB            75
#define T_POWER_LINES_TR            76
#define T_POWER_LINES_TL            77
#define T_POWER_LINES_TRB           78
#define T_POWER_LINES_LRB           79
#define T_POWER_LINES_TLB           80
#define T_POWER_LINES_TLR           81
#define T_POWER_LINES_TLRB          82 // 2 lines crossing
#define T_POWER_LINES_TB_BRIDGE     83 // Bridge
#define T_POWER_LINES_LR_BRIDGE     84

// Buildings: Services
#define T_POLICE_DEPT               85
#define T_POLICE_DEPT_CENTER        (T_POLICE_DEPT + 4) // 4=3+1 (3x3 building)
#define T_FIRE_DEPT                 94
#define T_FIRE_DEPT_CENTER          (T_FIRE_DEPT + 4)
#define T_HOSPITAL                  103
#define T_HOSPITAL_CENTER           (T_HOSPITAL + 4)

#define T_PARK_SMALL                112
#define T_PARK_BIG                  113
#define T_STADIUM                   122

#define T_SCHOOL                    142
#define T_SCHOOL_CENTER             (T_SCHOOL + 4) // 4=3+1 (3x2 building)
#define T_HIGH_SCHOOL               148
#define T_HIGH_SCHOOL_CENTER        (T_HIGH_SCHOOL + 4) // 4=3+1 (3x3 building)
#define T_UNIVERSITY                157
#define T_MUSEUM                    182
#define T_LIBRARY                   194

#define T_AIRPORT                   200
#define T_AIRPORT_RUNWAY            (T_AIRPORT + 10) // 10=5+5+0 (5x3 building)
#define T_PORT                      215
#define T_PORT_WATER_L              224 // Make sure that the docks are all in
#define T_PORT_WATER_R              225 // the same 256 tile bank.
#define T_PORT_WATER_D              226
#define T_PORT_WATER_U              227

// Power plants
#define T_POWER_PLANT_COAL          228
#define T_POWER_PLANT_OIL           244
#define T_POWER_PLANT_WIND          260
#define T_POWER_PLANT_SOLAR         264
#define T_POWER_PLANT_NUCLEAR       280 // 5=4+1 (4x4 building)
#define T_POWER_PLANT_NUCLEAR_CENTER (T_POWER_PLANT_NUCLEAR + 5)
#define T_POWER_PLANT_FUSION        296 // This one should be right after the
                                        // nuclear power plant.

// Residential
#define T_RESIDENTIAL_S1_A          312
#define T_RESIDENTIAL_S1_B          313
#define T_RESIDENTIAL_S1_C          314
#define T_RESIDENTIAL_S1_D          315

#define T_RESIDENTIAL_S2_A          316
#define T_RESIDENTIAL_S2_B          320
#define T_RESIDENTIAL_S2_C          324
#define T_RESIDENTIAL_S2_D          328

#define T_RESIDENTIAL_S3_A          332
#define T_RESIDENTIAL_S3_B          341
#define T_RESIDENTIAL_S3_C          350
#define T_RESIDENTIAL_S3_D          359

// Commercial
#define T_COMMERCIAL_S1_A           368
#define T_COMMERCIAL_S1_B           369
#define T_COMMERCIAL_S1_C           370
#define T_COMMERCIAL_S1_D           371

#define T_COMMERCIAL_S2_A           372
#define T_COMMERCIAL_S2_B           376
#define T_COMMERCIAL_S2_C           380
#define T_COMMERCIAL_S2_D           384

#define T_COMMERCIAL_S3_A           388
#define T_COMMERCIAL_S3_B           397
#define T_COMMERCIAL_S3_C           406
#define T_COMMERCIAL_S3_D           415

// Industrial
#define T_INDUSTRIAL_S1_A           424
#define T_INDUSTRIAL_S1_B           425
#define T_INDUSTRIAL_S1_C           426
#define T_INDUSTRIAL_S1_D           427

#define T_INDUSTRIAL_S2_A           428
#define T_INDUSTRIAL_S2_B           432
#define T_INDUSTRIAL_S2_C           436
#define T_INDUSTRIAL_S2_D           440

#define T_INDUSTRIAL_S3_A           444
#define T_INDUSTRIAL_S3_B           453
#define T_INDUSTRIAL_S3_C           462
#define T_INDUSTRIAL_S3_D           471

#define T_FIRE_1                    480 // Make sure this one is even and
#define T_FIRE_2                    481 // this one is odd

#define T_RADIATION_GROUND          482
#define T_RADIATION_WATER           483

// TODO Unique buildings (only one per map) ?
#define T_CITY_HALL                 484
#define T_RESEARCH_CENTRE           500
#define T_LANDMARK                  510

#endif // ROOM_GAME_TILESET_INFO_H__
