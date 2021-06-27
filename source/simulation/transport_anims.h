// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#ifndef SIMULATION_TRANSPORT_ANIMS_H__
#define SIMULATION_TRANSPORT_ANIMS_H__

#include <assert.h>

#define SIMULATION_MAX_PLANES           5 // They all must be greater than 0
#define SIMULATION_MAX_TRAINS           7
#define SIMULATION_MAX_BOATS            6

#define ANIMATED_SPRITE_OUT_OF_MAP      128 // Coordinate outside of the map

// First object index to use for transportation sprites
#define TRANSPORT_OBJ_BASE_INDEX        64
// Location of graphics in VRAM
#define TRANSPORT_PALETTE               (13)
#define TRANSPORT_TILE_INDEX_BASE       768

//Planes
#define PLANE_SPR_OBJ_BASE          TRANSPORT_OBJ_BASE_INDEX
#define PLANE_TILE_INDEX_BASE       TRANSPORT_TILE_INDEX_BASE

// Trains
#define TRAIN_SPR_OBJ_BASE          (PLANE_SPR_OBJ_BASE + SIMULATION_MAX_PLANES)
#define TRAIN_TILE_INDEX_BASE       (PLANE_TILE_INDEX_BASE + 3)

// Boats
#define BOAT_SPR_OBJ_BASE           (TRAIN_SPR_OBJ_BASE + SIMULATION_MAX_TRAINS)
#define BOAT_TILE_INDEX_BASE        (TRAIN_TILE_INDEX_BASE + 2)

// Make sure that this doesn't overlap the cursor objects
static_assert(BOAT_SPR_OBJ_BASE + SIMULATION_MAX_BOATS < 124,
              "Too many transportation objects.");

void Simulation_TransportLoadGraphics(void);

void Simulation_TransportAnimsInit(void);
void Simulation_TransportAnimsHide(void);
void Simulation_TransportAnimsShow(void);

void Simulation_TransportAnimsVBLHandle(void);
void Simulation_TransportAnimsHandle(void);
void Simulation_TransportAnimsScroll(void);

#endif // SIMULATION_TRANSPORT_ANIMS_H__
