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

tiled.tmxrasterizer ${IN}/bank_offer_menu_bg.tmx ${IN}/bank_offer_menu_bg.png
tiled.tmxrasterizer ${IN}/bank_repay_menu_bg.tmx ${IN}/bank_repay_menu_bg.png
tiled.tmxrasterizer ${IN}/budget_menu_bg.tmx ${IN}/budget_menu_bg.png
tiled.tmxrasterizer ${IN}/city_stats_bg.tmx ${IN}/city_stats_bg.png
tiled.tmxrasterizer ${IN}/credits_bg.tmx ${IN}/credits_bg.png
tiled.tmxrasterizer ${IN}/generate_map_bg.tmx ${IN}/generate_map_bg.png
tiled.tmxrasterizer ${IN}/graphs_frame_bg.tmx ${IN}/graphs_frame_bg.png
tiled.tmxrasterizer ${IN}/main_menu_bg.tmx ${IN}/main_menu_bg.png
tiled.tmxrasterizer ${IN}/minimap_frame_bg.tmx ${IN}/minimap_frame_bg.png
tiled.tmxrasterizer ${IN}/name_input_menu_bg.tmx ${IN}/name_input_menu_bg.png
tiled.tmxrasterizer ${IN}/save_menu_bg.tmx ${IN}/save_menu_bg.png
tiled.tmxrasterizer ${IN}/scenario_selection_bg.tmx ${IN}/scenario_selection_bg.png
