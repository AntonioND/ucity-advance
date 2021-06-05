// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#ifndef SIMULATION_SIMULATION_ANIM_TRAINS_H__
#define SIMULATION_SIMULATION_ANIM_TRAINS_H__

void TrainsReset(void);
void TrainsHide(void);
void TrainsShow(void);
void TrainsVBLHandle(void);
void TrainsHandle(void);
void TrainsHandleScroll(int deltax, int deltay);

#endif // SIMULATION_SIMULATION_ANIM_TRAINS_H__
