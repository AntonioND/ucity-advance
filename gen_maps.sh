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

# Paths to tools

export PNGS2STRIP=tools/build/pngs2strip/pngs2strip

# Generate city tileset

bash maps/buildings/convert.sh maps/city

bash maps/buildings_gbc/convert.sh maps/city

# Convert maps

bash maps/city/gen_maps.sh maps/city

bash maps/menus/gen_maps.sh maps/menus

bash maps/status_bar/gen_maps.sh maps/status_bar

# Done!

exit 0
