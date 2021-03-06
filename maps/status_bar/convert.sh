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
    --palettes 10 \
    --colors 16 \
    --color-zero FF00FF \
    --in-image ${IN}/text.png \
    --out-data ${OUT}/text_palette.bin \
    --out-image ${OUT}/text_palette.png \
    --verbose

${SUPERFAMICONV} tiles \
    --mode gba \
    --bpp 4 \
    --tile-width 8 --tile-height 8 \
    --max-tiles 512 \
    --in-image ${IN}/text.png \
    --in-palette ${OUT}/text_palette.bin \
    --out-data ${OUT}/text_tiles.bin \
    --no-flip --no-discard \
    --verbose

convert_map() {
    ${SUPERFAMICONV} map \
        --mode gba \
        --bpp 4 \
        --tile-width 8 --tile-height 8 \
        --tile-base-offset 0 \
        --palette-base-offset 0 \
        --map-width 32 --map-height 32 \
        --in-image ${IN}/$1.png \
        --in-palette ${OUT}/text_palette.bin \
        --in-tiles ${OUT}/text_tiles.bin \
        --out-data ${OUT}/$1.bin \
        --no-flip \
        --verbose
}

convert_map notification_bg
convert_map pause_menu_bg
