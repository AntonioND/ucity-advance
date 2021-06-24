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

${PNGS2STRIP} ${OUT}/building_menu_sprites.png \
    ${IN}/0_menu.png \
    ${IN}/1_demolish.png \
    ${IN}/2_rci.png \
    ${IN}/3_transportation.png \
    ${IN}/4_services.png \
    ${IN}/5_parks.png \
    ${IN}/6_education.png \
    ${IN}/7_culture.png \
    ${IN}/8_ports.png \
    ${IN}/9_energy_1.png \
    ${IN}/10_energy_2.png

${SUPERFAMICONV} palette \
    --mode gba \
    --palettes 1 \
    --colors 192 \
    --color-zero FF00FF \
    --in-image ${OUT}/building_menu_sprites.png \
    --out-data ${OUT}/building_menu_sprites_palette_gbc.bin \
    --out-image ${OUT}/building_menu_sprites_palette_gbc.png \
    --verbose

${SUPERFAMICONV} tiles \
    --mode gba \
    --bpp 8 \
    --tile-width 8 --tile-height 8 \
    --max-tiles 256 \
    --in-image ${OUT}/building_menu_sprites.png \
    --in-palette ${OUT}/building_menu_sprites_palette_gbc.bin \
    --out-data ${OUT}/building_menu_sprites_tiles_gbc.bin \
    --no-flip --no-discard \
    --verbose
