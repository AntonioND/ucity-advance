// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2017-2019, 2021, Antonio Niño Díaz

#ifndef ROOM_GAME_BUILDING_INFO_H__
#define ROOM_GAME_BUILDING_INFO_H__

typedef struct {
    int8_t width;
    int8_t height;
    uint16_t base_tile; // Base tile of tileset_info.h
    uint32_t price;
} building_info;

// The argument is one of the B_*** defines
building_info *Get_Building_Info(int building_type);

building_info *Get_BuildingFromBaseTile(uint16_t tile);

// Index of every building
#define B_Residential       0
#define B_Commercial        1
#define B_Industrial        2

#define B_PoliceDept        3
#define B_FireDept          4
#define B_Hospital          5

#define B_ParkSmall         6
#define B_ParkBig           7
#define B_Stadium           8

#define B_School            9
#define B_HighSchool        10
#define B_University        11

#define B_Museum            12
#define B_Library           13

#define B_TrainStation      14
#define B_Airport           15

#define B_PowerPlantCoal    16
#define B_PowerPlantOil     17
#define B_PowerPlantWind    18
#define B_PowerPlantSolar   19
#define B_PowerPlantNuclear 20
#define B_PowerPlantFusion  21

// Number of buildings. Everything past this are special "meta buildings"
#define B_BuildingMax       22

// Size is 1x1, which is useful, but it doesn't exist.
#define B_None              22

#define B_Road              23
#define B_Train             24
#define B_PowerLines        25
#define B_Port              26

#define B_MetabuildingMax   26

#define B_ResidentialS1A    30
#define B_ResidentialS1B    31
#define B_ResidentialS1C    32
#define B_ResidentialS1D    33
#define B_ResidentialS2A    34
#define B_ResidentialS2B    35
#define B_ResidentialS2C    36
#define B_ResidentialS2D    37
#define B_ResidentialS3A    38
#define B_ResidentialS3B    39
#define B_ResidentialS3C    40
#define B_ResidentialS3D    41

#define B_CommercialS1A     42
#define B_CommercialS1B     43
#define B_CommercialS1C     44
#define B_CommercialS1D     45
#define B_CommercialS2A     46
#define B_CommercialS2B     47
#define B_CommercialS2C     48
#define B_CommercialS2D     49
#define B_CommercialS3A     50
#define B_CommercialS3B     51
#define B_CommercialS3C     52
#define B_CommercialS3D     53

#define B_IndustrialS1A     54
#define B_IndustrialS1B     55
#define B_IndustrialS1C     56
#define B_IndustrialS1D     57
#define B_IndustrialS2A     58
#define B_IndustrialS2B     59
#define B_IndustrialS2C     60
#define B_IndustrialS2D     61
#define B_IndustrialS3A     62
#define B_IndustrialS3B     63
#define B_IndustrialS3C     64
#define B_IndustrialS3D     65

#define B_RadiationGround   66
#define B_RadiationWater    67

#define B_LastBuilding      67

#define B_Delete            255 // Special type

#endif // ROOM_GAME_BUILDING_INFO_H__
