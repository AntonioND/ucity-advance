// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#ifndef ROOM_INPUT_ROOM_INPUT_H__
#define ROOM_INPUT_ROOM_INPUT_H__

void Room_Input_Load(void);
void Room_Input_Unload(void);

const char *Room_Input_Text_String(void);

void Room_Input_Handle(void);

#endif // ROOM_INPUT_ROOM_INPUT_H__
