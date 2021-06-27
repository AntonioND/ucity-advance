// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef SIMULATION_HAPPINESS_H__
#define SIMULATION_HAPPINESS_H__

#include <stdint.h>

uint8_t *Simulation_HappinessGetMap(void);

uint8_t Simulation_HappinessGetFlags(int x, int y);

void Simulation_HappinessSetFlags(int x, int y, uint8_t flags);
void Simulation_HappinessResetFlags(int x, int y, uint8_t flags);

void Simulation_HappinessResetMap(void);

#endif // SIMULATION_HAPPINESS_H__
