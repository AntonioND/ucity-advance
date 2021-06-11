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
    --colors 160 \
    --color-zero FF00FF \
    --in-image ${IN}/city_map_tiles.png \
    --out-data ${OUT}/city_map_palette.bin \
    --out-image ${OUT}/city_map_palette.png \
    --verbose

${SUPERFAMICONV} tiles \
    --mode gba \
    --bpp 8 \
    --tile-width 8 --tile-height 8 \
    --max-tiles 512 \
    --in-image ${IN}/city_map_tiles.png \
    --in-palette ${OUT}/city_map_palette.bin \
    --out-data ${OUT}/city_map_tiles.bin \
    --no-flip --no-discard \
    --verbose

${SUPERFAMICONV} map \
    --mode gba \
    --bpp 8 \
    --tile-width 8 --tile-height 8 \
    --tile-base-offset 0 \
    --palette-base-offset 0 \
    --map-width 64 --map-height 64 \
    --split-width 64 --split-height 64 \
    --in-image ${IN}/scenario_0_rock_river.png \
    --in-palette ${OUT}/city_map_palette.bin \
    --in-tiles ${OUT}/city_map_tiles.bin \
    --out-data ${OUT}/scenario_0_rock_river.bin \
    --no-flip \
    --verbose

${SUPERFAMICONV} map \
    --mode gba \
    --bpp 8 \
    --tile-width 8 --tile-height 8 \
    --tile-base-offset 0 \
    --palette-base-offset 0 \
    --map-width 64 --map-height 64 \
    --split-width 64 --split-height 64 \
    --in-image ${IN}/scenario_1_boringtown.png \
    --in-palette ${OUT}/city_map_palette.bin \
    --in-tiles ${OUT}/city_map_tiles.bin \
    --out-data ${OUT}/scenario_1_boringtown.bin \
    --verbose

${SUPERFAMICONV} map \
    --mode gba \
    --bpp 8 \
    --tile-width 8 --tile-height 8 \
    --tile-base-offset 0 \
    --palette-base-offset 0 \
    --map-width 64 --map-height 64 \
    --split-width 64 --split-height 64 \
    --in-image ${IN}/scenario_2_portville.png \
    --in-palette ${OUT}/city_map_palette.bin \
    --in-tiles ${OUT}/city_map_tiles.bin \
    --out-data ${OUT}/scenario_2_portville.bin \
    --no-flip \
    --verbose

${SUPERFAMICONV} map \
    --mode gba \
    --bpp 8 \
    --tile-width 8 --tile-height 8 \
    --tile-base-offset 0 \
    --palette-base-offset 0 \
    --map-width 64 --map-height 64 \
    --split-width 64 --split-height 64 \
    --in-image ${IN}/scenario_3_newdale.png \
    --in-palette ${OUT}/city_map_palette.bin \
    --in-tiles ${OUT}/city_map_tiles.bin \
    --out-data ${OUT}/scenario_3_newdale.bin \
    --no-flip \
    --verbose

${SUPERFAMICONV} map \
    --mode gba \
    --bpp 8 \
    --tile-width 8 --tile-height 8 \
    --tile-base-offset 0 \
    --palette-base-offset 0 \
    --map-width 64 --map-height 64 \
    --split-width 64 --split-height 64 \
    --in-image ${IN}/test_map.png \
    --in-palette ${OUT}/city_map_palette.bin \
    --in-tiles ${OUT}/city_map_tiles.bin \
    --out-data ${OUT}/test_map.bin \
    --no-flip \
    --verbose
