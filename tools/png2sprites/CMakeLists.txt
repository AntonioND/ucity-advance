# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (c) 2021 Antonio Niño Díaz

add_executable(png2sprites png2sprites.c)

target_compile_options(png2sprites PRIVATE -Wformat-truncation=0)

# libpng is required
find_package(PNG REQUIRED 1.6)
target_link_libraries(png2sprites PRIVATE ${PNG_LIBRARIES})
target_include_directories(png2sprites PRIVATE ${PNG_INCLUDE_DIRS})
