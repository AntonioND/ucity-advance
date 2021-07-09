#!/bin/bash
#
# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (c) 2021, Antonio Niño Díaz

SCRIPT=`realpath $0`
IN=`dirname $SCRIPT`
OUT=$1

echo ""
echo "[*] Generating maps in ${IN}..."
echo ""

tiled.tmxrasterizer ${IN}/scenario_0_rock_river.tmx ${IN}/scenario_0_rock_river.png
tiled.tmxrasterizer ${IN}/scenario_1_boringtown.tmx ${IN}/scenario_1_boringtown.png
tiled.tmxrasterizer ${IN}/scenario_2_portville.tmx ${IN}/scenario_2_portville.png
tiled.tmxrasterizer ${IN}/scenario_3_newdale.tmx ${IN}/scenario_3_newdale.png
tiled.tmxrasterizer ${IN}/scenario_4_central.tmx ${IN}/scenario_4_central.png
tiled.tmxrasterizer ${IN}/scenario_5_futura.tmx ${IN}/scenario_5_futura.png
tiled.tmxrasterizer ${IN}/test_map.tmx ${IN}/test_map.png
