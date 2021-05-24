// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <stdint.h>
#include <stdlib.h>

#include <ugba/ugba.h>

#include "room_game/building_info.h"
#include "room_game/text_messages.h"
#include "simulation/simulation_building_count.h"
#include "simulation/simulation_technology.h"

static int technology_level; // Maxes out at TECH_LEVEL_MAX

void Technology_SetLevel(int level)
{
    technology_level = level;
}

int Technology_GetLevel(void)
{
    return technology_level;
}

// Returns 1 if available, 0 if not
int Technology_IsBuildingAvailable(int building_type)
{
    if (building_type == B_PowerPlantNuclear)
    {
        // Nuclear power plant
        if (technology_level >= TECH_LEVEL_NUCLEAR)
            return 1;
        return 0;
    }
    else if (building_type == B_PowerPlantFusion)
    {
        if (technology_level >= TECH_LEVEL_FUSION)
            return 1;
        return 0;
    }

    // The rest of buildings are always available

    return 1;
}

static void Technology_TryIncrement(void)
{
    // Each year, each university tries to increment the technology level of
    // the city. If a certain building is discovered at the next level, it tries
    // to discover it (with a certain % of it being discovered). If it's
    // discovered, the level increases. If not, try again next year (or next
    // university) and keep the same level.

    // 1. Check if the next level unlocks anything
    // -------------------------------------------

    int new_level = technology_level + 1;

    if ((new_level != TECH_LEVEL_NUCLEAR) && (new_level != TECH_LEVEL_FUSION))
    {
        // No match, just increment the level and exit
        technology_level = new_level;
        return;
    }

    // 2. Try to randomly increase a level if it unlocks something
    // -----------------------------------------------------------

    // 70 / 256 chances of failing to increase

    int r = rand() & 0xFF;
    if (r < 70)
        return;

    technology_level = new_level;

    // 3. If something is unlocked, show a message
    // -------------------------------------------

    if (technology_level == TECH_LEVEL_NUCLEAR)
    {
        MessageQueueAdd(ID_MSG_TECH_NUCLEAR);
    }
    else if (technology_level == TECH_LEVEL_FUSION)
    {
        MessageQueueAdd(ID_MSG_TECH_FUSION);
    }
    else
    {
        UGBA_Assert(0);
    }
}

void Simulation_AdvanceTechnology(void)
{
    // If technology is maxed out, just return now.

    if (technology_level >= TECH_LEVEL_MAX)
        return;

    // Increment technology level once for each university

    building_count_info *info = Simulation_CountBuildingsGet();

    for (uint32_t i = 0; i < info->universities; i++)
        Technology_TryIncrement();
}
