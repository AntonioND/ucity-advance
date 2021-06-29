// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <stdint.h>

#include <ugba/ugba.h>

void copy_map_to_sbb(const void *source, void *dest, int width, int height)
{
    uint16_t *src_ptr = (uint16_t *)source;
    uint16_t *dst_ptr = (uint16_t *)dest;

    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            int index = 0;

            if (i >= 32)
                index += 32 * 32;
            if (j >= 32)
                index += 32 * 64;

            index += i % 32;
            index += (j % 32) * 32;

            dst_ptr[index] = *src_ptr++;
        }
    }
}

IWRAM_CODE uint16_t *get_pointer_sbb(void *map, int x, int y)
{
    uint16_t *ptr = (uint16_t *)map;

    int index = 0;

    if (x >= 32)
        index += 32 * 32;
    if (y >= 32)
        index += 32 * 64;

    index += x % 32;
    index += (y % 32) * 32;

    return &ptr[index];
}

IWRAM_CODE void write_tile_sbb(uint16_t entry, void *dest, int x, int y)
{
    uint16_t *dst_ptr = get_pointer_sbb(dest, x, y);
    *dst_ptr = entry;
}

IWRAM_CODE void toggle_hflip_tile_sbb(void *map, int x, int y)
{
    uint16_t *map_ptr = get_pointer_sbb(map, x, y);
    *map_ptr ^= MAP_REGULAR_HFLIP;
}

IWRAM_CODE void toggle_vflip_tile_sbb(void *map, int x, int y)
{
    uint16_t *map_ptr = get_pointer_sbb(map, x, y);
    *map_ptr ^= MAP_REGULAR_VFLIP;
}

IWRAM_CODE uint16_t read_tile_sbb(void *source, int x, int y)
{
    uint16_t *src_ptr = get_pointer_sbb(source, x, y);
    return *src_ptr;
}
