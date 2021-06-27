// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef SIMULATION_SERVICES_H__
#define SIMULATION_SERVICES_H__

#include <stdint.h>

// Central tile of the building (tileset_info.h)
void Simulation_Services(uint16_t source_tile);
void Simulation_ServicesBig(uint16_t source_tile);

uint8_t *Simulation_ServicesGetMap(void);

void Simulation_ServicesSetTileOkFlag(void);
void Simulation_ServicesAddTileOkFlag(void);
void Simulation_EducationSetTileOkFlag(void);
void Simulation_EducationAddTileOkFlag(void);

#endif // SIMULATION_SERVICES_H__

