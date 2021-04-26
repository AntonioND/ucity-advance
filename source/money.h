// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef MONEY_H__
#define MONEY_H__

#include <stdint.h>

int MoneyIsThereEnough(int32_t cost);
void MoneyReduce(int32_t cost);
int32_t MoneyGet(void);

#endif // MONEY_H__
