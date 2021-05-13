// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#ifndef CURSOR_H__
#define CURSOR_H__

void Cursor_Reset_Position(void);

void Cursor_Set_Position(int x, int y);
void Cursor_Get_Position(int *x, int *y);

void Cursor_Hide(void);
void Cursor_Update(void);

void Cursor_Set_Size(int w, int h);
void Cursor_Get_Size(int *w, int *h);

void Cursor_Refresh(void);

void Load_Cursor_Graphics(void *tiles_base, int tiles_index);
void Load_Cursor_Palette(int pal_index);

#endif // CURSOR_H__
