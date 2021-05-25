// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#ifndef ROOM_GRAPHS_GRAPHS_HANDLER_H__
#define ROOM_GRAPHS_GRAPHS_HANDLER_H__

#include <stdint.h>

#define GRAPH_SIZE              64 // Entries per graph

#define GRAPH_MAX_VALUE         63
#define GRAPH_MIN_VALUE         -64

// This value means that the entry is empty.
#define GRAPH_INVALID_ENTRY     -128

typedef struct {
    int8_t values[GRAPH_SIZE];
    uint32_t write_ptr;
    uint32_t shift; // To get the real value, do "value[i] << shift"
} graph_info;

typedef enum {
    GRAPH_INFO_POPULATION,
    GRAPH_INFO_RESIDENTIAL,
    GRAPH_INFO_COMMERCIAL,
    GRAPH_INFO_INDUSTRIAL,
    GRAPH_INFO_FUNDS,
} graph_info_type;

graph_info *Graph_Get(graph_info_type type);

void Graph_Reset(graph_info *info);

void Graph_Add_Record(graph_info *info, int32_t value);

#endif // ROOM_GRAPHS_GRAPHS_HANDLER_H__
