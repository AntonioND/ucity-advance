// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef SIMULATION_FIRE_H__
#define SIMULATION_FIRE_H__

void MapDeleteBuildingFire(int x, int y);

void Simulation_FireTryStart(int force);

void Simulation_Fire(void);

void Simulation_FireAnimate(void);

#endif // SIMULATION_FIRE_H__
