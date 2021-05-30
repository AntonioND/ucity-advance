// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <umod/umod.h>

// Assets

#include "audio/umod_pack_info.h"

void SFX_Build(void)
{
    UMOD_SFX_Play(SFX_BUILD_WAV, UMOD_LOOP_DEFAULT);
}

void SFX_BuildError(void)
{
    UMOD_SFX_Play(SFX_BUILD_ERROR_WAV, UMOD_LOOP_DEFAULT);
}

void SFX_Clear(void)
{
    UMOD_SFX_Play(SFX_CLEAR_WAV, UMOD_LOOP_DEFAULT);
}

void SFX_Demolish(void)
{
    UMOD_SFX_Play(SFX_DEMOLISH_WAV, UMOD_LOOP_DEFAULT);
}

void SFX_FireExplosion(void)
{
    UMOD_SFX_Play(SFX_FIRE_EXPLOSION_WAV, UMOD_LOOP_DEFAULT);
}

void SFX_WrongSelection(void)
{
    UMOD_SFX_Play(SFX_WRONG_SELECTION_WAV, UMOD_LOOP_DEFAULT);
}
