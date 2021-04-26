// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <ugba/ugba.h>

#define PAD_AUTOREPEAT_WAIT_INITIAL     (10)
#define PAD_AUTOREPEAT_WAIT_REPEAT      (4)

// When it reaches 0, it is pressed
static int count_up = PAD_AUTOREPEAT_WAIT_INITIAL;
static int count_down = PAD_AUTOREPEAT_WAIT_INITIAL;
static int count_left = PAD_AUTOREPEAT_WAIT_INITIAL;
static int count_right = PAD_AUTOREPEAT_WAIT_INITIAL;

void Key_Autorepeat_Update(void)
{
    uint16_t keys = KEYS_Held();

    if (keys & KEY_UP)
    {
        if (count_up == 0)
            count_up = PAD_AUTOREPEAT_WAIT_REPEAT;

         count_up--;
    }
    else
    {
        count_up = PAD_AUTOREPEAT_WAIT_INITIAL;
    }

    if (keys & KEY_DOWN)
    {
        if (count_down == 0)
            count_down = PAD_AUTOREPEAT_WAIT_REPEAT;

        count_down--;
    }
    else
    {
        count_down = PAD_AUTOREPEAT_WAIT_INITIAL;
    }

    if (keys & KEY_RIGHT)
    {
        if (count_right == 0)
            count_right = PAD_AUTOREPEAT_WAIT_REPEAT;

        count_right--;
    }
    else
    {
        count_right = PAD_AUTOREPEAT_WAIT_INITIAL;
    }

    if (keys & KEY_LEFT)
    {
        if (count_left == 0)
            count_left = PAD_AUTOREPEAT_WAIT_REPEAT;

        count_left--;
    }
    else
    {
        count_left = PAD_AUTOREPEAT_WAIT_INITIAL;
    }
}

int Key_Autorepeat_Pressed_Up(void)
{
    if (KEYS_Pressed() & KEY_UP)
        return 1;

    if (count_up == 0)
        return 1;

    return 0;
}

int Key_Autorepeat_Pressed_Down(void)
{
    if (KEYS_Pressed() & KEY_DOWN)
        return 1;

    if (count_down == 0)
        return 1;

    return 0;
}

int Key_Autorepeat_Pressed_Right(void)
{
    if (KEYS_Pressed() & KEY_RIGHT)
        return 1;

    if (count_right == 0)
        return 1;

    return 0;
}

int Key_Autorepeat_Pressed_Left(void)
{
    if (KEYS_Pressed() & KEY_LEFT)
        return 1;

    if (count_left == 0)
        return 1;

    return 0;
}
