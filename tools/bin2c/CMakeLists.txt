# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (c) 2021 Antonio Niño Díaz

add_executable(bin2c bin2c.c)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(bin2c PRIVATE -Wformat-truncation=0)
endif()
