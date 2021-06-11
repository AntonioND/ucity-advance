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

${PNGS2STRIP} ${OUT}/city_map_tiles_gbc.png \
    ${IN}/0_grass_forest.png \
    ${IN}/16_grass_water.png \
    ${IN}/30_rci.png \
    ${IN}/34_road_train_power_lines.png \
    ${IN}/85_police_department.png \
    ${IN}/94_fire_department.png \
    ${IN}/103_hospital.png \
    ${IN}/112_park_small.png \
    ${IN}/113_park_big.png \
    ${IN}/122_stadium.png \
    ${IN}/142_school.png \
    ${IN}/148_high_school.png \
    ${IN}/157_university.png \
    ${IN}/182_museum.png \
    ${IN}/194_library.png \
    ${IN}/200_airport.png \
    ${IN}/215_port.png \
    ${IN}/228_power_plant_coal.png \
    ${IN}/244_power_plant_oil.png \
    ${IN}/260_power_plant_wind.png \
    ${IN}/264_power_plant_solar.png \
    ${IN}/280_power_plant_nuclear.png \
    ${IN}/296_power_plant_fusion.png \
    ${IN}/312_residential_1x1.png \
    ${IN}/316_residential_2x2.png \
    ${IN}/332_residential_3x3.png \
    ${IN}/368_commercial_1x1.png \
    ${IN}/372_commercial_2x2.png \
    ${IN}/388_commercial_3x3.png \
    ${IN}/424_industrial_1x1.png \
    ${IN}/428_industrial_2x2.png \
    ${IN}/444_industrial_3x3.png \
    ${IN}/480_fire.png \
    ${IN}/482_radiation.png
