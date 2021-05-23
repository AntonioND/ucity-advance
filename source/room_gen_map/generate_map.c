// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <stdint.h>

#include <ugba/ugba.h>

#include "room_gen_map/generate_map_circle.h"
#include "room_game/draw_common.h"
#include "room_game/room_game.h"
#include "room_game/tileset_info.h"

#define FIELD_DEFAULT_THRESHOLD     (128)
#define FOREST_DEFAULT_THRESHOLD    (128 + 24)

EWRAM_BSS static uint8_t scratch_bank_1[CITY_MAP_WIDTH * CITY_MAP_HEIGHT];
EWRAM_BSS static uint8_t scratch_bank_2[CITY_MAP_WIDTH * CITY_MAP_HEIGHT];

static uint8_t seedx, seedy, seedz, seedw;

static void gen_map_srand(uint8_t seed_x, uint8_t seed_y)
{
    seedx = seed_x; // 21
    seedy = seed_y; // 229
    seedz = 181;
    seedw = 51;
}

static uint8_t gen_map_rand(void)
{
    uint8_t t = seedx ^ (seedx << 3);

    seedx = seedy;
    seedy = seedz;
    seedz = seedw;

    seedw = seedw ^ (seedw >> 5) ^ (t ^ (t >> 2));

    return seedw;
}

// Result saved to bank 1
static void map_initialize(void)
{
    // Initialize bank to random values

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint8_t r = 128 + ((gen_map_rand() & 63) - 32);
            scratch_bank_1[j * CITY_MAP_WIDTH + i] = r;
        }
    }
}

// The coordinates are the top left corner of the circle
static void map_add_circle(uint8_t *buf, int x, int y, int radius, int addval)
{
    UGBA_Assert((radius == 64) || (radius == 32) || (radius == 16) ||
                (radius == 8) || (radius == 4));

    const uint8_t *gen_map_circle = GenMapCircleGet(radius);

    x += radius;
    y += radius;

    for (int j = 1 - radius; j < radius; j++)
    {
        // Check if this is outside of the map
        int mapy = y + j;
        if ((mapy < 0) || (mapy > (CITY_MAP_HEIGHT - 1)))
            continue;

        for (int i = 1 - radius; i < radius; i++)
        {
            // Check if this is outside of the map
            int mapx = x + i;
            if ((mapx < 0) || (mapx > (CITY_MAP_WIDTH - 1)))
                continue;

            int cx = i;
            int cy = j;

            if (cx < 0)
                cx = -cx;
            if (cx > (radius - 1))
                cx = radius - 1;

            if (cy < 0)
                cy = -cy;
            if (cy > (radius - 1))
                cy = radius - 1;

            int value = gen_map_circle[cy * radius + cx];

            if (value == 0)
                continue;

            int val = buf[mapy * CITY_MAP_WIDTH + mapx] + addval;
            if (val < 0)
                val = 0;
            else if (val > 255)
                val = 255;
            buf[mapy * CITY_MAP_WIDTH + mapx] = val;
        }
    }
}

static void map_add_circle_all(uint8_t *buf)
{
    // Only powers of 2 (4 to 64 only)
    const uint8_t circle_radius_array[] = {
        64, 64,
        32, 32, 32, 32,
        16, 16, 16, 16, 16, 16, 16, 16,
        8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
        4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
        4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
        0,
    };

    for (int i = 0; ; i++)
    {
        uint32_t radius = circle_radius_array[i];
        if (radius == 0)
            break;

        // Calculate starting coordinates for the circle
        // x, y = (rand() & (MAP_W - 1)) + (rand() & (R-1)) - R/2

        uint8_t b = gen_map_rand();
        uint8_t c = gen_map_rand();
        uint8_t d = gen_map_rand();
        uint8_t e = gen_map_rand();

        b &= CITY_MAP_WIDTH - 1;
        c &= CITY_MAP_HEIGHT - 1;

        d = d & (radius - 1);
        e = e & (radius - 1);

        d = d - (radius / 2);
        e = e - (radius / 2);

        b = b + d;
        c = c + e;

#define STEP_INCREMENT 16 // Amount to be added with each circle

        int addval = (i & 1) ? -STEP_INCREMENT : STEP_INCREMENT;

        map_add_circle(buf, b, c, radius, addval);
    }
}

static void map_normalize(uint8_t *map)
{
    // Calculate average value

    int32_t sum = 0;

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            sum += (int32_t)map[j * CITY_MAP_WIDTH + i];
        }
    }

    int32_t average = sum / (CITY_MAP_WIDTH * CITY_MAP_HEIGHT);

    // Subtract average value from all tiles in the map, and center it around
    // 128 so that the numbers are unsigned.

    int32_t addval = 128 - average;

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            int32_t val = map[j * CITY_MAP_WIDTH + i] + addval;
            if (val > 255)
                val = 255;
            else if (val < 0)
                val = 0;
            map[j * CITY_MAP_WIDTH + i] = val;
        }
    }
}

static uint8_t read_map_clamped(const uint8_t *map, int x, int y)
{
    if (x < 0)
        x = 0;
    else if (x >= CITY_MAP_WIDTH)
        x = CITY_MAP_WIDTH - 1;

    if (y < 0)
        y = 0;
    else if (y >= CITY_MAP_HEIGHT)
        y = CITY_MAP_HEIGHT - 1;

    return map[y * CITY_MAP_WIDTH + x];
}

static void map_smooth_src_to_dst(const uint8_t *src, uint8_t *dst)
{
    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint32_t left = read_map_clamped(src, i - 1, j);
            uint32_t top = read_map_clamped(src, i, j - 1);
            uint32_t right = read_map_clamped(src, i + 1, j);
            uint32_t bottom = read_map_clamped(src, i, j + 1);
            uint32_t center = src[j * CITY_MAP_WIDTH + i];

            uint32_t total = ((left + top + right + bottom) / 4) + center;

            dst[j * CITY_MAP_WIDTH + i] = total / 2;
        }
    }
}

static void map_apply_height_threshold(uint8_t *map, int field_threshold,
                                       int forest_threshold)
{
    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t tile;

            int val = map[j * CITY_MAP_WIDTH + i];
            if (val < field_threshold)
                tile = T_WATER;
            else if (val < forest_threshold)
                tile = T_GRASS;
            else
                tile = T_FOREST;

            CityMapDrawTile(tile, i, j);
        }
    }
}

typedef struct {
    uint8_t mask;
    uint8_t expected_result;
    uint16_t tile_valid;
} fix_tile_mask;

static const fix_tile_mask fix_tile_mask_table[] = {
    // From more restrictive to less restrictive

    // 8 neighbours of this tile.
    //
    // 0 1 2
    // 3 . 4 <- Bit order
    // 5 6 7

    { 0b11111111, 0b11111111, 1 },

    { 0b11111111, 0b01111111, 1 },
    { 0b11111111, 0b11011111, 1 },
    { 0b11111111, 0b11111011, 1 },
    { 0b11111111, 0b11111110, 1 },

    { 0b01011111, 0b00011111, 1 },
    { 0b11111010, 0b11111000, 1 },
    { 0b01111011, 0b01101011, 1 },
    { 0b11011110, 0b11010110, 1 },

    { 0b01011011, 0b00001011, 1 },
    { 0b01011110, 0b00010110, 1 },
    { 0b01111010, 0b01101000, 1 },
    { 0b11011010, 0b11010000, 1 },

    { 0b00000000, 0b00000000, 0 }, // Default -> Remove tile
};

// T_WATER or T_FOREST
static void map_fix_tile_type(uint16_t tile)
{
    while (1)
    {
        int fix_map_changed = 0;

        for (int j = 0; j < CITY_MAP_HEIGHT; j++)
        {
            for (int i = 0; i < CITY_MAP_WIDTH; i++)
            {
                if (CityMapGetTile(i, j) != tile)
                    continue;

                uint8_t flags = 0;

                if (CityMapGetTileClamped(i - 1, j - 1) == tile)
                    flags |= 1 << 0;
                if (CityMapGetTileClamped(i, j - 1) == tile)
                    flags |= 1 << 1;
                if (CityMapGetTileClamped(i + 1, j - 1) == tile)
                    flags |= 1 << 2;
                if (CityMapGetTileClamped(i - 1, j) == tile)
                    flags |= 1 << 3;
                if (CityMapGetTileClamped(i + 1, j) == tile)
                    flags |= 1 << 4;
                if (CityMapGetTileClamped(i - 1, j + 1) == tile)
                    flags |= 1 << 5;
                if (CityMapGetTileClamped(i, j + 1) == tile)
                    flags |= 1 << 6;
                if (CityMapGetTileClamped(i + 1, j + 1) == tile)
                    flags |= 1 << 7;

                const fix_tile_mask *ft = &fix_tile_mask_table[0];
                while (1)
                {
                    // This loop will always end because the last element of the
                    // table will always pass this check.
                    if ((ft->mask & flags) == ft->expected_result)
                    {
                        if (ft->tile_valid == 0)
                        {
                            CityMapDrawTile(T_GRASS, i, j);
                            fix_map_changed = 1;
                        }

                        break;
                    }

                    ft++;
                }
            }
        }

        if (fix_map_changed == 0)
            break;
    }
}

// Fix invalid patterns of tiles
static void map_tilemap_fix(void)
{
    map_fix_tile_type(T_WATER);
    map_fix_tile_type(T_FOREST);
}

typedef struct {
    uint8_t mask;
    uint8_t expected_result;
    uint16_t resulting_tile;
} convert_tile_mask;

static const convert_tile_mask convert_tile_water_table[] = {
    // From more restrictive to less restrictive

    // 8 neighbours of this tile.
    //
    // 0 1 2
    // 3 . 4 <- Bit order
    // 5 6 7

    // 1 = Water, 0 = Grass or forest

    { 0b11111111, 0b11111111, T_WATER },

    { 0b11111111, 0b01111111, T_WATER__GRASS_CORNER_BR },
    { 0b11111111, 0b11011111, T_WATER__GRASS_CORNER_BL },
    { 0b11111111, 0b11111011, T_WATER__GRASS_CORNER_TR },
    { 0b11111111, 0b11111110, T_WATER__GRASS_CORNER_TL },

    { 0b01011111, 0b00011111, T_WATER__GRASS_BC },
    { 0b11111010, 0b11111000, T_WATER__GRASS_TC },
    { 0b01111011, 0b01101011, T_WATER__GRASS_CR },
    { 0b11011110, 0b11010110, T_WATER__GRASS_CL },

    { 0b01011011, 0b00001011, T_WATER__GRASS_BR },
    { 0b01011110, 0b00010110, T_WATER__GRASS_BL },
    { 0b01111010, 0b01101000, T_WATER__GRASS_TR },
    { 0b11011010, 0b11010000, T_WATER__GRASS_TL },

    // Default -> Error! This shouldn't happen, so use a tile to make it obvious
    { 0b00000000, 0b00000000, T_INDUSTRIAL },
};

static const convert_tile_mask convert_tile_forest_table[] = {
    // From more restrictive to less restrictive

    // 1 = Forest, 0 = Grass or water

    { 0b11111111, 0b11111111, T_FOREST },

    { 0b11111111, 0b01111111, T_GRASS__FOREST_TL },
    { 0b11111111, 0b11011111, T_GRASS__FOREST_TR },
    { 0b11111111, 0b11111011, T_GRASS__FOREST_BL },
    { 0b11111111, 0b11111110, T_GRASS__FOREST_BR },

    { 0b01011111, 0b00011111, T_GRASS__FOREST_TC },
    { 0b11111010, 0b11111000, T_GRASS__FOREST_BC },
    { 0b01111011, 0b01101011, T_GRASS__FOREST_CL },
    { 0b11011110, 0b11010110, T_GRASS__FOREST_CR },

    { 0b01011011, 0b00001011, T_GRASS__FOREST_CORNER_TL },
    { 0b01011110, 0b00010110, T_GRASS__FOREST_CORNER_TR },
    { 0b01111010, 0b01101000, T_GRASS__FOREST_CORNER_BL },
    { 0b11011010, 0b11010000, T_GRASS__FOREST_CORNER_BR },

    { 0b00000000, 0b00000000, T_INDUSTRIAL }, // Default -> Error!
};

static uint16_t get_type_clamp(int x, int y)
{
    if (x < 0)
        x = 0;
    else if (x > (CITY_MAP_WIDTH - 1))
        x = CITY_MAP_WIDTH - 1;

    if (y < 0)
        y = 0;
    else if (y > (CITY_MAP_HEIGHT - 1))
        y = CITY_MAP_HEIGHT - 1;

    return CityMapGetTypeNoBoundCheck(x, y);
}

static void coarse_tiles_to_tileset(uint16_t type,
                                    const convert_tile_mask *ct_table)
{
    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            if (get_type_clamp(i, j) != type)
                continue;

            uint8_t flags = 0;

            if (get_type_clamp(i - 1, j - 1) == type)
                flags |= 1 << 0;
            if (get_type_clamp(i, j - 1) == type)
                flags |= 1 << 1;
            if (get_type_clamp(i + 1, j - 1) == type)
                flags |= 1 << 2;
            if (get_type_clamp(i - 1, j) == type)
                flags |= 1 << 3;
            if (get_type_clamp(i + 1, j) == type)
                flags |= 1 << 4;
            if (get_type_clamp(i - 1, j + 1) == type)
                flags |= 1 << 5;
            if (get_type_clamp(i, j + 1) == type)
                flags |= 1 << 6;
            if (get_type_clamp(i + 1, j + 1) == type)
                flags |= 1 << 7;

            const convert_tile_mask *ct = ct_table;
            while (1)
            {
                // This loop will always end because the last element of the
                // table will always pass this check.
                if ((ct->mask & flags) == ct->expected_result)
                {
                    CityMapDrawTile(ct->resulting_tile, i, j);
                    break;
                }

                ct++;
            }
        }
    }
}

static void map_tilemap_to_real_tiles(void)
{
    // Convert tiles to corners, etc.  T_GRASS will remain unchanged!

    coarse_tiles_to_tileset(TYPE_WATER, convert_tile_water_table);
    coarse_tiles_to_tileset(TYPE_FOREST, convert_tile_forest_table);

    // Randomize some of the tiles to the alternate versions

    int increment = gen_map_rand() & 63;

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            if (increment > 0)
            {
                increment--;
                continue;
            }

            increment = gen_map_rand() & 63;

            uint16_t tile = CityMapGetTile(i, j);

            if (tile == T_GRASS)
                CityMapDrawTile(T_GRASS_EXTRA, i, j);
            else if (tile == T_FOREST)
                CityMapDrawTile(T_FOREST_EXTRA, i, j);
            else if (tile == T_WATER)
                CityMapDrawTile(T_WATER_EXTRA, i, j);

        }
    }
}

void Generate_Map(uint8_t seed_x, uint8_t seed_y, int offset)
{
    int field_threshold = FIELD_DEFAULT_THRESHOLD + offset;
    int forest_threshold = FOREST_DEFAULT_THRESHOLD + offset;

    gen_map_srand(seed_x, seed_y);

    map_initialize(); // Result is saved to temp bank 1

    map_smooth_src_to_dst(scratch_bank_1, scratch_bank_2); // Bank 1 -> 2

    map_add_circle_all(scratch_bank_2);

    map_normalize(scratch_bank_2);

    map_smooth_src_to_dst(scratch_bank_2, scratch_bank_1); // Bank 2 -> 1
    map_smooth_src_to_dst(scratch_bank_1, scratch_bank_2); // Bank 1 -> 2

    // Convert to water / field / forest
    map_apply_height_threshold(scratch_bank_2, field_threshold,
                               forest_threshold);

    // Convert to a real map that can be converted to tiles, not all forms are
    // allowed by the tileset
    map_tilemap_fix();

    map_tilemap_to_real_tiles();
}
