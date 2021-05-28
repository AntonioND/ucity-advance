// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#ifndef ROOM_SAVE_SLOTS_ROOM_SAVE_SLOTS_H__
#define ROOM_SAVE_SLOTS_ROOM_SAVE_SLOTS_H__

typedef enum {
    ROOM_SAVE_SLOTS_LOAD,
    ROOM_SAVE_SLOTS_SAVE,
} room_save_slots_modes;

void Room_Save_Slots_Set_Mode(room_save_slots_modes mode);

void Room_Save_Slots_Load(void);
void Room_Save_Slots_Unload(void);

void Room_Save_Slots_Handle(void);

#endif // ROOM_SAVE_SLOTS_ROOM_SAVE_SLOTS_H__
