// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <ugba/ugba.h>

#include "input_utils.h"
#include "main.h"
#include "text_utils.h"
#include "room_bank/room_bank.h"
#include "simulation/simulation_calculate_stats.h"
#include "simulation/simulation_money.h"

// Assets

#include "maps/budget_menu_bg_bin.h"
#include "maps/minimap_frame_palette_bin.h"
#include "maps/minimap_frame_tiles_bin.h"

#define TAX_PERCENTAGE_MAX              20

#define BG_BUDGET_PALETTE               (0)
#define BG_BUDGET_TILES_BASE            MEM_BG_TILES_BLOCK_ADDR(3)
#define BG_BUDGET_MAP_BASE              MEM_BG_MAP_BLOCK_ADDR(28)

static int original_tax_percentage;

static void Room_Budget_Print(int x, int y, const char *text)
{
    uintptr_t addr = BG_BUDGET_MAP_BASE + (y * 32 + x) * 2;

    while (1)
    {
        int c = (uint8_t)*text++;
        if (c == '\0')
            break;

        uint16_t *ptr = (uint16_t *)addr;

        *ptr = MAP_REGULAR_TILE(c) | MAP_REGULAR_PALETTE(BG_BUDGET_PALETTE);

        addr += 2;
    }
}

static void Room_Budget_Draw(void)
{
    char str[31];

    Print_Integer_Decimal_Right(str, 3, Simulation_TaxPercentageGet());
    Room_Budget_Print(23, 4, str);

    budget_info *budget = Simulation_BudgetGet();

    Print_Integer_Decimal_Right(str, 11, budget->taxes_residential);
    Room_Budget_Print(18, 5, str);
    Print_Integer_Decimal_Right(str, 11, budget->taxes_commercial);
    Room_Budget_Print(18, 6, str);
    Print_Integer_Decimal_Right(str, 11, budget->taxes_industrial);
    Room_Budget_Print(18, 7, str);
    Print_Integer_Decimal_Right(str, 11, budget->taxes_other);
    Room_Budget_Print(18, 8, str);

    Print_Integer_Decimal_Right(str, 11, budget->budget_police);
    Room_Budget_Print(18, 11, str);
    Print_Integer_Decimal_Right(str, 11, budget->budget_firemen);
    Room_Budget_Print(18, 12, str);
    Print_Integer_Decimal_Right(str, 11, budget->budget_healthcare);
    Room_Budget_Print(18, 13, str);
    Print_Integer_Decimal_Right(str, 11, budget->budget_education);
    Room_Budget_Print(18, 14, str);
    Print_Integer_Decimal_Right(str, 11, budget->budget_transport);
    Room_Budget_Print(18, 15, str);
    // Loans
    int payments, amount;
    Room_Bank_Get_Loan(&payments, &amount);
    Print_Integer_Decimal_Right(str, 11, amount);
    Room_Budget_Print(18, 16, str);

    Print_Integer_Decimal_Right(str, 11, budget->budget_result);
    Room_Budget_Print(18, 18, str);
}

void Room_Budget_Load(void)
{
    // Load frame map
    // --------------

    // Load the tiles
    SWI_CpuSet_Copy16(minimap_frame_tiles_bin, (void *)BG_BUDGET_TILES_BASE,
                      minimap_frame_tiles_bin_size);

    // Load the map
    SWI_CpuSet_Copy16(budget_menu_bg_bin, (void *)BG_BUDGET_MAP_BASE,
                      budget_menu_bg_bin_size);

    // Setup background
    BG_RegularInit(1, BG_REGULAR_256x256, BG_16_COLORS,
                   BG_BUDGET_TILES_BASE, BG_BUDGET_MAP_BASE);
    BG_RegularScrollSet(1, 0, 0);

    // Update room state
    // -----------------

    // Refresh information. This is only calculated once every quarter, so it
    // may not be up to date.

    Simulation_CalculateBudgetAndTaxes();

    // Update map

    Room_Budget_Draw();

    // Setup display mode

    DISP_ModeSet(1);
    DISP_LayersEnable(0, 1, 0, 0, 0);

    // Load palettes
    // -------------

    // Load frame palettes
    SWI_CpuSet_Copy16(minimap_frame_palette_bin, &MEM_PALETTE_BG[BG_BUDGET_PALETTE],
                      minimap_frame_palette_bin_size);

    MEM_PALETTE_BG[0] = RGB15(31, 31, 31);

    // Initialize state
    // ----------------

    original_tax_percentage = Simulation_TaxPercentageGet();
}

void Room_Budget_Unload(void)
{
    Game_Clear_Screen();
}

void Room_Budget_Handle(void)
{
    uint16_t keys_pressed = KEYS_Pressed();

    if (keys_pressed & KEY_B)
    {
        // Restore the value that was there before opening the room
        Simulation_TaxPercentageSet(original_tax_percentage);
        Game_Room_Prepare_Switch(ROOM_GAME);
        return;
    }
    else if (keys_pressed & KEY_A)
    {
        // Exit and save the new percentage
        Game_Room_Prepare_Switch(ROOM_GAME);
        return;
    }

    if (Key_Autorepeat_Pressed_Left())
    {
        int value = Simulation_TaxPercentageGet() - 1;
        if (value >= 0)
        {
            Simulation_TaxPercentageSet(value);
            Simulation_CalculateBudgetAndTaxes();
            Room_Budget_Draw();
        }
    }
    else if (Key_Autorepeat_Pressed_Right())
    {
        int value = Simulation_TaxPercentageGet() + 1;
        if (value <= TAX_PERCENTAGE_MAX)
        {
            Simulation_TaxPercentageSet(value);
            Simulation_CalculateBudgetAndTaxes();
            Room_Budget_Draw();
        }
    }
}
