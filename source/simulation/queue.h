// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef SIMULATION_QUEUE_H__
#define SIMULATION_QUEUE_H__

#include <stdint.h>

void QueueInit(void);

void QueueAdd(int value);
int QueueGet(void);

void QueueAddPair(uint16_t a, uint16_t b);
void QueueGetPair(uint16_t *a, uint16_t *b);

int QueueIsEmpty(void);

#endif // SIMULATION_QUEUE_H__
