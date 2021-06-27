// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef SIMULATION_TRAFFIC_H__
#define SIMULATION_TRAFFIC_H__

#include <stdint.h>

uint8_t *Simulation_TrafficGetMap(void);
int Simulation_TrafficGetTrafficJamPercent(void);

void Simulation_Traffic(void);

void Simulation_TrafficRemoveAnimationTiles(void);
void Simulation_TrafficAnimate(void);

#endif // SIMULATION_TRAFFIC_H__
