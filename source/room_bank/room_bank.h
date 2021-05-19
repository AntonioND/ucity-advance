// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#ifndef ROOM_BANK_ROOM_BANK_H__
#define ROOM_BANK_ROOM_BANK_H__

void Room_Bank_Set_Loan(int payments, int amount);
void Room_Bank_Get_Loan(int *payments, int *amount);

void Room_Bank_Load(void);
void Room_Bank_Unload(void);

void Room_Bank_Handle(void);

#endif // ROOM_BANK_ROOM_BANK_H__
