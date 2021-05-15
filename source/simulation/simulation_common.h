// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef SIMULATION_SIMULATION_COMMON_H__
#define SIMULATION_SIMULATION_COMMON_H__

typedef enum {
    REQUESTED_DISASTER_NONE,
    REQUESTED_DISASTER_FIRE,
    REQUESTED_DISASTER_MELTDOWN,
} requested_disaster_type;

void Simulation_DisastersSetEnabled(int enable);
int Simulation_AreDisastersEnabled(void);
void Simulation_RequestDisaster(requested_disaster_type type);

void TypeMatrixRefresh(void);
uint8_t *TypeMatrixGet(void);

void Simulation_SetFirstStep(void);
void Simulation_SimulateAll(void);

#endif // SIMULATION_SIMULATION_COMMON_H__
