// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <stddef.h>
#include <string.h>

#include <ugba/ugba.h>

#include "save.h"

save_data *Save_Data_Get(void)
{
    return MEM_SRAM;
}

city_save_data *Save_Data_Get_City(int index)
{
    save_data *sav = Save_Data_Get();

    return &(sav->city[index]);
}

static uint32_t Save_Calculate_Checksum(void)
{
    save_data *sav = Save_Data_Get();
    uint8_t *src = (uint8_t *)sav;
    uint32_t offset = offsetof(save_data, checksum) + sizeof(sav->checksum);
    uint32_t checksum = 0;
    for (size_t i = offset; i < sizeof(save_data); i++)
        checksum += (uint32_t)src[i];
    return checksum;
}

void Save_Reset_Checksum(void)
{
    save_data *sav = Save_Data_Get();
    sav->checksum = Save_Calculate_Checksum();
}

void Save_Data_Reset_City(int index)
{
    city_save_data *city = Save_Data_Get_City(index);

    memset(city, 0, sizeof(city_save_data));
    Graph_Reset(&(city->graph_population));
    Graph_Reset(&(city->graph_residential));
    Graph_Reset(&(city->graph_commercial));
    Graph_Reset(&(city->graph_industrial));
    Graph_Reset(&(city->graph_funds));
}

void Save_Data_Reset(void)
{
    save_data *sav = Save_Data_Get();

    // Clear everything
    memset(sav, 0, sizeof(save_data));

    // Save magic string
    memcpy(sav->magic_string, MAGIC_STRING, MAGIC_STRING_LEN);

    for (int i = 0; i < 4; i++)
        Save_Data_Reset_City(i);

    // Save checksum
    sav->checksum = Save_Calculate_Checksum();
}

// This checks that the saved data is correct, and resets it if it isn't.
void Save_Data_Check(void)
{
    save_data *sav = Save_Data_Get();

    // Verify magic string
    if (memcmp(sav->magic_string, MAGIC_STRING, MAGIC_STRING_LEN) != 0)
    {
        Save_Data_Reset();
        return;
    }

    // Verify checksum
    if (sav->checksum != Save_Calculate_Checksum())
    {
        Save_Data_Reset();
        return;
    }
}
