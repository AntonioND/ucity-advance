// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef SIMULATION_SIMULATION_CALCULATE_STATS_H__
#define SIMULATION_SIMULATION_CALCULATE_STATS_H__

int Simulation_GetCityClass(void);

uint32_t Simulation_GetTotalPopulation(void);

void Simulation_GetDemandRCI(int *r, int *c, int *i);

void Simulation_CalculateRCIDemand(void);
void Simulation_CalculateStatistics(void);

int CityStats_IsBuildingAvailable(int building_type);

#endif // SIMULATION_SIMULATION_CALCULATE_STATS_H__
