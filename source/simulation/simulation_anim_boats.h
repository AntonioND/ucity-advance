// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#ifndef SIMULATION_SIMULATION_ANIM_BOATS_H__
#define SIMULATION_SIMULATION_ANIM_BOATS_H__

void BoatsReset(void);
void BoatsHide(void);
void BoatsShow(void);
void BoatsVBLHandle(void);
void BoatsHandle(void);
void BoatsHandleScroll(int deltax, int deltay);

#endif // SIMULATION_SIMULATION_ANIM_BOATS_H__
