# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (c) 2021 Antonio Niño Díaz

add_executable(pngs2strip pngs2strip.c)

target_compile_options(pngs2strip PRIVATE -Wformat-truncation=0)

# libpng is required
find_package(PNG REQUIRED 1.6)
target_link_libraries(pngs2strip PRIVATE ${PNG_LIBRARIES})
target_include_directories(pngs2strip PRIVATE ${PNG_INCLUDE_DIRS})
