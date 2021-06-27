// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <string.h>

#include <ugba/ugba.h>

#include "room_game/status_bar.h"

// Assets

#include "maps/status_bar/notification_bg_bin.h"

#define NOTIFICATION_BOX_MAP_BASE   MEM_BG_MAP_BLOCK_ADDR(30)

void Notification_Box_Show(void)
{
    BG_RegularScrollSet(0, 0, -(8 * 7));
}

void Notification_Box_Hide(void)
{
    BG_RegularScrollSet(0, 0, 8 * 8);
}

void Notification_Box_Clear(void)
{
    uint16_t *src = (uint16_t *)notification_bg_bin;
    uint16_t *dst = (uint16_t *)NOTIFICATION_BOX_MAP_BASE;

    for (int i = 0; i < 32 * 32; i++)
    {
        *dst = MAP_REGULAR_TILE(*src) | MAP_REGULAR_PALETTE(TEXT_PALETTE);
        src++;
        dst++;
    }
}

static char notification_text[90];
static int notification_text_len = 0;

void Notification_Box_Load(void)
{
    notification_text[0] = '\0';

    Notification_Box_Clear();

    Notification_Box_Hide();

    // Setup background
    BG_RegularInit(0, BG_REGULAR_256x256, BG_16_COLORS,
                   TEXT_TILES_BASE, NOTIFICATION_BOX_MAP_BASE);
}

void Notification_Box_Set_Text(const char *text)
{
    notification_text_len = 0;
    strncpy(&notification_text[0], text, sizeof(notification_text));
    notification_text[sizeof(notification_text) - 1] = '\0';
}

void Notification_Box_Print(const char *text, int len)
{
    const int minx = 4;
    const int miny = 1;
    const int maxx = 25;
    const int maxy = 4;

    int x = minx;
    int y = miny;

    while (1)
    {
        int c = (uint8_t)*text++;
        if (c == '\0')
        {
            return;
        }
        else if (c == '\n')
        {
            x = minx;
            y++;
        }
        else
        {
            uintptr_t addr = NOTIFICATION_BOX_MAP_BASE + (y * 32 + x) * 2;

            uint16_t *ptr = (uint16_t *)addr;

            *ptr = MAP_REGULAR_TILE(c) | MAP_REGULAR_PALETTE(TEXT_PALETTE);

            x++;
            if (x > maxx)
            {
                x = minx;
                y++;
            }
        }

        if (y == maxy)
            return;

        len--;
        if (len == 0)
            return;
    }
}

int Notification_Box_Message_Is_Completed(void)
{
    if (notification_text[notification_text_len] == '\0')
        return 1;

    return 0;
}

void Notification_Box_Update(void)
{
    if (notification_text[notification_text_len] == '\0')
        return;

    for (int i = 0; i < 2; i++)
    {
        if (notification_text[notification_text_len] == '\0')
            break;

        notification_text_len++;
    }

    Notification_Box_Print(notification_text, notification_text_len);
}
