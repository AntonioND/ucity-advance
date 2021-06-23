// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <stdint.h>

// Routines adapted from https://en.wikipedia.org/wiki/Xorshift

static uint32_t xorshift32(uint32_t *state)
{
    // Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs"
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;

    return x;
}

static uint64_t xorshift64(uint64_t *state)
{
    uint64_t x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;

    return x;
}

static uint32_t state_fast;

void rand_fast_set_seed(uint32_t seed)
{
    state_fast = seed;
}

uint32_t rand_fast_get_seed(void)
{
    return state_fast;
}

uint32_t rand_fast(void)
{
    return xorshift32(&state_fast);
}

static uint64_t state_slow;

void rand_slow_set_seed(uint64_t seed)
{
    state_slow = seed;
}

uint64_t rand_slow_get_seed(void)
{
    return state_fast;
}

uint32_t rand_slow(void)
{
    return xorshift64(&state_slow) >> 32;
}
