#!/bin/bash
#
# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (c) 2021, Antonio Niño Díaz

set -e

# Convert maps

bash maps/city/gen_maps.sh maps/city

bash maps/menus/gen_maps.sh maps/menus

bash maps/status_bar/gen_maps.sh maps/status_bar

# Done!

exit 0
