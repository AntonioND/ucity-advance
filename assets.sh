#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (c) 2021, Antonio Niño Díaz

set -e

BASE_DIR=$(pwd)

# Build tools

pushd tools

rm -rf build ; mkdir build ; cd build
cmake ..
make

popd

# Prepare destination folder

OUT_DIR=built_assets
rm -rf ${OUT_DIR}
mkdir ${OUT_DIR}

# Convert city tileset and maps

mkdir ${OUT_DIR}/maps

tools/build/png2maps/png2maps \
    ${OUT_DIR}/maps/ \
    maps/city_tileset.png \
    maps/city_tileset_map.png \
    maps/scenario_0_rock_river.png maps/scenario_1_boringtown.png \
    maps/scenario_2_portville.png maps/scenario_3_newdale.png \
    maps/test_map.png \

# Convert sprite sheets

mkdir ${OUT_DIR}/sprites

tools/build/png2sprites/png2sprites \
    ${OUT_DIR}/sprites/ \
    sprites/building_menu_tiles.png \
    sprites/building_menu_map.png

tools/build/png2sprites/png2sprites \
    ${OUT_DIR}/sprites/ \
    sprites/minimap_menu_tiles.png \
    sprites/minimap_menu_map.png

tools/build/png2maps/png2maps \
    ${OUT_DIR}/maps/ \
    maps/minimap_frame_tiles.png \
    maps/minimap_frame_bg.png \
    maps/bank_offer_menu_bg.png \
    maps/bank_repay_menu_bg.png \
    maps/budget_menu_bg.png \
    maps/city_stats_bg.png \
    maps/generate_map_bg.png \
    maps/main_menu_bg.png \
    maps/name_input_menu_bg.png \
    maps/notification_bg.png \
    maps/pause_menu_bg.png

# Convert other graphics

mkdir ${OUT_DIR}/graphics

pushd ${OUT_DIR}/graphics
for png in $(find "${BASE_DIR}/graphics/" -name "*.png")
do
    echo grit ${png} -ftc -o $(basename ${png} ".png")
    grit ${png} -ftc -o $(basename ${png} ".png")
done
popd

# Convert music

mkdir ${OUT_DIR}/audio

../umod-player/build/packer/umod_packer \
    ${OUT_DIR}/audio/umod_pack.bin \
    ${OUT_DIR}/audio/umod_pack_info.h \
    audio/*

# Convert data files

# TODO

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
