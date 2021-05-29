// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <ugba/ugba.h>

#include "input_utils.h"
#include "main.h"
#include "money.h"
#include "text_utils.h"
#include "room_game/status_bar.h"

// Assets

#include "maps/bank_offer_menu_bg_bin.h"
#include "maps/bank_repay_menu_bg_bin.h"
#include "maps/minimap_frame_palette_bin.h"
#include "maps/minimap_frame_tiles_bin.h"

#define BG_BANK_PALETTE                 (0)
#define BG_BANK_TILES_BASE              MEM_BG_TILES_BLOCK_ADDR(3)
#define BG_BANK_MAP_BASE                MEM_BG_MAP_BLOCK_ADDR(28)

static int loan_remaining_payments; // 0 if no remaining payments (no loan)
static int loan_payments_amount;

static int selected_loan = 0;

void Room_Bank_Set_Loan(int payments, int amount)
{
    loan_remaining_payments = payments;
    loan_payments_amount = amount;
}

void Room_Bank_Get_Loan(int *payments, int *amount)
{
    *payments = loan_remaining_payments;
    if (loan_remaining_payments == 0)
        *amount = 0;
    else
        *amount = loan_payments_amount;
}

static void Room_Bank_DrawCursor(void)
{
    int index;
    uint16_t *dst = (void *)BG_BANK_MAP_BASE;

    uint16_t tile_clear = ' ';
    uint16_t tile_cursor = 138; // TODO: Replace magic number

    int y0 = 7;
    int y1 = 13;

    if (selected_loan == 0)
    {
        index = 32 * y0 + 3;
        dst[index] =  MAP_REGULAR_TILE(tile_cursor) | MAP_REGULAR_PALETTE(BG_BANK_PALETTE);
        index = 32 * y1 + 3;
        dst[index] =  MAP_REGULAR_TILE(tile_clear) | MAP_REGULAR_PALETTE(BG_BANK_PALETTE);
    }
    else
    {
        index = 32 * y0 + 3;
        dst[index] =  MAP_REGULAR_TILE(tile_clear) | MAP_REGULAR_PALETTE(BG_BANK_PALETTE);
        index = 32 * y1 + 3;
        dst[index] =  MAP_REGULAR_TILE(tile_cursor) | MAP_REGULAR_PALETTE(BG_BANK_PALETTE);
    }
}

static void Room_Bank_Print(int x, int y, const char *text)
{
    uintptr_t addr = BG_BANK_MAP_BASE + (y * 32 + x) * 2;

    while (1)
    {
        int c = (uint8_t)*text++;
        if (c == '\0')
            break;

        uint16_t *ptr = (uint16_t *)addr;

        *ptr = MAP_REGULAR_TILE(c) | MAP_REGULAR_PALETTE(BG_BANK_PALETTE);

        addr += 2;
    }
}

static void Room_Bank_Draw(void)
{
    if (loan_remaining_payments > 0)
    {
        // There is a pre-existing loan

        char str[31];

        int total = loan_payments_amount * loan_remaining_payments;

        Print_Integer_Decimal_Right(str, 11, loan_remaining_payments);
        Room_Bank_Print(16, 12, str);
        Print_Integer_Decimal_Right(str, 11, loan_payments_amount);
        Room_Bank_Print(16, 13, str);
        Print_Integer_Decimal_Right(str, 11, total);
        Room_Bank_Print(16, 14, str);
    }
    else
    {
        // No pre-existing loan

        Room_Bank_DrawCursor();
    }
}

void Room_Bank_Load(void)
{
    // Load frame map
    // --------------

    // Load the tiles
    SWI_CpuSet_Copy16(minimap_frame_tiles_bin, (void *)BG_BANK_TILES_BASE,
                      minimap_frame_tiles_bin_size);

    // Load the map
    if (loan_remaining_payments > 0)
    {
        SWI_CpuSet_Copy16(bank_repay_menu_bg_bin, (void *)BG_BANK_MAP_BASE,
                          bank_repay_menu_bg_bin_size);
    }
    else
    {
        SWI_CpuSet_Copy16(bank_offer_menu_bg_bin, (void *)BG_BANK_MAP_BASE,
                          bank_offer_menu_bg_bin_size);
    }

    // Setup background
    BG_RegularInit(1, BG_REGULAR_256x256, BG_16_COLORS,
                   BG_BANK_TILES_BASE, BG_BANK_MAP_BASE);
    BG_RegularScrollSet(1, 0, 0);

    // Update room state
    // -----------------

    // Update map

    Room_Bank_Draw();

    // Setup display mode

    DISP_ModeSet(1);
    DISP_LayersEnable(0, 1, 0, 0, 0);

    // Load palettes
    // -------------

    // Load frame palettes
    SWI_CpuSet_Copy16(minimap_frame_palette_bin, &MEM_PALETTE_BG[BG_BANK_PALETTE],
                      minimap_frame_palette_bin_size);

    MEM_PALETTE_BG[0] = RGB15(31, 31, 31);

    // Initialize state
    // ----------------

    selected_loan = 0;

    Room_Bank_Draw();
}

void Room_Bank_Unload(void)
{
    Game_Clear_Screen();
}

void Room_Bank_Handle(void)
{
    uint16_t keys_pressed = KEYS_Pressed();

    if (loan_remaining_payments == 0)
    {
        if (keys_pressed & KEY_B)
        {
            // Exit without loan
            Game_Room_Prepare_Switch(ROOM_GAME);
            return;
        }
        else if (keys_pressed & KEY_A)
        {
            // Exit and save the new percentage
            if (selected_loan == 0)
            {
                MoneyAdd(10000);
                Room_Bank_Set_Loan(21, 500);
            }
            else
            {
                MoneyAdd(20000);
                Room_Bank_Set_Loan(21, 1000);
            }
            Game_Room_Prepare_Switch(ROOM_GAME);
            return;
        }

        if (Key_Autorepeat_Pressed_Up() || Key_Autorepeat_Pressed_Down())
        {
            selected_loan ^= 1;
            Room_Bank_DrawCursor();
        }
    }
    else
    {
        if (keys_pressed & KEY_B)
        {
            Game_Room_Prepare_Switch(ROOM_GAME);
            return;
        }
    }
}
