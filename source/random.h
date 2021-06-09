// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#ifndef RANDOM_H__
#define RANDOM_H__

#include <stdint.h>

#define RAND_FAST_DEFAULT_SEED  0x12345678

void rand_fast_set_seed(uint32_t seed);
uint32_t rand_fast_get_seed(void);
uint32_t rand_fast(void);

void rand_slow_set_seed(uint64_t seed);
uint64_t rand_slow_get_seed(void);
uint32_t rand_slow(void);

#endif // RANDOM_H__
