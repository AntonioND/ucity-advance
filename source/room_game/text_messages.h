// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#ifndef ROOM_GAME_TEXT_MESSAGES_H__
#define ROOM_GAME_TEXT_MESSAGES_H__

typedef enum {
    ID_MSG_EMPTY                        = 0,

    // Messages that are only shown once per year.
    ID_MSG_POLLUTION_HIGH               = 1,
    ID_MSG_TRAFFIC_HIGH                 = 2,
    ID_MSG_MONEY_NEGATIVE_CAN_LOAN      = 3,
    ID_MSG_MONEY_NEGATIVE_CANT_LOAN     = 4,

    ID_MSG_RESET_YEAR_NUM               = 8, // Multiple of 8

    // Persistent messages (they are only shown once per city)
    ID_MSG_CLASS_TOWN                   = 9,
    ID_MSG_CLASS_CITY                   = 10,
    ID_MSG_CLASS_METROPOLIS             = 11,
    ID_MSG_CLASS_CAPITAL                = 12,
    ID_MSG_TECH_NUCLEAR                 = 13,
    ID_MSG_TECH_FUSION                  = 14,

    ID_MSG_PERSISTENT_NUM               = 16, // Multiple of 8

    // Regular (non-persistent) messages
    ID_MSG_FIRE_INITED                  = 17,
    ID_MSG_NUCLEAR_MELTDOWN             = 18,
    ID_MSG_TECH_INSUFFICIENT            = 19,
    ID_MSG_POPULATION_INSUFFICIENT      = 20,
    ID_MSG_FINISHED_LOAN                = 21,
    ID_MSG_GAME_OVER_1                  = 22,
    ID_MSG_GAME_OVER_2                  = 23,
} message_ids;

// Number of bytes needed to store a flag for each persistent message. Rounded
// up to 8 bits.
#define BYTES_SAVE_PERSISTENT_MSG EQU   ((ID_MSG_PERSISTENT_NUM + 7) / 8)

void MessageQueueAdd(int id);
const char *MessageQueueGet(void);
int MessageQueueIsEmpty(void);

#endif // ROOM_GAME_TEXT_MESSAGES_H__
