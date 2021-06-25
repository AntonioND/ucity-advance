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

tiled.tmxrasterizer ${IN}/notification_bg.tmx ${IN}/notification_bg.png
tiled.tmxrasterizer ${IN}/pause_menu_bg.tmx ${IN}/pause_menu_bg.png
