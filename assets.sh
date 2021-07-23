#!/bin/bash
#
# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (c) 2021, Antonio Niño Díaz

set -e

# Build tools

pushd tools
rm -rf build ; mkdir build ; cd build
cmake ..
make
popd

pushd SuperFamiconv
rm -rf build ; mkdir build ; cd build
cmake ..
make -j`nproc`
popd

pushd umod-player
rm -rf build ; mkdir build ; cd build
cmake ..
make
popd

# Paths to tools

export SUPERFAMICONV=SuperFamiconv/build/superfamiconv
export PNGS2STRIP=tools/build/pngs2strip/pngs2strip

# Prepare destination folder

OUT_DIR=built_assets
rm -rf ${OUT_DIR}
mkdir ${OUT_DIR}

# Convert maps

OUT_DIR_MAPS_MENUS=${OUT_DIR}/maps/menus
mkdir -p ${OUT_DIR_MAPS_MENUS}
bash maps/menus/convert.sh ${OUT_DIR_MAPS_MENUS}

OUT_DIR_MAPS_CITY=${OUT_DIR}/maps/city
mkdir -p ${OUT_DIR_MAPS_CITY}
bash maps/city/convert.sh ${OUT_DIR_MAPS_CITY}

OUT_DIR_MAPS_INTRO=${OUT_DIR}/maps/intro
mkdir -p ${OUT_DIR_MAPS_INTRO}
bash maps/intro/convert.sh ${OUT_DIR_MAPS_INTRO}

OUT_DIR_MAPS_STATUS_BAR=${OUT_DIR}/maps/status_bar
mkdir -p ${OUT_DIR_MAPS_STATUS_BAR}
bash maps/status_bar/convert.sh ${OUT_DIR_MAPS_STATUS_BAR}

# Convert sprite sheets

OUT_DIR_SPRITES=${OUT_DIR}/sprites
mkdir -p ${OUT_DIR_SPRITES}
bash sprites/convert.sh ${OUT_DIR_SPRITES}

OUT_DIR_SPRITES_BUILDING_MENU=${OUT_DIR}/sprites/building_menu
mkdir -p ${OUT_DIR_SPRITES_BUILDING_MENU}
bash sprites/building_menu/convert.sh ${OUT_DIR_SPRITES_BUILDING_MENU}

OUT_DIR_SPRITES_BUILDING_MENU_GBC=${OUT_DIR}/sprites/building_menu_gbc
mkdir -p ${OUT_DIR_SPRITES_BUILDING_MENU_GBC}
bash sprites/building_menu_gbc/convert.sh ${OUT_DIR_SPRITES_BUILDING_MENU_GBC}

OUT_DIR_SPRITES_GRAPHS_MENU=${OUT_DIR}/sprites/graphs_menu
mkdir -p ${OUT_DIR_SPRITES_GRAPHS_MENU}
bash sprites/graphs_menu/convert.sh ${OUT_DIR_SPRITES_GRAPHS_MENU}

OUT_DIR_SPRITES_GRAPHS_MENU_GBC=${OUT_DIR}/sprites/graphs_menu_gbc
mkdir -p ${OUT_DIR_SPRITES_GRAPHS_MENU_GBC}
bash sprites/graphs_menu_gbc/convert.sh ${OUT_DIR_SPRITES_GRAPHS_MENU_GBC}

OUT_DIR_SPRITES_MINIMAP_MENU=${OUT_DIR}/sprites/minimap_menu
mkdir -p ${OUT_DIR_SPRITES_MINIMAP_MENU}
bash sprites/minimap_menu/convert.sh ${OUT_DIR_SPRITES_MINIMAP_MENU}

OUT_DIR_SPRITES_MINIMAP_MENU_GBC=${OUT_DIR}/sprites/minimap_menu_gbc
mkdir -p ${OUT_DIR_SPRITES_MINIMAP_MENU_GBC}
bash sprites/minimap_menu_gbc/convert.sh ${OUT_DIR_SPRITES_MINIMAP_MENU_GBC}

# Convert music

mkdir ${OUT_DIR}/audio

umod-player/build/packer/umod_packer \
    ${OUT_DIR}/audio/umod_pack.bin \
    ${OUT_DIR}/audio/umod_pack_info.h \
    audio/songs/*/*.mod \
    audio/*.wav

# Convert binary files generated by other stages

for dir in $(find built_assets -type d)
do
    for f in $(find $dir -maxdepth 1 -iname *.bin)
    do
        tools/build/bin2c/bin2c $f "$dir"
    done
done

# Done!

exit 0
