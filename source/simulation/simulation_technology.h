// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef SIMULATION_SIMULATION_TECHNOLOGY_H__
#define SIMULATION_SIMULATION_TECHNOLOGY_H__

#define TECH_LEVEL_NUCLEAR      10
#define TECH_LEVEL_FUSION       40

#define TECH_LEVEL_MAX          40

void Technology_SetLevel(int level);
int Technology_GetLevel(void);

int Technology_IsBuildingAvailable(int building_type);

void Simulation_AdvanceTechnology(void);

#endif // SIMULATION_SIMULATION_TECHNOLOGY_H__
