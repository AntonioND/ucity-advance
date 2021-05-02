// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2017-2019, 2021, Antonio Niño Díaz

#ifndef ROOM_GAME_ROOM_GAME_H__
#define ROOM_GAME_ROOM_GAME_H__

#include <ugba/ugba.h>

// ----------------------------------------------------------------------------

#define CITY_MAP_PALETTE            (0)
#define CITY_TILES_BASE             MEM_BG_TILES_BLOCK_ADDR(0)
#define CITY_MAP_BASE               MEM_BG_MAP_BLOCK_ADDR(12)

// ----------------------------------------------------------------------------

#define CITY_MAP_WIDTH              64
#define CITY_MAP_HEIGHT             64

// ----------------------------------------------------------------------------

#define TILE_OK_POWER_BIT           0
#define TILE_OK_SERVICES_BIT        1 // Hospitals, police and firemen
#define TILE_OK_EDUCATION_BIT       2
#define TILE_OK_POLLUTION_BIT       3 // Set to 1 if valid pollution level
#define TILE_OK_TRAFFIC_BIT         4 // - Roads: Traffic lower than threshold.
                                      // - Buildings: People from this tile
                                      //   could reach a valid destination.
#define TILE_BUILD_RESTED_BIT       5
#define TILE_DEMOLISH_RESTED_BIT    6
#define TILE_unused_BIT             7

#define TILE_OK_POWER               (1 << TILE_OK_POWER_BIT)
#define TILE_OK_SERVICES            (1 << TILE_OK_SERVICES_BIT)
#define TILE_OK_EDUCATION           (1 << TILE_OK_EDUCATION_BIT)
#define TILE_OK_POLLUTION           (1 << TILE_OK_POLLUTION_BIT)
#define TILE_OK_TRAFFIC             (1 << TILE_OK_TRAFFIC_BIT)

#define TILE_OK_MASK                (TILE_OK_POWER | TILE_OK_SERVICES | \
                                     TILE_OK_EDUCATION | TILE_OK_POLLUTION | \
                                     TILE_OK_TRAFFIC)

// ----------------------------------------------------------------------------

#define DISASTER_TYPE_NONE          0
#define DISASTER_TYPE_FIRE          1
#define DISASTER_TYPE_MELTDOWN      2

// ----------------------------------------------------------------------------

#define GAME_STATE_WATCH            0
#define GAME_STATE_EDIT             1
#define GAME_STATE_WATCH_FAST_MOVE  2
#define GAME_STATE_SELECT_BUILDING  3
#define GAME_STATE_PAUSE_MENU       4
#define GAME_STATE_SHOW_MESSAGE     5

// ----------------------------------------------------------------------------

#define CLASS_VILLAGE               0
#define CLASS_TOWN                  1
#define CLASS_CITY                  2
#define CLASS_METROPOLIS            3
#define CLASS_CAPITAL               4

// ----------------------------------------------------------------------------

// Item type
// ---------

#define TYPE_HAS_ROAD_BIT           7
#define TYPE_HAS_TRAIN_BIT          6
#define TYPE_HAS_POWER_BIT          5

#define TYPE_HAS_ROAD               (1 << TYPE_HAS_ROAD_BIT)
#define TYPE_HAS_TRAIN              (1 << TYPE_HAS_TRAIN_BIT)
#define TYPE_HAS_POWER              (1 << TYPE_HAS_POWER_BIT)

#define TYPE_MASK                   0x1F
#define TYPE_FLAGS_MASK             0xE0

// TYPE_FIELD must be 0 always so that TYPE_HAS_ROAD, TYPE_HAS_TRAIN and
// TYPE_HAS_POWER are always considered to be in TYPE_FIELD. The exceptions are
// bridges, that are a combination of ``TYPE_WATER`` and the corresponding flag.
#define TYPE_FIELD                  0 // This one must be 0.
#define TYPE_FOREST                 1
#define TYPE_WATER                  2
#define TYPE_RESIDENTIAL            3
#define TYPE_INDUSTRIAL             4
#define TYPE_COMMERCIAL             5
#define TYPE_POLICE_DEPT            6
#define TYPE_FIRE_DEPT              7
#define TYPE_HOSPITAL               8
#define TYPE_PARK                   9
#define TYPE_STADIUM                10
#define TYPE_SCHOOL                 11
#define TYPE_HIGH_SCHOOL            12
#define TYPE_UNIVERSITY             13
#define TYPE_MUSEUM                 14
#define TYPE_LIBRARY                15
#define TYPE_AIRPORT                16
#define TYPE_PORT                   17
#define TYPE_DOCK                   18
#define TYPE_POWER_PLANT            19
#define TYPE_FIRE                   20
#define TYPE_RADIATION              21

#define TYPE_NUMBER                 32 // Max. Upper 3 bits used as flags

// ----------------------------------------------------------------------------

void Load_City_Data(const void *map, int scx, int scy);

void Room_Game_Load(void);
void Room_Game_Handle(void);

void BuildModeUpdateStatusBar(void);

// ----------------------------------------------------------------------------

#endif // ROOM_GAME_ROOM_GAME_H__
