// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <assert.h>
#include <stdint.h>

#include "room_game/building_info.h"
#include "room_game/draw_common.h"
#include "room_game/room_game.h"
#include "room_game/text_messages.h"
#include "room_game/tileset_info.h"
#include "simulation/building_density.h"
#include "simulation/building_count.h"

#define CITY_HAS_STADIUM        (1 << 0)
#define CITY_HAS_UNIVERSITY     (1 << 1)
#define CITY_HAS_MUSEUM         (1 << 2)
#define CITY_HAS_LIBRARY        (1 << 3)
#define CITY_HAS_AIRPORT        (1 << 4)
#define CITY_HAS_PORT           (1 << 5)

// Each flag is set if the city has at least one of that kind of building
static int city_services_flags;

static int city_class; // CLASS_CITY, etc

// Internal variables used to calculate the demmand of RCI zones (and RCI graph)
static uint32_t population_residential;
static uint32_t population_commercial;
static uint32_t population_industrial;
static uint32_t population_other;
static uint32_t population_total;

// Areas in tiles
static uint32_t residential_area_empty;
static uint32_t residential_area_used;
static uint32_t commercial_area_empty;
static uint32_t commercial_area_used;
static uint32_t industrial_area_empty;
static uint32_t industrial_area_used;

// 0-7 (0 = high demand, 3,4 = neutral, 7 = low demand)
// They are stored with an offset of -3 to make 0 the central value
static int graph_value_r;
static int graph_value_c;
static int graph_value_i;

int Simulation_GetCityClass(void)
{
    return city_class;
}

// This only works until the first simulation step
void Simulation_SetCityClass(int type)
{
    city_class = type;
}

const char *Simulation_GetCityClassString(void)
{
    const char *city_class_name[] = {
        [CLASS_VILLAGE]     = "   Village",
        [CLASS_TOWN]        = "      Town",
        [CLASS_CITY]        = "      City",
        [CLASS_METROPOLIS]  = "Metropolis",
        [CLASS_CAPITAL]     = "   Capital",
    };

    return city_class_name[city_class];
}

uint32_t Simulation_GetTotalPopulation(void)
{
    return population_total;
}

void Simulation_GetPopulationRCI(uint32_t *r, uint32_t *c, uint32_t *i)
{
    *r = population_residential;
    *c = population_commercial;
    *i = population_industrial;
}

void Simulation_GetDemandRCI(int *r, int *c, int *i)
{
    *r = graph_value_r;
    *c = graph_value_c;
    *i = graph_value_i;
}

void Simulation_GetRCIAreasTotal(int *r, int *c, int *i)
{
    *r = residential_area_empty + residential_area_used;
    *c = commercial_area_empty + commercial_area_used;
    *i = industrial_area_empty + industrial_area_used;
}

static int Simulation_CalculateDemand(uint32_t used, uint32_t empty)
{
    // Calculate proportion of land used. The more percentage of area is used,
    // the higher the demand!

static_assert((2 * CITY_MAP_HEIGHT * CITY_MAP_WIDTH) < (1 << 16), "Map too big");

    // Get fraction
    uint32_t used_percentage;
    if ((used + empty) > 0)
        used_percentage = ((used << 16) / (used + empty));
    else
        used_percentage = 0;

    if (used_percentage >= (1 << 16))
        used_percentage = (1 << 16) - 1;

    // The result is a 16-bit number, reduce it to 3

    // 0-7 (0 = high demand, 3,4 = neutral, 7 = low demand)
    uint32_t tile_index = 7 - (used_percentage >> 13);

    return tile_index;
}

void Simulation_CalculateRCIDemand(void)
{
    // Clear variables

    residential_area_empty = 0;
    residential_area_used = 0;
    commercial_area_empty = 0;
    commercial_area_used = 0;
    industrial_area_empty = 0;
    industrial_area_used = 0;

    // Calculate area used and free

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t tile, type;
            CityMapGetTypeAndTileUnsafe(i, j, &tile, &type);

            type &= TYPE_MASK;

            if (type == TYPE_RESIDENTIAL)
            {
                if (tile == T_RESIDENTIAL)
                    residential_area_empty++;
                else
                    residential_area_used++;
            }
            else if (type == TYPE_COMMERCIAL)
            {
                if (tile == T_COMMERCIAL)
                    commercial_area_empty++;
                else
                    commercial_area_used++;
            }
            else if (type == TYPE_INDUSTRIAL)
            {
                if (tile == T_INDUSTRIAL)
                    industrial_area_empty++;
                else
                    industrial_area_used++;
            }
            else
            {
                continue;
            }
        }
    }

    // Calculate proportion of land used.

    graph_value_r = Simulation_CalculateDemand(residential_area_used,
                                               residential_area_empty);
    graph_value_c = Simulation_CalculateDemand(commercial_area_used,
                                               commercial_area_empty);
    graph_value_i = Simulation_CalculateDemand(industrial_area_used,
                                               industrial_area_empty);
}

static void Simulation_CalculateCityType(void)
{
#define POPULATION_TOWN         500
#define POPULATION_CITY         1000
#define POPULATION_METROPOLIS   3000
#define POPULATION_CAPITAL      6000

    // Default to village

    city_class = CLASS_VILLAGE;

    // Upgrade to town if the population is big enough

    if (population_total < POPULATION_TOWN)
        return;

    PersistentMessageShow(ID_MSG_CLASS_TOWN);
    city_class = CLASS_TOWN;

    // Upgrade to city if enough population and there are libraries

    if (population_total < POPULATION_CITY)
        return;

    if ((city_services_flags & CITY_HAS_LIBRARY) == 0)
        return;

    PersistentMessageShow(ID_MSG_CLASS_CITY);
    city_class = CLASS_CITY;

    // Upgrade to metropolis if there is enough population and there are
    // stadiums, universities and museums

    if (population_total < POPULATION_METROPOLIS)
        return;

    const int metropolis_flags = CITY_HAS_STADIUM | CITY_HAS_UNIVERSITY |
                                 CITY_HAS_MUSEUM;

    if ((city_services_flags & metropolis_flags) != metropolis_flags)
        return;

    PersistentMessageShow(ID_MSG_CLASS_METROPOLIS);
    city_class = CLASS_METROPOLIS;

    // Upgrade to capital if there is enough population and there are airports
    // and ports

    if (population_total < POPULATION_CAPITAL)
        return;

    const int capital_flags = CITY_HAS_AIRPORT | CITY_HAS_PORT;

    if ((city_services_flags & capital_flags) != capital_flags)
        return;

    PersistentMessageShow(ID_MSG_CLASS_CAPITAL);
    city_class = CLASS_CAPITAL;
}

// The precalculated building count should be available when calling this.
void Simulation_CalculateStatistics(void)
{
    // Set city flags
    // --------------

    // Each flag is set if the city has at least one of that kind of building
    city_services_flags = 0;

    building_count_info *info = Simulation_CountBuildingsGet();

    if (info->stadiums > 0)
        city_services_flags |= CITY_HAS_STADIUM;
    if (info->universities > 0)
        city_services_flags |= CITY_HAS_UNIVERSITY;
    if (info->museums > 0)
        city_services_flags |= CITY_HAS_MUSEUM;
    if (info->libraries > 0)
        city_services_flags |= CITY_HAS_LIBRARY;
    if (info->airports > 0)
        city_services_flags |= CITY_HAS_AIRPORT;
    if (info->ports > 0)
        city_services_flags |= CITY_HAS_PORT;

    // First, add up population (total population and separated by types)
    // ------------------------------------------------------------------

    population_total = 0;
    population_residential = 0;
    population_commercial = 0;
    population_industrial = 0;
    population_other = 0;

    for (int j = 0; j < CITY_MAP_HEIGHT; j++)
    {
        for (int i = 0; i < CITY_MAP_WIDTH; i++)
        {
            uint16_t tile, type;
            CityMapGetTypeAndTileUnsafe(i, j, &tile, &type);

            type &= TYPE_MASK;

            if ((type == TYPE_FIELD) || (type == TYPE_FOREST) ||
                (type == TYPE_WATER) || (type == TYPE_DOCK))
            {
                continue;
            }

            // Not the origin of the building, already handled
            if (BuildingIsCoordinateOrigin(tile) == 0)
                continue;

            // Add population to the corresponding type variable

            const city_tile_density_info *di = CityTileDensityInfo(tile);

            switch (type)
            {
                case TYPE_RESIDENTIAL:
                    population_residential += di->population;
                    break;
                case TYPE_INDUSTRIAL:
                    population_industrial += di->population;
                    break;
                case TYPE_COMMERCIAL:
                    population_commercial += di->population;
                    break;

                case TYPE_POLICE_DEPT:
                case TYPE_FIRE_DEPT:
                case TYPE_HOSPITAL:
                case TYPE_PARK:
                case TYPE_STADIUM:
                case TYPE_SCHOOL:
                case TYPE_HIGH_SCHOOL:
                case TYPE_UNIVERSITY:
                case TYPE_MUSEUM:
                case TYPE_LIBRARY:
                case TYPE_AIRPORT:
                case TYPE_PORT:
                case TYPE_POWER_PLANT:
                case TYPE_RADIATION:
                    population_other += di->population;
                    break;

                case TYPE_FIELD:
                case TYPE_FOREST:
                case TYPE_WATER:
                case TYPE_DOCK:
                case TYPE_FIRE:
                default:
                    UGBA_Assert(0);
                    break;
            }

            // Add population to the global population variable

            population_total += di->population;
        }
    }

    // Save city type to variable
    // --------------------------

    Simulation_CalculateCityType();
}

// Returns 1 if available (city is big enough), 0 if not. This doesn't consider
// the technology needed for it to exist.
int CityStats_IsBuildingAvailable(int building_type)
{
    // Buildings that require a city
    if ((building_type == B_Stadium) || (building_type == B_Port) ||
        (building_type == B_Airport))
    {
        if (city_class >= CLASS_CITY)
            return 1;
        return 0;
    }

    // The rest of buildings are always available

    return 1;
}
