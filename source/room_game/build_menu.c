// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2017-2019, 2021, Antonio Niño Díaz

#include <ugba/ugba.h>

#include "input_utils.h"
#include "money.h"
#include "room_game/build_menu.h"
#include "room_game/room_game.h"
#include "room_game/status_bar.h"
#include "room_game/tileset_info.h"
#include "room_game/building_info.h"
#include "simulation/simulation_calculate_stats.h"
#include "simulation/simulation_technology.h"

// Assets

#include "sprites/building_menu_tiles.h"
#include "sprites/building_menu_map.h"

int selected_group = 0;
int selected_group_icon = 0;

typedef  enum {
    Icon_Group_Delete,
    Icon_Group_RCI,
    Icon_Group_RoadTrainPower,
    Icon_Group_PoliceFiremenHospital,
    Icon_Group_ParksAndRecreation,
    Icon_Group_Education,
    Icon_Group_Culture,
    Icon_Group_Transport,
    Icon_Group_PowerFossil,
    Icon_Group_PowerRenewable,

    Icon_Number_Groups,

    Icon_Destroy = Icon_Number_Groups,

    Icon_Residential,
    Icon_Commercial,
    Icon_Industrial,

    Icon_Road,
    Icon_Train,
    Icon_PowerLines,

    Icon_Police,
    Icon_Firemen,
    Icon_Hospital,

    Icon_ParkSmall,
    Icon_ParkBig,
    Icon_Stadium,

    Icon_School,
    Icon_HighSchool,
    Icon_University,

    Icon_Museum,
    Icon_Library,

    Icon_Port,
    Icon_Airport,

    Icon_PowerPlantCoal,
    Icon_PowerPlantOil,
    Icon_PowerPlantNuclear,

    Icon_PowerPlantWind,
    Icon_PowerPlantSolar,
    Icon_PowerPlantFusion,

    // Placeholder for unused spaces in the menu
    Icon_NULL,

} icon_index;

static const int icon_to_building[] = {
    [Icon_Group_Delete] = B_None,
    [Icon_Group_RCI] = B_None,
    [Icon_Group_RoadTrainPower] = B_None,
    [Icon_Group_PoliceFiremenHospital] = B_None,
    [Icon_Group_ParksAndRecreation] = B_None,
    [Icon_Group_Education] = B_None,
    [Icon_Group_Culture] = B_None,
    [Icon_Group_Transport] = B_None,
    [Icon_Group_PowerFossil] = B_None,
    [Icon_Group_PowerRenewable] = B_None,

    [Icon_Destroy] = B_Delete,

    [Icon_Residential] = B_Residential,
    [Icon_Commercial] = B_Commercial,
    [Icon_Industrial] = B_Industrial,

    [Icon_Road] = B_Road,
    [Icon_Train] = B_Train,
    [Icon_PowerLines] = B_PowerLines,

    [Icon_Police] = B_PoliceDept,
    [Icon_Firemen] = B_FireDept,
    [Icon_Hospital] = B_Hospital,

    [Icon_ParkSmall] =  B_ParkSmall,
    [Icon_ParkBig] = B_ParkBig,
    [Icon_Stadium] = B_Stadium,

    [Icon_School] = B_School,
    [Icon_HighSchool] = B_HighSchool,
    [Icon_University] = B_University,

    [Icon_Museum] = B_Museum,
    [Icon_Library] = B_Library,

    [Icon_Port] = B_Port,
    [Icon_Airport] = B_Airport,

    [Icon_PowerPlantCoal] = B_PowerPlantCoal,
    [Icon_PowerPlantOil] = B_PowerPlantOil,
    [Icon_PowerPlantNuclear] = B_PowerPlantNuclear,

    [Icon_PowerPlantWind] = B_PowerPlantWind,
    [Icon_PowerPlantSolar] = B_PowerPlantSolar,
    [Icon_PowerPlantFusion] = B_PowerPlantFusion,

    // Placeholder for unused spaces in the menu
    [Icon_NULL] = B_None,
};

#define Icon_Number_Icons_Per_Groups 3

typedef struct {
    int icon[Icon_Number_Icons_Per_Groups]; // Pad with Icon_NULL
} menu_group;

static const menu_group menu[] = {

    [Icon_Group_Delete] =
        {{ Icon_Destroy, Icon_NULL, Icon_NULL }},

    [Icon_Group_RCI] =
        {{ Icon_Residential, Icon_Commercial, Icon_Industrial }},

    [Icon_Group_RoadTrainPower] =
        {{ Icon_Road, Icon_Train, Icon_PowerLines }},

    [Icon_Group_PoliceFiremenHospital] =
        {{ Icon_Police, Icon_Firemen, Icon_Hospital }},

    [Icon_Group_ParksAndRecreation] =
        {{ Icon_ParkSmall, Icon_ParkBig, Icon_Stadium }},

    [Icon_Group_Education] =
        {{ Icon_School, Icon_HighSchool, Icon_University }},

    [Icon_Group_Culture] =
        {{ Icon_Museum, Icon_Library, Icon_NULL }},

    [Icon_Group_Transport] =
        {{ Icon_Port, Icon_Airport, Icon_NULL }},

    [Icon_Group_PowerFossil] =
        {{ Icon_PowerPlantCoal, Icon_PowerPlantOil, Icon_PowerPlantNuclear }},

    [Icon_Group_PowerRenewable] =
        {{ Icon_PowerPlantWind, Icon_PowerPlantSolar, Icon_PowerPlantFusion }},
};

void BuildIconPlace(int building, int x, int y)
{
    int icon = -1;
    int elements = sizeof(icon_to_building) / sizeof(icon_to_building[0]);
    for (int i = 0; i < elements; i++)
    {
        if (icon_to_building[i] == building)
        {
            icon = i;
            break;
        }
    }

    if (icon == -1)
    {
        OBJ_RegularInit(127, 0, 256, OBJ_SIZE_16x16, OBJ_16_COLORS, 0, 0);
    }
    else
    {
        int icon_base_tile = icon * 4;
        int pal = building_menu_map_map[icon_base_tile] >> 12;

        OBJ_RegularInit(127, x, y, OBJ_SIZE_16x16, OBJ_16_COLORS,
                        pal, icon_base_tile);
        OBJ_PrioritySet(127, 0);
    }
}

void BuildSelectMenuShow(void)
{
    int index = 0;

    int y = 20;
    if (selected_group > (Icon_Number_Groups / 2))
        y -= (selected_group - (Icon_Number_Groups / 2)) * 16;

    if (StatusBarPositionGet() == STATUS_BAR_UP)
        y += 16;

    for (int group = 0; group < Icon_Number_Groups; group++, y += 16)
    {
        int x = 4;

        {
            int icon_base_tile = group * 4;
            int pal = building_menu_map_map[icon_base_tile] >> 12;

            OBJ_RegularInit(index, x, y, OBJ_SIZE_16x16, OBJ_16_COLORS,
                            pal, icon_base_tile);
            OBJ_PrioritySet(index, 2);

            x += 16;
            index++;
        }

        if (group != selected_group)
            continue;

        for (int i = 0; i < Icon_Number_Icons_Per_Groups; i++)
        {
            int icon = menu[group].icon[i];
            if (icon == Icon_NULL)
                continue;

            int building = icon_to_building[icon];

            if ((CityStats_IsBuildingAvailable(building) == 0) ||
                (Technology_IsBuildingAvailable(building) == 0))
                continue;

            int icon_base_tile = menu[group].icon[i] * 4;
            int pal = building_menu_map_map[icon_base_tile] >> 12;

            if (i == selected_group_icon)
            {
                const obj_affine_src objsrc_init[] =
                {
                    { 0.6 * (1 << 8), 0.6 * (1 << 8), 0, 0 }
                };
                SWI_ObjAffineSet_OAM(&objsrc_init[0], MEM_OAM, 1);

                OBJ_AffineInit(index, x - 8, y - 8,
                               OBJ_SIZE_16x16, 0, OBJ_16_COLORS,
                               pal, icon_base_tile, 1);
                OBJ_PrioritySet(index, 1);
                x += 16;
            }
            else
            {
                OBJ_RegularInit(index, x, y, OBJ_SIZE_16x16, OBJ_16_COLORS,
                                pal, icon_base_tile);
                OBJ_PrioritySet(index, 2);
                x += 16;
            }
            index++;
        }
    }

    // Clear a few more sprites in case one of them would be left on the screen.
    // This can happen when moving from a group with many elements to a group
    // with less elements.
    for (int i = 0; i < 5; i++)
    {
        OBJ_RegularInit(index, 0, 200, OBJ_SIZE_8x8, OBJ_16_COLORS, 0, 0);
        OBJ_RegularEnableSet(index, 0);
        index++;
    }
}

void BuildSelectMenuHide(void)
{
    for (int i = 0; i < 128; i++)
    {
        OBJ_RegularInit(i, 0, 200, OBJ_SIZE_8x8, OBJ_16_COLORS, 0, 0);
        OBJ_RegularEnableSet(i, 0);
    }
}

void BuildSelectMenuLoadGfx(void)
{
#define SPRITES_TILES_BASE  MEM_BG_TILES_BLOCK_ADDR(4)

    // Load the palettes
    VRAM_OBJPalette16Copy(building_menu_tiles_pal, building_menu_tiles_pal_size,
                          building_menu_tiles_pal_start);

    // Load the tiles
    SWI_CpuSet_Copy16(building_menu_tiles_tiles, (void *)SPRITES_TILES_BASE,
                      building_menu_tiles_tiles_size);
}

int BuildMenuSelection(void)
{
    int icon = menu[selected_group].icon[selected_group_icon];
    return icon_to_building[icon];
}

void BuildMenuReset(void)
{
    selected_group = 0;
    selected_group_icon = 0;

    BuildModeUpdateStatusBar();
}

void BuildMenuHandleInput(void)
{
    int update = 0;

    if (Key_Autorepeat_Pressed_Up())
    {
        if (selected_group > 0)
        {
            selected_group--;
            selected_group_icon = 0;
            update = 1;
        }
    }
    else if (Key_Autorepeat_Pressed_Down())
    {
        if (selected_group < (Icon_Number_Groups - 1))
        {
            selected_group++;
            selected_group_icon = 0;
            update = 1;
        }
    }

    if (Key_Autorepeat_Pressed_Left())
    {
        if (selected_group_icon > 0)
        {
            selected_group_icon--;
            update = 1;
        }
    }
    else if (Key_Autorepeat_Pressed_Right())
    {
        if (selected_group_icon < (Icon_Number_Icons_Per_Groups - 1))
        {
            int icon = menu[selected_group].icon[selected_group_icon + 1];
            if (icon != Icon_NULL)
            {
                int building = icon_to_building[icon];

                if (CityStats_IsBuildingAvailable(building) &&
                    Technology_IsBuildingAvailable(building))
                {
                    selected_group_icon++;
                    update = 1;
                }
            }
        }
    }

    if (update)
    {
        BuildSelectMenuShow();
        BuildModeUpdateStatusBar();
    }
}
