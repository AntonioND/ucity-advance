// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <ugba/ugba.h>

#include <umod/umod.h>

#include "date.h"
#include "input_utils.h"
#include "main.h"
#include "money.h"
#include "room_bank/room_bank.h"
#include "room_budget/room_budget.h"
#include "room_city_stats/room_city_stats.h"
#include "room_game/room_game.h"
#include "room_gen_map/room_gen_map.h"
#include "room_input/room_input.h"
#include "room_main_menu/room_main_menu.h"
#include "room_minimap/room_minimap.h"
#include "room_scenarios/room_scenarios.h"

// Assets

#include "audio/umod_pack_bin.h"
#include "audio/umod_pack_info.h"

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

    // TODO: Clear all VRAM except for the city map
    //SWI_CpuSet_Fill32(&zero, (void *)MEM_VRAM, MEM_VRAM_SIZE);
}

// ----------------------------------------------------------------------------

static room_type next_room = ROOM_INVALID;
static room_type current_room = ROOM_INVALID;

#define SWITCH_IDLE             0
#define SWITCH_LEAVING_ROOM     1
#define SWITCH_ENTERING_ROOM    2

#define SWITCH_COUNT_FRAMES     10

static int switch_mode = SWITCH_IDLE;
static int switch_countup;

static void Game_Room_Unload(int room)
{
    switch (room)
    {
        case ROOM_GAME:
            Room_Game_Unload();
            break;
        case ROOM_MINIMAP:
            Room_Minimap_Unload();
            break;
        case ROOM_BANK:
            Room_Bank_Unload();
            break;
        case ROOM_BUDGET:
            Room_Budget_Unload();
            break;
        case ROOM_CITY_STATS:
            Room_City_Stats_Unload();
            break;
        case ROOM_INPUT:
            Room_Input_Unload();
            break;
        case ROOM_GENERATE_MAP:
            Room_Generate_Map_Unload();
            break;
        case ROOM_MAIN_MENU:
            Room_Main_Menu_Unload();
            break;
        case ROOM_SCENARIOS:
            Room_Scenarios_Unload();
            break;
        default:
            UGBA_Assert(0);
            return;
    }
}

static void Game_Room_Load(int room)
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
        case ROOM_BANK:
            Room_Bank_Load();
            break;
        case ROOM_BUDGET:
            Room_Budget_Load();
            break;
        case ROOM_CITY_STATS:
            Room_City_Stats_Load();
            break;
        case ROOM_INPUT:
            Room_Input_Load();
            break;
        case ROOM_GENERATE_MAP:
            Room_Generate_Map_Load();
            break;
        case ROOM_MAIN_MENU:
            Room_Main_Menu_Load();
            break;
        case ROOM_SCENARIOS:
            Room_Scenarios_Load();
            break;
        default:
            UGBA_Assert(0);
            return;
    }

    current_room = room;
    next_room = room;
}

static int Game_Room_HasToSwitch(void)
{
    if (next_room == current_room)
        return 0;

    if (switch_mode != SWITCH_LEAVING_ROOM)
        return 0;

    if (switch_countup < SWITCH_COUNT_FRAMES)
        return 0;

    return 1;
}

static void Game_Room_Update_Switch(void)
{
    const uint16_t first_target = BLDCNT_1ST_BG0 | BLDCNT_1ST_BG1 |
                                  BLDCNT_1ST_BG2 | BLDCNT_1ST_BG3 |
                                  BLDCNT_1ST_OBJ | BLDCNT_1ST_BD;
    switch (switch_mode)
    {
        case SWITCH_IDLE:
            // Do nothing
            break;
        case SWITCH_LEAVING_ROOM:
        {
            switch_countup += 4;

            if (switch_countup > 16)
                DISP_BlendYSet(16);
            else
                DISP_BlendYSet(switch_countup);

            DISP_BlendSetup(first_target, 0, BLDCNT_BRIGTHNESS_DECREASE);
            break;
        }
        case SWITCH_ENTERING_ROOM:
        {
            switch_countup += 4;

            if (switch_countup > 16)
            {
                DISP_BlendSetup(0, 0, BLDCNT_DISABLE);
                switch_mode = SWITCH_IDLE;
            }
            else
            {
                DISP_BlendSetup(first_target, 0, BLDCNT_BRIGTHNESS_DECREASE);
                DISP_BlendYSet(16 - switch_countup);
            }
            break;
        }
        default:
            UGBA_Assert(0);
            break;
    }
}

void Game_Room_Prepare_Switch(room_type new_room)
{
    // If another change has been requested, skip this new change
    if (switch_mode != SWITCH_IDLE)
        return;

    switch_mode = SWITCH_LEAVING_ROOM;
    switch_countup = 0;

    UGBA_Assert(new_room != current_room);

    next_room = new_room;
}

static void Game_Room_DoSwitch(void)
{
    room_type unload_room = current_room;
    room_type load_room = next_room;

    // Make sure that the interrupt handler of this room isn't called again
    current_room = ROOM_INVALID;
    next_room = ROOM_INVALID;

    // Unload room
    Game_Room_Unload(unload_room);

    // Make sure that the next room doesn't see any of the presses/releases from
    // the previous room.
    KEYS_Update();
    KEYS_Update();

    // Update state machine of switching rooms
    switch_mode = SWITCH_ENTERING_ROOM;
    switch_countup = 0;

    // Load room
    Game_Room_Load(load_room);

    // Setup room handlers
    current_room = load_room;
    next_room = load_room;
}

static void Game_Room_Handle_Current(void)
{
    switch (current_room)
    {
        case ROOM_GAME:
            Room_Game_Handle();
            break;
        case ROOM_MINIMAP:
            Room_Minimap_Handle();
            break;
        case ROOM_BANK:
            Room_Bank_Handle();
            break;
        case ROOM_BUDGET:
            Room_Budget_Handle();
            break;
        case ROOM_CITY_STATS:
            Room_City_Stats_Handle();
            break;
        case ROOM_INPUT:
            Room_Input_Handle();
            break;
        case ROOM_GENERATE_MAP:
            Room_Generate_Map_Handle();
            break;
        case ROOM_MAIN_MENU:
            Room_Main_Menu_Handle();
            break;
        case ROOM_SCENARIOS:
            Room_Scenarios_Handle();
            break;
        default:
            UGBA_Assert(0);
            return;
    }
}

// ----------------------------------------------------------------------------

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

static int nested_vbl_handler = 0;

IWRAM_CODE ARM_CODE void Master_VBL_Handler(void)
{
    // The buffer swap needs to be done right at the beginning of the VBL
    // interrupt handler so that the timing is always the same in each frame.

    if (current_dma_buffer == 0)
        SOUND_DMA_Retrigger_AB();

    // This key update can be asynchronous from the other update

    Key_Autorepeat_Update();

    // Do critical animations

    if (current_room == ROOM_GAME)
        Room_Game_FastVBLHandler();

    Game_Room_Update_Switch();

    // Now that the fast parts of the handler are done, let other (short)
    // interrupts happen if needed.

    REG_IME = 1;

    if (current_dma_buffer == 1)
        UMOD_Mix(wave_a, wave_b, BUFFER_SIZE);
    else
        UMOD_Mix(&wave_a[BUFFER_SIZE], &wave_b[BUFFER_SIZE], BUFFER_SIZE);

    current_dma_buffer ^= 1;

    // All the code from this point can take over a frame to end. In order not
    // to nest multiple long calls and get locked, the following part of the
    // handler can only be run if it isn't being run already.

    if (nested_vbl_handler)
        return;

    nested_vbl_handler = 1;

    // Update keys here. Input handling should be done in the slow VBL handler.

    KEYS_Update();

    // Handle things that can take longer than a frame

    if (current_room == ROOM_GAME)
        Room_Game_SlowVBLHandler();

    // Let the next VBL handler do all the work again

    nested_vbl_handler = 0;
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

int main(int argc, char *argv[])
{
    UGBA_Init(&argc, &argv);

    IRQ_SetHandler(IRQ_VBLANK, Master_VBL_Handler);
    IRQ_Enable(IRQ_VBLANK);

    Game_Clear_Screen();

    Sound_Init();

    //UMOD_Song_Play(SONG_KAOS_OCH_DEKADENS_MOD);

    Game_Room_Load(ROOM_MAIN_MENU);

    while (1)
    {
        SWI_VBlankIntrWait();

        Game_Room_Handle_Current();

        if (Game_Room_HasToSwitch())
            Game_Room_DoSwitch();
    }

    return 0;
}
