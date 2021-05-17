// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <ugba/ugba.h>

#include "room_game/text_messages.h"

static uint8_t persistent_msg_flags[BYTES_SAVE_PERSISTENT_MSG];

// TODO: Adjust messages for the new size of the text box
static const char *msg_text[] = {
    [ID_MSG_EMPTY] =
        "",

    [ID_MSG_POLLUTION_HIGH] =
        "Pollution is too high!",
    [ID_MSG_TRAFFIC_HIGH] =
        "Traffic is too high!",
    [ID_MSG_MONEY_NEGATIVE_CAN_LOAN] =
        "You have run out\n"
        "of money. Consider\n"
        "getting a loan.",
    [ID_MSG_MONEY_NEGATIVE_CANT_LOAN] =
        "You have run out\n"
        "of money!",

    [ID_MSG_CLASS_TOWN] =
        "Your village is\n"
        "now a town!",
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
        "Scientists have\n"
        "invented nuclear\n"
        "power plants!",
    [ID_MSG_TECH_FUSION] =
        "Scientists have\n"
        "invented fusion\n"
        "power plants!",

    [ID_MSG_FIRE_INITED] =
        "A fire has started\n"
        "somewhere!",
    [ID_MSG_NUCLEAR_MELTDOWN] =
        "A nuclear power\n"
        "plant has had a\n"
        "meltdown!",

    [ID_MSG_TECH_INSUFFICIENT] =
        "Technology isn't\n"
        "advanced enough\n"
        "to build that!",
    [ID_MSG_POPULATION_INSUFFICIENT] =
        "There isn't enough\n"
        "population to\n"
        "build that!",
    [ID_MSG_FINISHED_LOAN] =
        "You have finished\n"
        "repaying your\n"
        "loan.",
    [ID_MSG_GAME_OVER_1] =
        "The people are\n"
        "tired of you.",
    [ID_MSG_GAME_OVER_2] =
        "\n"
        "     Game Over",
};

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

    return msg_text[id];
}

int MessageQueueIsEmpty(void)
{
    if (message_queue_size > 0)
        return 0;

    return 1;
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

    MessageQueueAdd(id);
}

void PersistentYearlyMessagesReset(void)
{
    for (int i = 0; i < (ID_MSG_RESET_YEAR_NUM / 8); i++)
        persistent_msg_flags[i] = 0;
}
