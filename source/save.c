// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <stddef.h>
#include <string.h>

#include <ugba/ugba.h>

#include "audio.h"
#include "random.h"
#include "save.h"
#include "simulation/simulation_common.h"

volatile save_data *Save_Data_Get(void)
{
    return MEM_SRAM;
}

volatile city_save_data *Save_Data_Get_City(int index)
{
    volatile save_data *sav = Save_Data_Get();

    return &(sav->city[index]);
}

city_save_data *Save_Data_Get_City_Temporary(void)
{
    static city_save_data temp_city;

    return &temp_city;
}

static void Save_Data_Copy(volatile void *dst, const volatile void *src,
                           size_t size)
{
    volatile uint8_t *s = (volatile uint8_t *)src;
    volatile uint8_t *d = (volatile uint8_t *)dst;

    for (size_t i = 0; i < size; i++)
        *d++ = *s++;
}

void Save_Data_Safe_Copy(volatile city_save_data *dst,
                         const volatile city_save_data *src)
{
    Save_Data_Copy((void *)dst, (const void *)src, sizeof(city_save_data));
}

static uint32_t Save_Calculate_Checksum(void)
{
    volatile save_data *sav = Save_Data_Get();
    uint8_t *src = (uint8_t *)sav;
    uint32_t offset = offsetof(save_data, checksum) + sizeof(sav->checksum);
    uint32_t checksum = 0;
    for (size_t i = offset; i < sizeof(save_data); i++)
        checksum += (uint32_t)src[i];
    return checksum;
}

void Save_Reset_Checksum(void)
{
    volatile save_data *sav = Save_Data_Get();
    uint32_t checksum = Save_Calculate_Checksum();

    sav->checksum[0] = (checksum >> 0) & 0xFF;
    sav->checksum[1] = (checksum >> 8) & 0xFF;
    sav->checksum[2] = (checksum >> 16) & 0xFF;
    sav->checksum[3] = (checksum >> 24) & 0xFF;
}

void Save_Data_Reset_City(int index)
{
    volatile city_save_data *sav_city = Save_Data_Get_City(index);

    city_save_data *city = Save_Data_Get_City_Temporary();

    memset(city, 0, sizeof(city_save_data));
    Graph_Reset(&(city->graph_population));
    Graph_Reset(&(city->graph_residential));
    Graph_Reset(&(city->graph_commercial));
    Graph_Reset(&(city->graph_industrial));
    Graph_Reset(&(city->graph_funds));

    // Use the fast random number generator to generate the starting seed for
    // the city.
    city->rand_slow_seed = rand_fast();

    Save_Data_Safe_Copy(sav_city, city);
}

void Save_Data_Reset(void)
{
    volatile save_data *sav = Save_Data_Get();

    // Clear everything
    for (size_t i = 0; i < sizeof(save_data); i++)
        ((uint8_t *)sav)[i] = 0;

    // Save magic string
    Save_Data_Copy(sav->magic_string, MAGIC_STRING, MAGIC_STRING_LEN);

    // Default settings
    Simulation_DisastersSetEnabled(1);
    Room_Game_SetAnimationsEnabled(1);
    Audio_Enable_Set(1);
    sav->disasters_enabled = 1;
    sav->animations_enabled = 1;
    sav->music_enabled = 1;

    rand_fast_set_seed(RAND_FAST_DEFAULT_SEED);
    sav->rand_fast_seed[3] = (RAND_FAST_DEFAULT_SEED >> 24) & 0xFF;
    sav->rand_fast_seed[2] = (RAND_FAST_DEFAULT_SEED >> 16) & 0xFF;
    sav->rand_fast_seed[1] = (RAND_FAST_DEFAULT_SEED >> 8) & 0xFF;
    sav->rand_fast_seed[0] = (RAND_FAST_DEFAULT_SEED >> 0) & 0xFF;

    for (int i = 0; i < 4; i++)
        Save_Data_Reset_City(i);

    // Save checksum
    Save_Reset_Checksum();
}

// This checks that the saved data is correct, and resets it if it isn't.
void Save_Data_Check(void)
{
    volatile save_data *sav = Save_Data_Get();

    // Verify magic string
    const char *magic_string = MAGIC_STRING;
    for (size_t i = 0; i < MAGIC_STRING_LEN; i++)
    {
        if (sav->magic_string[i] != magic_string[i])
        {
            Save_Data_Reset();
            return;
        }
    }

    uint32_t checksum = Save_Calculate_Checksum();

    // Verify checksum
    if ((sav->checksum[0] != ((checksum >> 0) & 0xFF)) ||
        (sav->checksum[1] != ((checksum >> 8) & 0xFF)) ||
        (sav->checksum[2] != ((checksum >> 16) & 0xFF)) ||
        (sav->checksum[3] != ((checksum >> 24) & 0xFF)))
    {
        Save_Data_Reset();
        return;
    }
}
