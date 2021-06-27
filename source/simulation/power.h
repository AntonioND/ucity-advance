// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef SIMULATION_POWER_H__
#define SIMULATION_POWER_H__

#include <stdint.h>

uint8_t *Simulation_PowerDistributionGetMap(void);

// Make sure to call Simulation_HappinessResetMap() before this one, or the
// happiness flags will be saved on top of the previous state.
void Simulation_PowerDistribution(void);

#endif // SIMULATION_POWER_H__
