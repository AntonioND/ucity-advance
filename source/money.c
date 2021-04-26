// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <stdint.h>

static int32_t global_money = 99999999;

int MoneyIsThereEnough(int32_t cost)
{
    if (global_money >= cost)
        return 1;

    return 0;
}

void MoneyReduce(int32_t cost)
{
    global_money -= cost;
}

int32_t MoneyGet(void)
{
    return global_money;
}
