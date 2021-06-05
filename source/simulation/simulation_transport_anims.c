// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include "room_game/room_game.h"
#include "simulation/simulation_anim_boats.h"
#include "simulation/simulation_anim_planes.h"
#include "simulation/simulation_anim_trains.h"
#include "simulation/simulation_transport_anims.h"

// Assets

#include "sprites/transport_palette_bin.h"
#include "sprites/transport_tiles_bin.h"

static int sprites_shown;

// This holds the previous scroll values. We are only going to handle small
// displacements, not jumps to other places of the screen, so we only need the
// lower 8 bits of the scroll. If a big jump is performed, the correct thing to
// do is a refresh of all the objects.
int simulation_scx_old;
int simulation_scy_old;

void Simulation_TransportLoadGraphics(void)
{
    VRAM_OBJTiles16Copy(transport_tiles_bin, transport_tiles_bin_size,
                        TRANSPORT_TILE_INDEX_BASE);

    VRAM_OBJPalette16Copy(transport_palette_bin, transport_palette_bin_size,
                          TRANSPORT_PALETTE);
}

// Initialize objects to random locations. It doesn't refresh the sprites.
// It must be called after initializing the BG and its scroll as this function
// sets the initial reference.
void Simulation_TransportAnimsInit(void)
{
    if (Room_Game_AreAnimationsEnabled() == 0)
        return;

    sprites_shown = 0;

    PlanesReset();
    TrainsReset();
    BoatsReset();

    // Set scroll reference

    int scx_new, scy_new;
    Room_Game_GetCurrentScroll(&scx_new, &scy_new);
    simulation_scx_old = scx_new;
    simulation_scy_old = scy_new;
}

void Simulation_TransportAnimsHide(void)
{
    if (Room_Game_AreAnimationsEnabled() == 0)
        return;

    if (sprites_shown == 0)
        return;

    sprites_shown = 0;

    PlanesHide();
    TrainsHide();
    BoatsHide();
}

// This refreshes all sprites. It should be used right after the objects are
// generated or if they have been hidden before and they should appear on the
// place they were back then. This should also be used after a jump to a
// different part of the map, at least if the objects are supposed to be visible
// after the jump.
void Simulation_TransportAnimsShow(void)
{
    if (Room_Game_AreAnimationsEnabled() == 0)
        return;

    if (sprites_shown == 1)
        return;

    PlanesShow();
    TrainsShow();
    BoatsShow();

    sprites_shown = 1;
}

// Move objects according to the movement of each transport means, to be called
// once per VBL. This should be really fast! It hides objects when they leave
// the screen and shows them when they reach the screen area again. It doesn't
// create or destroy objects when they leave the map.
void Simulation_TransportAnimsVBLHandle(void)
{
    if (Room_Game_AreAnimationsEnabled() == 0)
        return;

    // Check if sprites are hidden or not (for example, during disasters). If
    // they aren't visible, do nothing, they will have to be refreshed when the
    // disaster ends, for example...
    if (sprites_shown == 0)
        return;

    PlanesVBLHandle();
    TrainsVBLHandle();
    BoatsVBLHandle();
}

// Handle objects that leave the map and destroy them, create new objects, etc.
// Called once per animation step. This can take all the time it needs.
void Simulation_TransportAnimsHandle(void)
{
    if (Room_Game_AreAnimationsEnabled() == 0)
        return;

    if (sprites_shown == 0)
        return;

    PlanesHandle();
    TrainsHandle();
    BoatsHandle();
}

// Update sprites according to the scroll of the background. It checks the
// current scroll and compares it with the previous one. It can only handle
// small displacements, in case of a jump to another part of the map, this
// function isn't enough (a refresh is needed).
void Simulation_TransportAnimsScroll(void)
{
    if (Room_Game_AreAnimationsEnabled() == 0)
        return;

    if (sprites_shown == 0)
        return;

    // Calculate increment

    int scx_new, scy_new;
    Room_Game_GetCurrentScroll(&scx_new, &scy_new);

    int deltax = simulation_scx_old - scx_new;
    int deltay = simulation_scy_old - scy_new;

    // Update references

    simulation_scx_old = scx_new;
    simulation_scy_old = scy_new;

    // Scroll sprites

    if ((deltax == 0) && (deltay == 0))
        return;

    PlanesHandleScroll(deltax, deltay);
    TrainsHandleScroll(deltax, deltay);
    BoatsHandleScroll(deltax, deltay);
}
