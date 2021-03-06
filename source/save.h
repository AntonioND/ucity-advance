// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#ifndef SAVE_H__
#define SAVE_H__

#include <assert.h>

#include <ugba/ugba.h>

#include "room_game/room_game.h"
#include "room_game/text_messages.h"
#include "room_graphs/graphs_handler.h"

#define MAGIC_STRING        "UCY0"
#define MAGIC_STRING_LEN    4

typedef struct {
    uint8_t     name[CITY_MAX_NAME_LENGTH];

    uint8_t     month;
    uint16_t    year;

    uint64_t    rand_slow_seed;

    int32_t     funds;

    int32_t     city_type;

    // Last scroll position when saving the game
    uint8_t     last_scroll_x;
    uint8_t     last_scroll_y;

    uint8_t     tax_percent;

    uint8_t     technology_level;

    uint8_t     negative_budget_count;

    uint8_t     loan_remaining_payments; // 0 if no loan active
    uint16_t    loan_payment_amount;

    uint8_t     persistent_msg_flags[BYTES_SAVE_PERSISTENT_MSG];

    graph_info  graph_population;
    graph_info  graph_residential;
    graph_info  graph_commercial;
    graph_info  graph_industrial;
    graph_info  graph_funds;

    uint8_t     map_lsb[CITY_MAP_WIDTH * CITY_MAP_HEIGHT];
    uint8_t     map_msb[CITY_MAP_WIDTH * CITY_MAP_HEIGHT / 8];

} city_save_data;

typedef struct {
    // Make sure that all entries here are always read as uint8_t because SRAM
    // can only be accessed in 8-bit accesses.
    uint8_t     magic_string[MAGIC_STRING_LEN];
    uint8_t     checksum[4];

    uint8_t     rand_fast_seed[4];

    uint8_t     disasters_enabled;
    uint8_t     animations_enabled;
    uint8_t     music_enabled;
    uint8_t     new_graphics;

    city_save_data city[4];
} save_data;

// TODO: Support more than 32 KB if available (MEM_SRAM_SIZE)
static_assert(sizeof(save_data) <= 32 * 1024,
              "Save data struct doesn't fit in SRAM");

// ----------------------------------------------------------------------------

volatile save_data *Save_Data_Get(void);
volatile city_save_data *Save_Data_Get_City(int index);

city_save_data *Save_Data_Get_City_Temporary(void);

void Save_Data_Safe_Copy(volatile city_save_data *dst,
                         const volatile city_save_data *src);

void Save_Reset_Checksum(void);

void Save_Data_Reset_City(int index);
void Save_Data_Reset(void);

void Save_Data_Check(void);

#endif // SAVE_H__
