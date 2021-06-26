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

# New version

${SUPERFAMICONV} palette \
    --mode gba \
    --palettes 6 \
    --colors 16 \
    --color-zero FF00FF \
    --in-image ${IN}/ucity_logo.png \
    --out-data ${OUT}/ucity_logo_palette.bin \
    --verbose

${SUPERFAMICONV} tiles \
    --mode gba \
    --bpp 4 \
    --tile-width 8 --tile-height 8 \
    --max-tiles 512 \
    --in-image ${IN}/ucity_logo.png \
    --in-palette ${OUT}/ucity_logo_palette.bin \
    --out-data ${OUT}/ucity_logo_tiles.bin \
    --verbose

${SUPERFAMICONV} map \
    --mode gba \
    --bpp 4 \
    --tile-width 8 --tile-height 8 \
    --tile-base-offset 0 \
    --palette-base-offset 10 \
    --map-width 32 --map-height 32 \
    --in-image ${IN}/ucity_logo.png \
    --in-palette ${OUT}/ucity_logo_palette.bin \
    --in-tiles ${OUT}/ucity_logo_tiles.bin \
    --out-data ${OUT}/ucity_logo_map.bin \
    --verbose

# GBC version

${SUPERFAMICONV} palette \
    --mode gba \
    --palettes 6 \
    --colors 16 \
    --color-zero FF00FF \
    --in-image ${IN}/ucity_logo_gbc.png \
    --out-data ${OUT}/ucity_logo_gbc_palette.bin \
    --verbose

${SUPERFAMICONV} tiles \
    --mode gba \
    --bpp 4 \
    --tile-width 8 --tile-height 8 \
    --max-tiles 512 \
    --in-image ${IN}/ucity_logo_gbc.png \
    --in-palette ${OUT}/ucity_logo_gbc_palette.bin \
    --out-data ${OUT}/ucity_logo_gbc_tiles.bin \
    --verbose

${SUPERFAMICONV} map \
    --mode gba \
    --bpp 4 \
    --tile-width 8 --tile-height 8 \
    --tile-base-offset 0 \
    --palette-base-offset 10 \
    --map-width 32 --map-height 32 \
    --in-image ${IN}/ucity_logo_gbc.png \
    --in-palette ${OUT}/ucity_logo_gbc_palette.bin \
    --in-tiles ${OUT}/ucity_logo_gbc_tiles.bin \
    --out-data ${OUT}/ucity_logo_gbc_map.bin \
    --verbose
