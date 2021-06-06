// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef DATE_H__
#define DATE_H__

#include <stdint.h>

const char *DateString(void);

void DateStringMake(char *buf, uint32_t month, uint32_t year);

void DateSet(uint32_t month, uint32_t year);

// 0 = January, 11 = December
int DateGetMonth(void);
int DateGetYear(void);

void DateStep(void);

#endif // DATE_H__
