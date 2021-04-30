// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <string.h>

static void reverse(char *s)
{
    int e = strlen(s) - 1;

    for (int b = 0; b < e; b++, e--)
    {
        char c = s[b];
        s[b] = s[e];
        s[e] = c;
    }
}

int Print_Integer_Decimal(char *str, int value)
{
    int l = 0;

    if (value < 0)
    {
        *str++ = '-';
        value = -value;
        l++;
    }

    char *num = str;

    do
    {
        *str++ = (value % 10) + '0';
        value = value / 10;
        l++;
    }
    while (value > 0);

    *str = '\0';

    reverse(num);

    return l;
}

void Print_Integer_Decimal_Right(char *str, size_t size, int value)
{
    char tempstr[12];
    int l = Print_Integer_Decimal(tempstr, value);

    memset(str, ' ', size);

    char *src = tempstr;
    char *dst = str + size - l - 1;
    memmove(dst, src, l + 1);
}
