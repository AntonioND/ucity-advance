// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef ROOM_GEN_MAP_GENERATE_MAP_CIRCLE_H__
#define ROOM_GEN_MAP_GENERATE_MAP_CIRCLE_H__

#include <stdint.h>

// The following array is organized like a quarter of a circle:
//
// (0,0)
// +-----------+
// |.......... |
// |.......... |
// |.........  |
// |........   |
// |.....      |
// |           |
// +-----------+
//        (63,63)
//
// The right and bottom bounds should have only 0s so that a read clamping the
// values to 0,63 will still result in anything above 62 being empty.

const uint8_t *GenMapCircleGet(int radius);

#endif // ROOM_GEN_MAP_GENERATE_MAP_CIRCLE_H__
