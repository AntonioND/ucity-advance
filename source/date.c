// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <stdio.h>

static int date_year;
static int date_month; // 0 (January) - 11 (December)

static const char *month_name[12] = {
    "  January",
    " February",
    "    March",
    "    April",
    "      May",
    "     June",
    "     July",
    "   August",
    "September",
    "  October",
    " November",
    " December",
};

static char date_str[30];

const char *DateString(void)
{
    snprintf(date_str, sizeof(date_str),
             "%s %4d", month_name[date_month], date_year);

    return date_str;
}

void DateSet(int month, int year)
{
    date_month = month;
    date_year = year;
}

void DateReset(void)
{
    DateSet(0, 1950);
}

void DateStep(void)
{
    date_month++;
    if (date_month > 11)
    {
        date_month = 0;
        if (date_year < 9999)
        {
            // If year 9999 is reached, stay there!
            // Months will continue to increment
            date_year++;
        }
    }
}
