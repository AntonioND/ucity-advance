// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <stdint.h>

#include <ugba/ugba.h>

#include "room_graphs/graphs_handler.h"

static graph_info graph_population;
static graph_info graph_residential, graph_commercial, graph_industrial;
static graph_info graph_funds;

graph_info *Graph_Get(graph_info_type type)
{
    switch (type)
    {
        case GRAPH_INFO_POPULATION:
            return &graph_population;
        case GRAPH_INFO_RESIDENTIAL:
            return &graph_residential;
        case GRAPH_INFO_COMMERCIAL:
            return &graph_commercial;
        case GRAPH_INFO_INDUSTRIAL:
            return &graph_industrial;
        case GRAPH_INFO_FUNDS:
            return &graph_funds;
        default:
            UGBA_Assert(0);
            return NULL;
    }
}

static void scale_down_graph(graph_info *info)
{
    for (int i = 0; i < GRAPH_SIZE; i++)
    {
        if (info->values[i] != GRAPH_INVALID_ENTRY)
        {
            info->values[i] >>= 1;
        }
    }

    info->shift++;
}

void Graph_Reset(graph_info *info)
{
    for (int i = 0; i < GRAPH_SIZE; i++)
        info->values[i] = GRAPH_INVALID_ENTRY;
    info->shift = 0;
    info->write_ptr = 0;
}

void Graph_Add_Record(graph_info *info, int32_t value)
{
    int8_t final_value;

    while (1)
    {
        int32_t try_value = value >> info->shift;

        if ((try_value > GRAPH_MAX_VALUE) || (try_value < GRAPH_MIN_VALUE))
        {
            scale_down_graph(info);
        }
        else
        {
            final_value = try_value;
            break;
        }
    }

    info->values[info->write_ptr] = final_value;

    info->write_ptr++;
    if (info->write_ptr >= GRAPH_SIZE)
        info->write_ptr = 0;
}
