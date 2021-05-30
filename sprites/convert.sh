#!/bin/bash
#
# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (c) 2021, Antonio Niño Díaz

SCRIPT=`realpath $0`
IN=`dirname $SCRIPT`
OUT=$1

echo ""
echo "[*] Converting ${IN}..."
echo ""

${SUPERFAMICONV} palette \
    --mode gba \
    --palettes 1 \
    --colors 16 \
    --color-zero FF00FF \
    --in-image ${IN}/cursor.png \
    --out-data ${OUT}/cursor_palette.bin \
    --out-image ${OUT}/cursor_palette.png \
    --verbose

${SUPERFAMICONV} tiles \
    --mode gba \
    --bpp 4 \
    --tile-width 8 --tile-height 8 \
    --in-image ${IN}/cursor.png \
    --in-palette ${OUT}/cursor_palette.bin \
    --out-data ${OUT}/cursor_tiles.bin \
    --verbose
