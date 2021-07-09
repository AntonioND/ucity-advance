// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#ifndef AUDIO_H__
#define AUDIO_H__

#include <ugba/ugba.h>

// Assets. By including this header here it isn't needed to include it in every
// file that wants to play a song or sound effect.
#include "audio/umod_pack_info.h"

void Audio_Init(void);

IWRAM_CODE ARM_CODE void Audio_Swap_Buffers(void);
IWRAM_CODE ARM_CODE void Audio_Mix(void);

void Audio_Enable_Set(int enabled);
int Audio_Enable_Get(void);

void Audio_Song_Play_Force(uint32_t song);
void Audio_Song_Play(uint32_t song);

#endif // AUDIO_H__
