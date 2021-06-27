// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef SIMULATION_POLLUTION_H__
#define SIMULATION_POLLUTION_H__

#include <stdint.h>

uint8_t *Simulation_PollutionGetMap(void);

int Simulation_PollutionGetTotal(void);
int Simulation_PollutionGetPercentage(void);

void Simulation_Pollution(void);

#endif // SIMULATION_POLLUTION_H__
