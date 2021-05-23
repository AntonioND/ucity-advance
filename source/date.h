// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef DATE_H__
#define DATE_H__

const char *DateString(void);

void DateSet(int month, int year);

// 0 = January, 11 = December
int DateGetMonth(void);

void DateStep(void);

#endif // DATE_H__
