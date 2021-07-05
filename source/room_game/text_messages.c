// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <ugba/ugba.h>

#include <string.h>

#include "jukebox.h"
#include "room_game/text_messages.h"

static uint8_t persistent_msg_flags[BYTES_SAVE_PERSISTENT_MSG];

static const char *msg_text[] = {
    [ID_MSG_EMPTY] =
        "",

    [ID_MSG_POLLUTION_HIGH] =
        "Air pollution is too\n"
        "high!",
    [ID_MSG_TRAFFIC_HIGH] =
        "Traffic is too high!",
    [ID_MSG_MONEY_NEGATIVE_CAN_LOAN] =
        "You have run out\n"
        "of money. Consider\n"
        "getting a loan.",
    [ID_MSG_MONEY_NEGATIVE_CANT_LOAN] =
        "You have run out of\n"
        "money!",

    [ID_MSG_CLASS_TOWN] =
        "Your village is now a\n"
        "town!",
    [ID_MSG_CLASS_CITY] =
        "Your town is now a\n"
        "city!",
    [ID_MSG_CLASS_METROPOLIS] =
        "Your city is now a\n"
        "metropolis!",
    [ID_MSG_CLASS_CAPITAL] =
        "Your metropolis is\n"
        "now a capital!",

    [ID_MSG_TECH_NUCLEAR] =
        "Scientists have just\n"
        "invented nuclear\n"
        "power plants!",
    [ID_MSG_TECH_FUSION] =
        "Scientists have just\n"
        "invented fusion\n"
        "power plants!",

    [ID_MSG_FIRE_INITED] =
        "A fire has started\n"
        "somewhere in the city!",
    [ID_MSG_NUCLEAR_MELTDOWN] =
        "A nuclear power\n"
        "plant has just had a\n"
        "meltdown!",

    [ID_MSG_FINISHED_LOAN] =
        "You have finished\n"
        "repaying your loan.\n",
    [ID_MSG_GAME_OVER_1] =
        "The people of this\n"
        "city are tired of you.",
    [ID_MSG_GAME_OVER_2] =
        "\n"
        "      GAME  OVER",
};

static char custom_message_string[60];

void CustomMessageStringSet(const char *str)
{
    strncpy(&custom_message_string[0], str, sizeof(custom_message_string));
    custom_message_string[sizeof(custom_message_string) - 1] = '\0';
}

#define MESSAGE_QUEUE_SIZE  10

static int message_queue[MESSAGE_QUEUE_SIZE];
static int message_queue_read_ptr;
static int message_queue_write_ptr;
static int message_queue_size;

void MessageQueueInit(void)
{
    message_queue_read_ptr = 0;
    message_queue_write_ptr = 0;
    message_queue_size = 0;
}

void MessageQueueAdd(int id)
{
    UGBA_Assert(message_queue_size < MESSAGE_QUEUE_SIZE);

    message_queue[message_queue_write_ptr++] = id;

    if (message_queue_write_ptr == MESSAGE_QUEUE_SIZE)
        message_queue_write_ptr = 0;

    message_queue_size++;
}

const char *MessageQueueGet(void)
{
    UGBA_Assert(message_queue_size > 0);

    int id = message_queue[message_queue_read_ptr++];

    if (message_queue_read_ptr == MESSAGE_QUEUE_SIZE)
        message_queue_read_ptr = 0;

    message_queue_size--;

    if (id == ID_MSG_CUSTOM)
        return &custom_message_string[0];

    return msg_text[id];
}

int MessageQueueIsEmpty(void)
{
    if (message_queue_size > 0)
        return 0;

    return 1;
}

void PersistentMessageFlagAsShown(message_ids id)
{
    // Message IDs start at 1, so subtract 1.

    int which_byte = (id - 1) / 8;
    int which_bit = (id - 1) % 8;
    uint8_t bit_mask = 1 << which_bit;

    persistent_msg_flags[which_byte] |= bit_mask;
}

// The message ID should be a valid persistent message ID
void PersistentMessageShow(message_ids id)
{
    // Message IDs start at 1, so subtract 1.

    int which_byte = (id - 1) / 8;
    int which_bit = (id - 1) % 8;
    uint8_t bit_mask = 1 << which_bit;

    // If this message has already been shown, don't show it again

    if (persistent_msg_flags[which_byte] & bit_mask)
        return;

    persistent_msg_flags[which_byte] |= bit_mask;

    // If this is a change of city type, refresh music

    if ((id == ID_MSG_CLASS_TOWN) || (id == ID_MSG_CLASS_CITY) ||
        (id == ID_MSG_CLASS_METROPOLIS) || (id == ID_MSG_CLASS_CAPITAL))
    {
        Jukebox_RoomSet(JUKEBOX_ROOM_GAME);
    }

    // Finally, show message

    MessageQueueAdd(id);
}

void PersistentYearlyMessagesReset(void)
{
    for (int i = 0; i < (ID_MSG_RESET_YEAR_NUM / 8); i++)
        persistent_msg_flags[i] = 0;
}

void PersistentMessageFlagsReset(void)
{
    memset(persistent_msg_flags, 0, BYTES_SAVE_PERSISTENT_MSG);
}

void PersistentMessageFlagsSet(uint8_t *flags)
{
    memcpy(flags, persistent_msg_flags, BYTES_SAVE_PERSISTENT_MSG);
}

void PersistentMessageFlagsGet(const uint8_t *flags)
{
    memcpy(persistent_msg_flags, flags, BYTES_SAVE_PERSISTENT_MSG);
}
