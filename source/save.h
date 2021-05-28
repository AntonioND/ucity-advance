// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#ifndef SAVE_H__
#define SAVE_H__

#include <assert.h>

#include <ugba/ugba.h>

#include "room_game/room_game.h"
#include "room_graphs/graphs_handler.h"

#define MAGIC_STRING        "UCY0"
#define MAGIC_STRING_LEN    4

typedef struct {
    uint8_t     name[CITY_MAX_NAME_LENGTH];

    uint8_t     month;
    uint16_t    year;

    int32_t     funds;

    // Last scroll position when saving the game
    uint8_t     last_scroll_x;
    uint8_t     last_scroll_y;

    uint8_t     tax_percent;

    uint8_t     technology_level;

    uint8_t     negative_budget_count;

    uint8_t     loan_remaining_payments; // 0 if no loan active
    uint16_t    loan_payment_amount;

    // TODO: Persistent message flags

    graph_info  graph_population;
    graph_info  graph_residential;
    graph_info  graph_commercial;
    graph_info  graph_industrial;
    graph_info  graph_funds;

    uint8_t     map_lsb[CITY_MAP_WIDTH * CITY_MAP_HEIGHT];
    uint8_t     map_msb[CITY_MAP_WIDTH * CITY_MAP_HEIGHT / 8];

} city_save_data;

typedef struct {
    uint8_t     magic_string[MAGIC_STRING_LEN];
    uint32_t    checksum;

    uint8_t     disasters_enabled;
    uint8_t     animations_enabled;
    uint8_t     music_enabled;

    city_save_data city[4];
} save_data;

// TODO: Support more than 32 KB if available (MEM_SRAM_SIZE)
static_assert(sizeof(save_data) <= 32 * 1024,
              "Save data struct doesn't fit in SRAM");

// ----------------------------------------------------------------------------

save_data *Save_Data_Get(void);
city_save_data *Save_Data_Get_City(int index);

void Save_Reset_Checksum(void);

void Save_Data_Reset_City(int index);
void Save_Data_Reset(void);

void Save_Data_Check(void);

#endif // SAVE_H__
