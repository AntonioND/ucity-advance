// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef SIMULATION_SIMULATION_SERVICES_H__
#define SIMULATION_SIMULATION_SERVICES_H__

#include <stdint.h>

// Central tile of the building (tileset_info.h)
void Simulation_Services(uint16_t source_tile);
void Simulation_ServicesBig(uint16_t source_tile);

uint8_t *Simulation_ServicesGetMap(void);

#endif // SIMULATION_SIMULATION_SERVICES_H__

