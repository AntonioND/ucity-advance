// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <ugba/ugba.h>

#include "random.h"
#include "room_game/building_info.h"
#include "room_game/draw_building.h"
#include "room_game/draw_common.h"
#include "room_game/room_game.h"
#include "room_game/tileset_info.h"
#include "simulation/budget.h"
#include "simulation/happiness.h"
#include "simulation/pollution.h"
#include "simulation/traffic.h"

// ----------------------------------------------------------------------------

EWRAM_BSS static uint8_t building_command[CITY_MAP_WIDTH * CITY_MAP_HEIGHT];
EWRAM_BSS static uint8_t building_flags[CITY_MAP_WIDTH * CITY_MAP_HEIGHT];
EWRAM_BSS static uint8_t building_level[CITY_MAP_WIDTH * CITY_MAP_HEIGHT];

// ----------------------------------------------------------------------------

#define COMMAND_DO_NOTHING   (0)
#define COMMAND_BUILD        (1)
#define COMMAND_DEMOLISH     (2)

// ----------------------------------------------------------------------------

// Flags: Power | Services | Education | Pollution | Traffic
#define FPOW        TILE_OK_POWER
#define FSER        TILE_OK_SERVICES
#define FEDU        TILE_OK_EDUCATION
#define FPOL        TILE_OK_POLLUTION
#define FTRA        TILE_OK_TRAFFIC

// The needed flags must be a subset of the desired ones

// TYPE_RESIDENTIAL
#define R_NEEDED    (FPOW | FPOL | FTRA)
#define R_DESIRED   (FPOW | FSER | FEDU | FPOL | FTRA)

// TYPE_COMMERCIAL
#define C_NEEDED    (FPOW | FSER | FPOL | FTRA)
#define C_DESIRED   (FPOW | FPOL | FTRA)

// TYPE_INDUSTRIAL
#define I_NEEDED    (FPOW | FSER | FTRA)
#define I_DESIRED   (FPOW | FTRA)

// ----------------------------------------------------------------------------

// Generate flags to build/demolish buildings on a per-tile basis. Flags are
// only calculated for RCI type zones!
IWRAM_CODE void Simulation_FlagCreateBuildings(void)
{
    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t type = CityMapGetType(i, j);
            int flags = Simulation_HappinessGetFlags(i, j);
            uint8_t result = COMMAND_DO_NOTHING;

            if (type == TYPE_RESIDENTIAL)
            {
                if ((flags & R_DESIRED) == R_DESIRED)
                {
                    // Build
                    result = COMMAND_BUILD;
                }
                else if ((flags & R_NEEDED) == R_NEEDED)
                {
                    // Do nothing
                }
                else
                {
                    // Demolish
                    result = COMMAND_DEMOLISH;
                }
            }
            else if (type == TYPE_COMMERCIAL)
            {
                if ((flags & C_DESIRED) == C_DESIRED)
                {
                    // Build
                    result = COMMAND_BUILD;
                }
                else if ((flags & C_NEEDED) == C_NEEDED)
                {
                    // Do nothing
                }
                else
                {
                    // Demolish
                    result = COMMAND_DEMOLISH;
                }
            }
            else if (type == TYPE_INDUSTRIAL)
            {
                if ((flags & I_DESIRED) == I_DESIRED)
                {
                    // Build
                    result = COMMAND_BUILD;
                }
                else if ((flags & I_NEEDED) == I_NEEDED)
                {
                    // Do nothing
                }
                else
                {
                    // Demolish
                    result = COMMAND_DEMOLISH;
                }
            }

            building_command[j * CITY_MAP_WIDTH + i] = result;
        }
    }
}

// The following is used to verify if a building can actually grow in this place
// taking in consideration if there are more buildings on the way.

// Vertical and horizontal flags separated for easier understanding. The result
// is the addition of both sides:

// T T T   L C R
// C C C   L C R
// B B B   L C R

// TC TC   LC RC
// BC BC   LC RC

// TCB     LCR

// In short, the map is filled with the flags corresponding to the buildings
// that are already there. If a new building wants to grow on a certaing place,
// all the new tiles have to be checked. If the flags corresponding to each one
// of the new tiles ANDed with the ones that are already on SCRATCH_RAM is not 0
// that means that it's not going to cut another building that was there before.
//
// Also, it is needed to check if the new building is bigger than the old one,
// as this check only works when the new building is bigger. For this check,
// SCRATCH_RAM_2 is filled with the size of the building (being 0 for RCI tiles,
// 1 for the 1x1 buildings, 2 for 2x2 and 3 for 3x3).

#define F_TL    (1 << 0) // Top left
#define F_TC    (1 << 1) // Top center
#define F_TR    (1 << 2) // Top right
#define F_CL    (1 << 3) // Center left
#define F_CR    (1 << 4) // Center right
#define F_BL    (1 << 5) // Bottom left
#define F_BC    (1 << 6) // Bottom center
#define F_BR    (1 << 7) // Bottom right

// Center center
#define F_CC    (F_TL | F_TC | F_TR | F_CL | F_CR | F_BL | F_BC | F_BR)

// 0
#define BUILDING1X1FLAGS(base_tile)         \
    [base_tile + 0] = F_CC,

#define BUILDING1X1LEVEL(base_tile)         \
    [base_tile + 0] = 1,

// 0 1
// 2 3
#define BUILDING2X2FLAGS(base_tile)         \
    [base_tile + 0] = F_TL | F_TC | F_CL,   \
    [base_tile + 1] = F_TC | F_TR | F_CR,   \
    [base_tile + 2] = F_CL | F_BL | F_BC,   \
    [base_tile + 3] = F_CR | F_BC | F_BR,

#define BUILDING2X2LEVEL(base_tile)         \
    [base_tile + 0] = 2,                    \
    [base_tile + 1] = 2,                    \
    [base_tile + 2] = 2,                    \
    [base_tile + 3] = 2,

// 0 1 2
// 3 4 5
// 6 7 8
#define BUILDING3X3FLAGS(base_tile)         \
    [base_tile + 0] = F_TL,                 \
    [base_tile + 1] = F_TC,                 \
    [base_tile + 2] = F_TR,                 \
    [base_tile + 3] = F_CL,                 \
    [base_tile + 4] = F_CC,                 \
    [base_tile + 5] = F_CR,                 \
    [base_tile + 6] = F_BL,                 \
    [base_tile + 7] = F_BC,                 \
    [base_tile + 8] = F_BR,

#define BUILDING3X3FLAGS_CLEAR(base_tile)   \
    [base_tile + 0] = 0,                    \
    [base_tile + 1] = 0,                    \
    [base_tile + 2] = 0,                    \
    [base_tile + 3] = 0,                    \
    [base_tile + 4] = 0,                    \
    [base_tile + 5] = 0,                    \
    [base_tile + 6] = 0,                    \
    [base_tile + 7] = 0,                    \
    [base_tile + 8] = 0,

#define BUILDING3X3LEVEL(base_tile)         \
    [base_tile + 0] = 3,                    \
    [base_tile + 1] = 3,                    \
    [base_tile + 2] = 3,                    \
    [base_tile + 3] = 3,                    \
    [base_tile + 4] = 3,                    \
    [base_tile + 5] = 3,                    \
    [base_tile + 6] = 3,                    \
    [base_tile + 7] = 3,                    \
    [base_tile + 8] = 3,

// Input = tile number
static const uint8_t create_building_flags[] = {

    // RCI Tiles

    BUILDING1X1FLAGS(T_RESIDENTIAL)
    BUILDING1X1FLAGS(T_COMMERCIAL)
    BUILDING1X1FLAGS(T_INDUSTRIAL)

    // Residential buildings

    BUILDING1X1FLAGS(T_RESIDENTIAL_S1_A)
    BUILDING1X1FLAGS(T_RESIDENTIAL_S1_B)
    BUILDING1X1FLAGS(T_RESIDENTIAL_S1_C)
    BUILDING1X1FLAGS(T_RESIDENTIAL_S1_D)

    BUILDING2X2FLAGS(T_RESIDENTIAL_S2_A)
    BUILDING2X2FLAGS(T_RESIDENTIAL_S2_B)
    BUILDING2X2FLAGS(T_RESIDENTIAL_S2_C)
    BUILDING2X2FLAGS(T_RESIDENTIAL_S2_D)

    // Not needed to set flags because this is only used to make buildings grow
    // on top of this. A 3x3 building can't be replaced by anything, so save
    // some CPU by setting this to 0 and preventing calculations.
    BUILDING3X3FLAGS_CLEAR(T_RESIDENTIAL_S3_A)
    BUILDING3X3FLAGS_CLEAR(T_RESIDENTIAL_S3_B)
    BUILDING3X3FLAGS_CLEAR(T_RESIDENTIAL_S3_C)
    BUILDING3X3FLAGS_CLEAR(T_RESIDENTIAL_S3_D)

    // Commercial buildings

    BUILDING1X1FLAGS(T_COMMERCIAL_S1_A)
    BUILDING1X1FLAGS(T_COMMERCIAL_S1_B)
    BUILDING1X1FLAGS(T_COMMERCIAL_S1_C)
    BUILDING1X1FLAGS(T_COMMERCIAL_S1_D)

    BUILDING2X2FLAGS(T_COMMERCIAL_S2_A)
    BUILDING2X2FLAGS(T_COMMERCIAL_S2_B)
    BUILDING2X2FLAGS(T_COMMERCIAL_S2_C)
    BUILDING2X2FLAGS(T_COMMERCIAL_S2_D)

    BUILDING3X3FLAGS_CLEAR(T_COMMERCIAL_S3_A)
    BUILDING3X3FLAGS_CLEAR(T_COMMERCIAL_S3_B)
    BUILDING3X3FLAGS_CLEAR(T_COMMERCIAL_S3_C)
    BUILDING3X3FLAGS_CLEAR(T_COMMERCIAL_S3_D)

    // Industrial buildings

    BUILDING1X1FLAGS(T_INDUSTRIAL_S1_A)
    BUILDING1X1FLAGS(T_INDUSTRIAL_S1_B)
    BUILDING1X1FLAGS(T_INDUSTRIAL_S1_C)
    BUILDING1X1FLAGS(T_INDUSTRIAL_S1_D)

    BUILDING2X2FLAGS(T_INDUSTRIAL_S2_A)
    BUILDING2X2FLAGS(T_INDUSTRIAL_S2_B)
    BUILDING2X2FLAGS(T_INDUSTRIAL_S2_C)
    BUILDING2X2FLAGS(T_INDUSTRIAL_S2_D)

    BUILDING3X3FLAGS_CLEAR(T_INDUSTRIAL_S3_A)
    BUILDING3X3FLAGS_CLEAR(T_INDUSTRIAL_S3_B)
    BUILDING3X3FLAGS_CLEAR(T_INDUSTRIAL_S3_C)
    BUILDING3X3FLAGS_CLEAR(T_INDUSTRIAL_S3_D)
};

// Gets the building level. Used to prevent building small buildings on top of
// bigger ones. Level = size
static const uint8_t create_building_level[] = {

    // RCI Tiles

    // Level 0 so that 1x1 buildings can be built on top of them
    [T_RESIDENTIAL] = 0,
    [T_COMMERCIAL] = 0,
    [T_INDUSTRIAL] = 0,

    // Residential buildings

    BUILDING1X1LEVEL(T_RESIDENTIAL_S1_A)
    BUILDING1X1LEVEL(T_RESIDENTIAL_S1_B)
    BUILDING1X1LEVEL(T_RESIDENTIAL_S1_C)
    BUILDING1X1LEVEL(T_RESIDENTIAL_S1_D)

    BUILDING2X2LEVEL(T_RESIDENTIAL_S2_A)
    BUILDING2X2LEVEL(T_RESIDENTIAL_S2_B)
    BUILDING2X2LEVEL(T_RESIDENTIAL_S2_C)
    BUILDING2X2LEVEL(T_RESIDENTIAL_S2_D)

    BUILDING3X3LEVEL(T_RESIDENTIAL_S3_A)
    BUILDING3X3LEVEL(T_RESIDENTIAL_S3_B)
    BUILDING3X3LEVEL(T_RESIDENTIAL_S3_C)
    BUILDING3X3LEVEL(T_RESIDENTIAL_S3_D)

    // Commercial buildings

    BUILDING1X1LEVEL(T_COMMERCIAL_S1_A)
    BUILDING1X1LEVEL(T_COMMERCIAL_S1_B)
    BUILDING1X1LEVEL(T_COMMERCIAL_S1_C)
    BUILDING1X1LEVEL(T_COMMERCIAL_S1_D)

    BUILDING2X2LEVEL(T_COMMERCIAL_S2_A)
    BUILDING2X2LEVEL(T_COMMERCIAL_S2_B)
    BUILDING2X2LEVEL(T_COMMERCIAL_S2_C)
    BUILDING2X2LEVEL(T_COMMERCIAL_S2_D)

    BUILDING3X3LEVEL(T_COMMERCIAL_S3_A)
    BUILDING3X3LEVEL(T_COMMERCIAL_S3_B)
    BUILDING3X3LEVEL(T_COMMERCIAL_S3_C)
    BUILDING3X3LEVEL(T_COMMERCIAL_S3_D)

    // Industrial buildings

    BUILDING1X1LEVEL(T_INDUSTRIAL_S1_A)
    BUILDING1X1LEVEL(T_INDUSTRIAL_S1_B)
    BUILDING1X1LEVEL(T_INDUSTRIAL_S1_C)
    BUILDING1X1LEVEL(T_INDUSTRIAL_S1_D)

    BUILDING2X2LEVEL(T_INDUSTRIAL_S2_A)
    BUILDING2X2LEVEL(T_INDUSTRIAL_S2_B)
    BUILDING2X2LEVEL(T_INDUSTRIAL_S2_C)
    BUILDING2X2LEVEL(T_INDUSTRIAL_S2_D)

    BUILDING3X3LEVEL(T_INDUSTRIAL_S3_A)
    BUILDING3X3LEVEL(T_INDUSTRIAL_S3_B)
    BUILDING3X3LEVEL(T_INDUSTRIAL_S3_C)
    BUILDING3X3LEVEL(T_INDUSTRIAL_S3_D)
};

IWRAM_CODE static int PositionTest(int x, int y, uint16_t ref_type,
                                   uint8_t flags, int level)
{
    uint16_t type = CityMapGetType(x, y);

    if (type != ref_type)
        return 0;

    if (building_command[y * CITY_MAP_WIDTH + x] != COMMAND_BUILD)
        return 0;

    // Make sure that the position flags allow us to build here
    if ((building_flags[y * CITY_MAP_WIDTH + x] & flags) == 0)
        return 0;

    // Make sure that the building already present here has a lower level than
    // the one we are trying to build. This will also prevent a building to be
    // built on top of another one of the same size.

    if (building_level[y * CITY_MAP_WIDTH + x] >= level)
        return 0;

    return 1;
}

// Try to build a building as big as possible.
IWRAM_CODE void Simulation_CreateBuildingsTryBuild(int x, int y, uint16_t type)
{
    int building_result;
    int size;

    // The tiles to test are arranged like this (0 is the origin):
    //
    // 0 1 2
    // 3 4 5
    // 6 7 8
    //
    // Check if all tiles are the same type as register C and if they are
    // flagged to build. Any tile outside the map makes the function to fail.
    //
    // 1) Start trying with 3x3. Set size to 3x3.
    //
    // 2)
    //    a. Check coordinates to see if 3x3 fits.
    //    b. Check tiles 8, 7, 5, 6, 2. If any of them fails, fall back to 2x2.
    //
    // 3)
    //    a. Check coordinates to see if 2x2 fits.
    //    b. Check tiles 4, 3, 1. If they fail, fall back to 1x1.
    //
    // 4) Check tile 0 to see if a 1x1 building can be built here
    //
    // 5) Build building.
    //
    // The order of the checks has been chosen to minimize the number of checks
    // in a regular map.

    // 2) a) Check coordinates to see if 3x3 fits
    if ((x <= (CITY_MAP_WIDTH - 3)) && (y <= (CITY_MAP_HEIGHT - 3)))
    {
        // Right column and bottom row

        // Check 8
        if (PositionTest(x + 2, y + 2, type, F_BR, 3) == 0)
            goto end3x3;

        // Check 7
        if (PositionTest(x + 1, y + 2, type, F_BC, 3) == 0)
            goto end3x3;

        // Check 5
        if (PositionTest(x + 2, y + 1, type, F_CR, 3) == 0)
            goto end3x3;

        // Check 6
        if (PositionTest(x + 0, y + 2, type, F_BL, 3) == 0)
            goto end3x3;

        // Check 2
        if (PositionTest(x + 2, y + 0, type, F_TR, 3) == 0)
            goto end3x3;

        // Inner 2x2 square

        // Check 4
        if (PositionTest(x + 1, y + 1, type, F_CC, 3) == 0)
            goto end3x3;

        // Check 3
        if (PositionTest(x + 0, y + 1, type, F_CL, 3) == 0)
            goto end3x3;

        // Check 1
        if (PositionTest(x + 1, y + 0, type, F_TC, 3) == 0)
            goto end3x3;

        // Check 0
        if (PositionTest(x + 0, y + 0, type, F_TL, 3) == 0)
            goto end3x3;

        // Build 3x3 building

        if (type == TYPE_RESIDENTIAL)
        {
            building_result = B_ResidentialS3A;
        }
        else if (type == TYPE_COMMERCIAL)
        {
            building_result = B_CommercialS3A;
        }
        else if (type == TYPE_INDUSTRIAL)
        {
            building_result = B_IndustrialS3A;
        }
        else
        {
            UGBA_Assert(0);
            return;
        }

        size = 3;

        goto build_building;
    }

end3x3:

    // 3) a) Check coordinates to see if 2x2 fits
    if ((x <= (CITY_MAP_WIDTH - 2)) && (y <= (CITY_MAP_HEIGHT - 2)))
    {
        // 3) b) Check 4, 3, 1. If they fail, fall back to 1x1.

        // Check 4
        if (PositionTest(x + 1, y + 1, type, F_CR | F_BC | F_BR, 2) == 0)
            goto end2x2;

        // Check 3
        if (PositionTest(x + 0, y + 1, type, F_CL | F_BL | F_BC, 2) == 0)
            goto end2x2;

        // Check 1
        if (PositionTest(x + 1, y + 0, type, F_TC | F_TR | F_CR, 2) == 0)
            goto end2x2;

        // Check 0
        if (PositionTest(x + 0, y + 0, type, F_TL | F_TC | F_CL, 2) == 0)
            goto end2x2;

        // Build 2x2 building

        if (type == TYPE_RESIDENTIAL)
        {
            building_result = B_ResidentialS2A;
        }
        else if (type == TYPE_COMMERCIAL)
        {
            building_result = B_CommercialS2A;
        }
        else if (type == TYPE_INDUSTRIAL)
        {
            building_result = B_IndustrialS2A;
        }
        else
        {
            UGBA_Assert(0);
            return;
        }

        size = 2;

        goto build_building;
    }

end2x2:

    // 4) Check tile 0 to see if a 1x1 building can be built here

    if (PositionTest(x + 0, y + 0, type, F_CC, 1) != 0)
    {
        // Build 1x1 building

        if (type == TYPE_RESIDENTIAL)
        {
            building_result = B_ResidentialS1A;
        }
        else if (type == TYPE_COMMERCIAL)
        {
            building_result = B_CommercialS1A;
        }
        else if (type == TYPE_INDUSTRIAL)
        {
            building_result = B_IndustrialS1A;
        }
        else
        {
            UGBA_Assert(0);
            return;
        }

        size = 1;

        goto build_building;
    }

    // Failed

    return;

build_building:

    // There are 4 versions of all buildings. Pick one randomly.

    building_result += rand_slow() & 3;

    MapDrawBuilding(1, building_result, x, y);

    for (int j = y; j < (y + size); j++)
    {
        for (int i = x; i < (x + size); i++)
            building_command[j * CITY_MAP_WIDTH + i] = COMMAND_DO_NOTHING;
    }
}

// After calling Simulation_FlagCreateBuildings and calculating the RCI demand in
// Simulation_CalculateStatistics, create and destroy buildings!
//
// The functions used to build and delete buildings will clear the FLAGS in the
// tiles affected by the change, so the loop won't try to handle all tiles after
// one of these changes.
IWRAM_CODE void Simulation_CreateBuildings(void)
{
    // The probability of creating and destroying buildings depend on the amount
    // of taxes.

    int index = Simulation_TaxPercentageGet();

    // Penalize if pollution is high
    // Max value = 255 * 64 * 64 = 0x0FF000
    int pollution_total = Simulation_PollutionGetTotal();
    index += pollution_total >> 17; // 0 to 7

    // Penalize if traffic is high
    int traffic_percent = Simulation_TrafficGetTrafficJamPercent();
    index += traffic_percent >> 4; // 0 to 100 / 16

    // Limit to 20
    if (index > 20)
        index = 20;

    // Higher number = Higher probability (0-255)

    // 0 to 20% taxes - 21 values

    const uint8_t CreateBuildingProbability[21] = {
         0xFF, // No taxes = everyone wants to come!
         0xFF, 0xFE, 0xFE, 0xFB, 0xFB, 0xF8, 0xF8, 0xF3, 0xF3, 0xEE,
         0xEE, 0xE7, 0xE7, 0xE0, 0xE0, 0xD8, 0xD8, 0xD0, 0xD0, 0xC6,
    };

    const uint8_t DemolishBuildingProbability[21] = {
         0x04, // No taxes = nobody wants to leave!
         0x04, 0x05, 0x05, 0x07, 0x07, 0x0A, 0x0A, 0x0F, 0x0F, 0x14,
         0x14, 0x1B, 0x1B, 0x22, 0x22, 0x2A, 0x2A, 0x32, 0x32, 0x3C,
    };

    int create_probability = CreateBuildingProbability[index];
    int demolish_probability = DemolishBuildingProbability[index];

    // Create buildings
    // ----------------

    // First, set a temporary map with information to expand buildings and
    // another one with the building size in order not to build small buildings
    // on top of a big one.
    //
    // Not needed to clear BANK_SCRATCH_RAM_2 because it will only be used if a
    // building is being built in a tile, and to get to that point a few extra
    // checks are needed.

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t tile, type;
            CityMapGetTypeAndTile(i, j, &tile, &type);

            if ((type != TYPE_RESIDENTIAL) && (type != TYPE_COMMERCIAL) &&
                (type != TYPE_INDUSTRIAL))
                continue;

            uint8_t flags = create_building_flags[tile];
            uint8_t level = create_building_level[tile];

            building_flags[j * CITY_MAP_WIDTH + i] = flags;
            building_level[j * CITY_MAP_WIDTH + i] = level;
        }
    }

    // We know that only RCI type tiles can have a build or demolish flag set.
    // If the flag is set to 1, check if we can build. If built, clear the build
    // flag from the modified tiles!

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t type = CityMapGetType(i, j);

            if ((type != TYPE_RESIDENTIAL) && (type != TYPE_COMMERCIAL) &&
                (type != TYPE_INDUSTRIAL))
                continue;

            // This is a RCI tile, check that we got a request to build or
            // demolish.
            //
            // - To build, all tiles must be flagged to build.
            // - Demolish if even one single tile is flagged to demolish.

            int command = building_command[j * CITY_MAP_WIDTH + i];

            if (command == COMMAND_BUILD)
            {
                // Try to build
                uint8_t r = rand_slow();

                if (create_probability > r)
                    Simulation_CreateBuildingsTryBuild(i, j, type);
            }
        }
    }

    // Demolish buildings. Make sure that we are not demolishing a RCI tile!
    // ---------------------------------------------------------------------

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t tile, type;
            CityMapGetTypeAndTile(i, j, &tile, &type);

            // Don't demolish RCI tiles
            if ((tile == T_RESIDENTIAL) || (tile == T_COMMERCIAL) ||
                (tile == T_INDUSTRIAL))
                continue;

            int command = building_command[j * CITY_MAP_WIDTH + i];

            if (command == COMMAND_DEMOLISH)
            {
                // Demolish building
                uint8_t r = rand_slow();

                if (demolish_probability > r)
                {
                    MapDeleteBuilding(1, i, j); // Forced

                    // After demolishing the building all the tiles will be RCI,
                    // so it is not needed to clear the demolish request flag.
                    // Only non-RCI tiles with demolish request flag are
                    // demolished, when demolishing a RCI building the tiles
                    // will go back to RCI tiles, so they won't be demolished
                    // again.
                }
            }
        }
    }

    // Done
    // ----
}
