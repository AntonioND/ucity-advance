// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#ifndef ROOM_GAME_NOTIFICATION_BOX_H__
#define ROOM_GAME_NOTIFICATION_BOX_H__

void Notification_Box_Show(void);
void Notification_Box_Hide(void);

void Notification_Box_Load(void);

void Notification_Box_Clear(void);

void Notification_Box_Set_Text(const char *text);
int Notification_Box_Message_Is_Completed(void);
void Notification_Box_Update(void);

#endif // ROOM_GAME_NOTIFICATION_BOX_H__
