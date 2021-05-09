// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <stdint.h>

#include "date.h"
#include "room_game/draw_common.h"
#include "room_game/room_game.h"
#include "room_game/tileset_info.h"
#include "simulation/simulation_create_buildings.h"
#include "simulation/simulation_pollution.h"
#include "simulation/simulation_power.h"
#include "simulation/simulation_services.h"
#include "simulation/simulation_traffic.h"

//EWRAM_BSS
static uint8_t type_matrix[CITY_MAP_WIDTH * CITY_MAP_HEIGHT];

void TypeMatrixRefresh(void)
{
    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t type = CityMapGetType(i, j);
            type_matrix[j * CITY_MAP_WIDTH + i] = type;
        }
    }
}

uint8_t *TypeMatrixGet(void)
{
    return &type_matrix[0];
}

static int first_simulation_iteration = 1;

void Simulation_SetFirstStep(void)
{
    first_simulation_iteration = 1;
}

void Simulation_SimulateAll(void)
{
    // First, get data from last frame and build new buildings or destroy
    // them (if there haven't been changes since the previous step!)
    // depending on the tile ok flags map. In the first iteration step the
    // flags should be 0, so this can be called as well.

    Simulation_CreateBuildings();

    // Now, simulate this new map. First, power distribution, as it will be
    // needed for other simulations

    Simulation_PowerDistribution();

    // After knowing the power distribution, the rest of the simulations can
    // be done.

    Simulation_Traffic();

    // Simulate services, like police and firemen. They depend on the power
    // simulation, as they can't work without electricity, so handle this
    // after simulating the power grid.

    Simulation_Services(T_POLICE_DEPT_CENTER);
    Simulation_ServicesSetTileOkFlag();

    if (1) // TODO: if (city_class >= CLASS_VILLAGE)
    {
        // Ignore if the city is too small

        Simulation_Services(T_FIRE_DEPT_CENTER);
        Simulation_ServicesAddTileOkFlag();

        Simulation_Services(T_HOSPITAL_CENTER);
        Simulation_ServicesAddTileOkFlag();
    }

    Simulation_Services(T_SCHOOL_CENTER);
    Simulation_EducationSetTileOkFlag();

    if (1) // TODO: if (city_class >= CLASS_VILLAGE)
    {
        Simulation_ServicesBig(T_SCHOOL_CENTER);
        Simulation_EducationAddTileOkFlag();
    }

    // After simulating traffic, power, etc, simulate pollution

    Simulation_Pollution();

    // After simulating, flag buildings to be created or demolished.

    Simulation_FlagCreateBuildings();

    // Calculate total population and other statistics

    // TODO: Simulation_CalculateStatistics()

    // Calculate RCI graph

    // TODO: Simulation_CalculateRCIDemand()

    // Update date, apply budget, etc.
    // Note: Only if this is not the first iteration step! The first iteration
    // is considered a refresh of the previous state when loading the city.

    if (first_simulation_iteration)
    {
        // Flag as not first iteration for the next one
        first_simulation_iteration = 0;
    }
    else
    {
        // Advance date

        DateStep();

        int month = DateGetMonth();

        if (month == 0)
        {
            // If january has started, handle events that only happen once per
            // year (when Dec -> Jan)

            // Increment technology level

            // TODO : Simulation_AdvanceTechnology();
        }

        // Check if this is the start of a quarter (January, April, July or
        // October).
        if ((month == 0) || (month == 3) || (month == 6) || (month == 9))
        {
            // Handle events that only happen once per quarter
            // (when Dec -> Jan, Mar -> Apr, Jun -> Jul, Sep -> Oct)

            // Calculate and apply budget

            // TODO: Simulation_CalculateBudgetAndTaxes();
            // TODO: Simulation_ApplyBudgetAndTaxes();
        }
    }
}
