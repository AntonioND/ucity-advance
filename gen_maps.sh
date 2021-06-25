#!/bin/bash
#
# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (c) 2021, Antonio Niño Díaz

set -e

# Convert maps

MAPS_CITY=maps/city
bash maps/city/gen_maps.sh ${MAPS_CITY}

# Done!

exit 0
