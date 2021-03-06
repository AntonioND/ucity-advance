// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <stdint.h>

#include "date.h"
#include "money.h"
#include "room_game/draw_common.h"
#include "room_game/room_game.h"
#include "room_game/text_messages.h"
#include "room_game/tileset_info.h"
#include "room_graphs/graphs_handler.h"
#include "simulation/budget.h"
#include "simulation/calculate_stats.h"
#include "simulation/common.h"
#include "simulation/create_buildings.h"
#include "simulation/fire.h"
#include "simulation/meltdown.h"
#include "simulation/pollution.h"
#include "simulation/power.h"
#include "simulation/services.h"
#include "simulation/technology.h"
#include "simulation/traffic.h"

//EWRAM_BSS
static uint8_t type_matrix[CITY_MAP_WIDTH * CITY_MAP_HEIGHT];

// ----------------------------------------------------------------------------

static int disasters_enabled;

void Simulation_DisastersSetEnabled(int enable)
{
    disasters_enabled = enable;
}

int Simulation_AreDisastersEnabled(void)
{
    return disasters_enabled;
}

static requested_disaster_type requested_disaster;

void Simulation_RequestDisaster(requested_disaster_type type)
{
    if (requested_disaster != REQUESTED_DISASTER_NONE)
        return;

    requested_disaster = type;
}

// ----------------------------------------------------------------------------

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

static void GraphHandleRecords(void)
{
    Graph_Add_Record(Graph_Get(GRAPH_INFO_POPULATION),
                     Simulation_GetTotalPopulation());

    uint32_t r, c, i;
    Simulation_GetPopulationRCI(&r, &c, &i);
    Graph_Add_Record(Graph_Get(GRAPH_INFO_RESIDENTIAL), r);
    Graph_Add_Record(Graph_Get(GRAPH_INFO_COMMERCIAL), c);
    Graph_Add_Record(Graph_Get(GRAPH_INFO_INDUSTRIAL), i);

    Graph_Add_Record(Graph_Get(GRAPH_INFO_FUNDS), MoneyGet());
}

void Simulation_GraphsResetAll(void)
{
    Graph_Reset(Graph_Get(GRAPH_INFO_POPULATION));

    Graph_Reset(Graph_Get(GRAPH_INFO_RESIDENTIAL));
    Graph_Reset(Graph_Get(GRAPH_INFO_COMMERCIAL));
    Graph_Reset(Graph_Get(GRAPH_INFO_INDUSTRIAL));

    Graph_Reset(Graph_Get(GRAPH_INFO_FUNDS));
}

void Simulation_SimulateAll(void)
{
    // Simulate disasters if in disaster mode

    if (Room_Game_IsInDisasterMode())
    {
        Simulation_Fire();
        return;
    }

    // First, get data from last frame and build new buildings or destroy
    // them (if there haven't been changes since the previous step!)
    // depending on the tile ok flags map. In the first iteration the
    // build/demolish flags are being initialized, so don't call it.

    if (first_simulation_iteration == 0)
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

    int city_class = Simulation_GetCityClass();

    if (city_class >= CLASS_VILLAGE)
    {
        // Ignore if the city is too small

        Simulation_Services(T_FIRE_DEPT_CENTER);
        Simulation_ServicesAddTileOkFlag();

        Simulation_Services(T_HOSPITAL_CENTER);
        Simulation_ServicesAddTileOkFlag();
    }

    Simulation_Services(T_SCHOOL_CENTER);
    Simulation_EducationSetTileOkFlag();

    if (city_class >= CLASS_VILLAGE)
    {
        Simulation_ServicesBig(T_HIGH_SCHOOL_CENTER);
        Simulation_EducationAddTileOkFlag();
    }

    // After simulating traffic, power, etc, simulate pollution

    Simulation_Pollution();

    // After simulating, flag buildings to be created or demolished.

    Simulation_FlagCreateBuildings();

    // Calculate total population and other statistics

    Simulation_CalculateStatistics();

    // Calculate RCI graph

    Simulation_CalculateRCIDemand();

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

            Simulation_AdvanceTechnology();

            // Reset yearly messages

            PersistentYearlyMessagesReset();
        }

        // Check if this is the start of a quarter (January, April, July or
        // October).
        if ((month == 0) || (month == 3) || (month == 6) || (month == 9))
        {
            // Handle events that only happen once per quarter
            // (when Dec -> Jan, Mar -> Apr, Jun -> Jul, Sep -> Oct)

            // Calculate and apply budget

            Simulation_CalculateBudgetAndTaxes();
            Simulation_ApplyBudgetAndTaxes();
        }
    }

    // Start disasters if there isn't one active

    int force_fire = (requested_disaster == REQUESTED_DISASTER_FIRE);
    int force_meltdown = (requested_disaster == REQUESTED_DISASTER_MELTDOWN);

    if (force_fire || force_meltdown || disasters_enabled)
    {
        Simulation_FireTryStart(force_fire);
        Simulation_MeltdownTryStart(force_meltdown);

        requested_disaster = REQUESTED_DISASTER_NONE;
    }

    // Remove radiation

    Simulation_Radiation();

    // Handle historical records

    GraphHandleRecords();

    // End of this simulation step
}
