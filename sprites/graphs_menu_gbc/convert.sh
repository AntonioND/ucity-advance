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

${PNGS2STRIP} ${OUT}/graphs_menu_sprites.png \
    ${IN}/icons.png

${SUPERFAMICONV} palette \
    --mode gba \
    --palettes 1 \
    --colors 256 \
    --color-zero FF00FF \
    --in-image ${OUT}/graphs_menu_sprites.png \
    --out-data ${OUT}/graphs_menu_sprites_palette_gbc.bin \
    --out-image ${OUT}/graphs_menu_sprites_palette_gbc.png \
    --verbose

${SUPERFAMICONV} tiles \
    --mode gba \
    --bpp 8 \
    --tile-width 8 --tile-height 8 \
    --max-tiles 256 \
    --in-image ${OUT}/graphs_menu_sprites.png \
    --in-palette ${OUT}/graphs_menu_sprites_palette_gbc.bin \
    --out-data ${OUT}/graphs_menu_sprites_tiles_gbc.bin \
    --no-flip --no-discard \
    --verbose
