// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2017-2019, 2021, Antonio Niño Díaz

#include <stdint.h>

#include <ugba/ugba.h>

#include "room_game/building_info.h"
#include "room_game/tileset_info.h"

static building_info buildings_info[] = { // TODO: BUILDING_INFO_STRUCTS_ARRAY
    // Dummy element. The only thing that matters here is the size.
    [B_None] = { 1, 1, T_DEMOLISHED, 0 }, // Demolished or something...

    [B_Residential] = { 1, 1, T_RESIDENTIAL, 10 },
    [B_Commercial] = { 1, 1, T_COMMERCIAL, 12 },
    [B_Industrial] = { 1, 1, T_INDUSTRIAL, 14 },

    [B_Road] = { 1, 1, T_ROAD_LR, 5 }, // Tile doesn't matter
    [B_Train] = { 1, 1, T_TRAIN_LR, 10 }, // Tile doesn't matter
    [B_PowerLines] = { 1, 1, T_POWER_LINES_LR, 2 }, // Tile doesn't matter

    // Modify the corresponding file in 'simulation' folder if changing this:
    [B_PoliceDept] = { 3, 3, T_POLICE_DEPT, 500 },
    [B_FireDept] = { 3, 3, T_FIRE_DEPT, 500 },
    [B_Hospital] = { 3, 3, T_HOSPITAL, 500 },

    [B_ParkSmall] = { 1, 1, T_PARK_SMALL, 10 },
    [B_ParkBig] = { 3, 3, T_PARK_BIG, 100 },
    [B_Stadium] = { 5, 4, T_STADIUM, 5000 },

    [B_School] = { 3, 2, T_SCHOOL, 100 },
    [B_HighSchool] = { 3, 3, T_HIGH_SCHOOL, 1000 },
    [B_University] = { 5, 5, T_UNIVERSITY, 7000 },

    [B_Museum] = { 4, 3, T_MUSEUM, 3000 },
    [B_Library] = { 3, 2, T_LIBRARY, 500 },

    [B_Airport] = { 5, 3, T_AIRPORT, 10000 },
    [B_Port] = { 3, 3, T_PORT, 1000 },

    [B_PowerPlantCoal] = { 4, 4, T_POWER_PLANT_COAL, 3000 },
    [B_PowerPlantOil] = { 4, 4, T_POWER_PLANT_OIL, 5000 },
    [B_PowerPlantWind] = { 2, 2, T_POWER_PLANT_WIND, 1000 },
    [B_PowerPlantSolar] = { 4, 4, T_POWER_PLANT_SOLAR, 5000 },
    [B_PowerPlantNuclear] = { 4, 4, T_POWER_PLANT_NUCLEAR, 10000 },
    [B_PowerPlantFusion] = { 4, 4, T_POWER_PLANT_FUSION, 20000 },

    [B_ResidentialS1A] = { 1, 1, T_RESIDENTIAL_S1_A, 0 },
    [B_ResidentialS1B] = { 1, 1, T_RESIDENTIAL_S1_B, 0 },
    [B_ResidentialS1C] = { 1, 1, T_RESIDENTIAL_S1_C, 0 },
    [B_ResidentialS1D] = { 1, 1, T_RESIDENTIAL_S1_D, 0 },
    [B_ResidentialS2A] = { 2, 2, T_RESIDENTIAL_S2_A, 0 },
    [B_ResidentialS2B] = { 2, 2, T_RESIDENTIAL_S2_B, 0 },
    [B_ResidentialS2C] = { 2, 2, T_RESIDENTIAL_S2_C, 0 },
    [B_ResidentialS2D] = { 2, 2, T_RESIDENTIAL_S2_D, 0 },
    [B_ResidentialS3A] = { 3, 3, T_RESIDENTIAL_S3_A, 0 },
    [B_ResidentialS3B] = { 3, 3, T_RESIDENTIAL_S3_B, 0 },
    [B_ResidentialS3C] = { 3, 3, T_RESIDENTIAL_S3_C, 0 },
    [B_ResidentialS3D] = { 3, 3, T_RESIDENTIAL_S3_D, 0 },

    [B_CommercialS1A] = { 1, 1, T_COMMERCIAL_S1_A, 0 },
    [B_CommercialS1B] = { 1, 1, T_COMMERCIAL_S1_B, 0 },
    [B_CommercialS1C] = { 1, 1, T_COMMERCIAL_S1_C, 0 },
    [B_CommercialS1D] = { 1, 1, T_COMMERCIAL_S1_D, 0 },
    [B_CommercialS2A] = { 2, 2, T_COMMERCIAL_S2_A, 0 },
    [B_CommercialS2B] = { 2, 2, T_COMMERCIAL_S2_B, 0 },
    [B_CommercialS2C] = { 2, 2, T_COMMERCIAL_S2_C, 0 },
    [B_CommercialS2D] = { 2, 2, T_COMMERCIAL_S2_D, 0 },
    [B_CommercialS3A] = { 3, 3, T_COMMERCIAL_S3_A, 0 },
    [B_CommercialS3B] = { 3, 3, T_COMMERCIAL_S3_B, 0 },
    [B_CommercialS3C] = { 3, 3, T_COMMERCIAL_S3_C, 0 },
    [B_CommercialS3D] = { 3, 3, T_COMMERCIAL_S3_D, 0 },

    [B_IndustrialS1A] = { 1, 1, T_INDUSTRIAL_S1_A, 0 },
    [B_IndustrialS1B] = { 1, 1, T_INDUSTRIAL_S1_B, 0 },
    [B_IndustrialS1C] = { 1, 1, T_INDUSTRIAL_S1_C, 0 },
    [B_IndustrialS1D] = { 1, 1, T_INDUSTRIAL_S1_D, 0 },
    [B_IndustrialS2A] = { 2, 2, T_INDUSTRIAL_S2_A, 0 },
    [B_IndustrialS2B] = { 2, 2, T_INDUSTRIAL_S2_B, 0 },
    [B_IndustrialS2C] = { 2, 2, T_INDUSTRIAL_S2_C, 0 },
    [B_IndustrialS2D] = { 2, 2, T_INDUSTRIAL_S2_D, 0 },
    [B_IndustrialS3A] = { 3, 3, T_INDUSTRIAL_S3_A, 0 },
    [B_IndustrialS3B] = { 3, 3, T_INDUSTRIAL_S3_B, 0 },
    [B_IndustrialS3C] = { 3, 3, T_INDUSTRIAL_S3_C, 0 },
    [B_IndustrialS3D] = { 3, 3, T_INDUSTRIAL_S3_D, 0 },

    [B_RadiationGround] = { 1, 1, T_RADIATION_GROUND, 0 },
    [B_RadiationWater] = { 1, 1, T_RADIATION_WATER, 0 },

    [B_Delete] = { 1, 1, T_DEMOLISHED, 5 },
};

building_info *Get_Building_Info(int building_type)
{
    UGBA_Assert((building_type <= B_LastBuilding) || (building_type == B_Delete));

    return &buildings_info[building_type];
}

building_info *Get_BuildingFromBaseTile(uint16_t tile)
{
    int elems = sizeof(buildings_info) / sizeof(buildings_info[0]);

    for (int i = 0; i < elems; i++)
    {
        building_info *info = &buildings_info[i];
        if (info->base_tile == tile)
            return info;
    }

    return &buildings_info[B_None];
}
