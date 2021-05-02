// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <stdint.h>

// The functions in this file implement a FIFO circular buffer

int queue_in_ptr;  // Pointer to the place where to add elements
int queue_out_ptr; // Pointer to the place where to read elements

#define BUFFER_SIZE     (1024)

static int circular_buffer[BUFFER_SIZE];

void QueueInit(void)
{
    // Reset pointers
    queue_in_ptr = 0;
    queue_out_ptr = 0;
}

void QueueAdd(uint32_t value)
{
    circular_buffer[queue_in_ptr++] = value;

    if (queue_in_ptr == BUFFER_SIZE)
        queue_in_ptr = 0;

    // TODO: Check for overflows
}

uint32_t QueueGet(void)
{
    uint32_t value = circular_buffer[queue_out_ptr++];

    if (queue_out_ptr == BUFFER_SIZE)
        queue_out_ptr = 0;

    // TODO: Check for underflows

    return value;
}

void QueueAddPair(uint16_t a, uint16_t b)
{
    QueueAdd(((uint32_t)a << 16) | b);
}

void QueueGetPair(uint16_t *a, uint16_t *b)
{
    uint32_t val = QueueGet();
    *a = val >> 16;
    *b = val & 0xFFFF;
}

// Returns 1 if empty
int QueueIsEmpty(void)
{
    if (queue_out_ptr == queue_in_ptr)
        return 1;

    return 0;
}
