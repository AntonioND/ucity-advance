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

tmxrasterizer ${IN}/bank_offer_menu_bg.tmx ${IN}/bank_offer_menu_bg.png
tmxrasterizer ${IN}/bank_repay_menu_bg.tmx ${IN}/bank_repay_menu_bg.png
tmxrasterizer ${IN}/budget_menu_bg.tmx ${IN}/budget_menu_bg.png
tmxrasterizer ${IN}/city_stats_bg.tmx ${IN}/city_stats_bg.png
tmxrasterizer ${IN}/credits_bg.tmx ${IN}/credits_bg.png
tmxrasterizer ${IN}/generate_map_bg.tmx ${IN}/generate_map_bg.png
tmxrasterizer ${IN}/graphs_frame_bg.tmx ${IN}/graphs_frame_bg.png
tmxrasterizer ${IN}/main_menu_bg.tmx ${IN}/main_menu_bg.png
tmxrasterizer ${IN}/minimap_frame_bg.tmx ${IN}/minimap_frame_bg.png
tmxrasterizer ${IN}/name_input_menu_bg.tmx ${IN}/name_input_menu_bg.png
tmxrasterizer ${IN}/save_menu_bg.tmx ${IN}/save_menu_bg.png
tmxrasterizer ${IN}/scenario_selection_bg.tmx ${IN}/scenario_selection_bg.png
