// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <stdint.h>

#include <ugba/ugba.h>

#include <umod/umod.h>

// Assets

#include "audio/umod_pack_bin.h"

// Buffer size needs to be a multiple of 16 (the amount of bytes copied to the
// FIFO whenever it gets data from DMA).
//
// Timer Reload = Clocks per frame / buffer size = 280896 / buffer size
// It needs to be an exact division.
//
// Sample rate = Buffer size * FPS = Buffer size * CPU Freq / Clocks per frame
// Sample rate = (Buffer size * 16 * 1024 * 1024) / 280896
//
// Valid combinations:
//
// Sample Rate | Timer Reload | Buffer Size
// ------------+--------------+------------
// 10512.04    | 1596         | 176
// 13378.96    | 1254         | 224
// 18157.16    | 924          | 304
// 21024.08    | 798          | 352
// 26757.92    | 627          | 448
// 31536.12    | 532          | 528
// 36314.32    | 462          | 608
// 40136.88    | 418          | 672

#define SAMPLE_RATE         31536

#define TICKS_PER_RELOAD    532
#define RELOAD_VALUE        (65536 - TICKS_PER_RELOAD)

#define BUFFER_SIZE         528

#define DMA_TIMER_INDEX     0 // Timer 0 controls the transfer rate of DMA A/B

static int current_dma_buffer = 0;

ALIGNED(32) int8_t wave_a[BUFFER_SIZE * 2];
ALIGNED(32) int8_t wave_b[BUFFER_SIZE * 2];

void Audio_Init(void)
{
    UMOD_Init(SAMPLE_RATE);
    UMOD_LoadPack(umod_pack_bin);

    // The sound hardware needs to be enabled to write to any other register.
    SOUND_MasterEnable(1);
    SOUND_DMA_Volume(100, 100);
    SOUND_DMA_Pan(1, 0, 0, 1); // DMA A to the left, DMA B to the right

    SOUND_DMA_TimerSetup(DMA_TIMER_INDEX, DMA_TIMER_INDEX);
    TM_TimerStop(DMA_TIMER_INDEX);
    TM_TimerStart(DMA_TIMER_INDEX, RELOAD_VALUE, 1, 0);

    SOUND_DMA_Setup_AB(wave_a, wave_b);
}

IWRAM_CODE ARM_CODE void Audio_Swap_Buffers(void)
{
    if (current_dma_buffer == 0)
        SOUND_DMA_Retrigger_AB();
}

IWRAM_CODE ARM_CODE void Audio_Mix(void)
{
    if (current_dma_buffer == 1)
        UMOD_Mix(wave_a, wave_b, BUFFER_SIZE);
    else
        UMOD_Mix(&wave_a[BUFFER_SIZE], &wave_b[BUFFER_SIZE], BUFFER_SIZE);

    current_dma_buffer ^= 1;
}

static uint32_t current_song = UINT32_MAX;
static int audio_enabled = 1;

void Audio_Enable_Set(int enabled)
{
    audio_enabled = enabled;

    if (audio_enabled)
        UMOD_Song_Play(current_song);
    else
         UMOD_Song_Stop();
}

int Audio_Enable_Get(void)
{
    return audio_enabled;
}

void Audio_Song_Play_Force(uint32_t song)
{
    current_song = song;

    if (audio_enabled)
        UMOD_Song_Play(song);
    else
         UMOD_Song_Stop();
}

// Play song, but don't restart it if it is already playing
void Audio_Song_Play(uint32_t song)
{
    if (current_song == song)
        return;

    Audio_Song_Play_Force(song);
}
