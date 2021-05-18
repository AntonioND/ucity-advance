// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#ifndef SIMULATION_SIMULATION_MONEY_H__
#define SIMULATION_SIMULATION_MONEY_H__

typedef struct {
    int32_t taxes_residential;
    int32_t taxes_commercial;
    int32_t taxes_industrial;
    int32_t taxes_other; // Stadium, airport, seaport

    int32_t budget_police;
    int32_t budget_firemen;
    int32_t budget_healthcare; // Hospital, park
    int32_t budget_education; // Schools, university, museum, library
    int32_t budget_transport; // Road, train tracks

    int32_t budget_result;
} budget_info;

budget_info *Simulation_BudgetGet(void);

int Simulation_TaxPercentageGet(void);
void Simulation_TaxPercentageSet(int value);

void Simulation_CalculateBudgetAndTaxes(void);
void Simulation_ApplyBudgetAndTaxes(void);

#endif // SIMULATION_SIMULATION_MONEY_H__
