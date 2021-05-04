// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <ugba/ugba.h>

#include <umod/umod.h>

#include "date.h"
#include "input_utils.h"
#include "main.h"
#include "room_game/room_game.h"
#include "room_minimap/room_minimap.h"

// Assets

#include "audio/umod_pack_bin.h"
#include "audio/umod_pack_info.h"
#include "maps/test_map.h"

static int current_room;

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

IWRAM_CODE ARM_CODE void Master_VBL_Handler(void)
{
    // The buffer swap needs to be done right at the beginning of the VBL
    // interrupt handler so that the timing is always the same in each frame.

    if (current_dma_buffer == 0)
        SOUND_DMA_Retrigger_AB();

    if (current_room == ROOM_GAME)
        Room_Game_FastVBLHandler();

    REG_IME = 1;

    if (current_dma_buffer == 1)
        UMOD_Mix(wave_a, wave_b, BUFFER_SIZE);
    else
        UMOD_Mix(&wave_a[BUFFER_SIZE], &wave_b[BUFFER_SIZE], BUFFER_SIZE);

    current_dma_buffer ^= 1;

    if (current_room == ROOM_GAME)
        Room_Game_SlowVBLHandler();
}

void Sound_Init(void)
{
    // Start music
    // ===========

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

void Game_Clear_Screen(void)
{
    DISP_LayersEnable(0, 0, 0, 0, 0);

    uint32_t zero = 0;

    SWI_CpuSet_Fill32(&zero, (void *)MEM_PALETTE, MEM_PALETTE_SIZE);

    SWI_CpuSet_Fill32(&zero, (void *)MEM_OAM, MEM_OAM_SIZE);

    for (int i = 0; i < 128; i++)
    {
        OBJ_RegularInit(i, 0, 200, OBJ_SIZE_8x8, OBJ_16_COLORS, 0, 0);
        OBJ_RegularEnableSet(i, 0);
    }

    //SWI_CpuSet_Fill32(&zero, (void *)MEM_VRAM, MEM_VRAM_SIZE);
}

void Game_Room_Load(int room)
{
    Game_Clear_Screen();

    switch (room)
    {
        case ROOM_GAME:
            Room_Game_Load();
            break;

        case ROOM_MINIMAP:
            Room_Minimap_Load();
            break;

        default:
            UGBA_Assert(0);
            return;
    }

    current_room = room;
}

void Game_Room_Handle_Current(void)
{
    switch (current_room)
    {
        case ROOM_GAME:
            Room_Game_Handle();
            break;

        case ROOM_MINIMAP:
            Room_Minimap_Handle();
            break;

        default:
            UGBA_Assert(0);
            return;
    }
}

int main(int argc, char *argv[])
{
    UGBA_Init(&argc, &argv);

    IRQ_SetHandler(IRQ_VBLANK, Master_VBL_Handler);
    IRQ_Enable(IRQ_VBLANK);

    Game_Clear_Screen();

    Sound_Init();

    //UMOD_Song_Play(SONG_KAOS_OCH_DEKADENS_MOD);

    DateReset();

    Load_City_Data(test_map_map, 9, 9);

    Game_Room_Load(ROOM_GAME);

    while (1)
    {
        SWI_VBlankIntrWait();

        KEYS_Update();
        Key_Autorepeat_Update();

        Game_Room_Handle_Current();
    }

    return 0;
}
