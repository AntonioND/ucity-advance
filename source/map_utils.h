// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef MAP_UTILS_H__
#define MAP_UTILS_H__

#include <stdint.h>

void copy_map_to_sbb(const void *source, void *dest, int width, int height);
void write_tile_sbb(uint16_t entry, void *dest, int x, int y);
uint16_t read_tile_sbb(void *source, int x, int y);

#endif // MAP_UTILS_H__
