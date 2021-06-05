// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#ifndef SIMULATION_SIMULATION_ANIM_PLANES_H__
#define SIMULATION_SIMULATION_ANIM_PLANES_H__

void PlanesReset(void);
void PlanesHide(void);
void PlanesShow(void);
void PlanesVBLHandle(void);
void PlanesHandle(void);
void PlanesHandleScroll(int deltax, int deltay);

#endif // SIMULATION_SIMULATION_ANIM_PLANES_H__
