# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (c) 2020-2022 Antonio Niño Díaz

cmake_minimum_required(VERSION 3.15)

# Name this project the same way as the folder it's in
get_filename_component(EXECUTABLE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

project(${EXECUTABLE_NAME})
enable_language(C)

set(C_STANDARD 11)
set(C_EXTENSIONS ON)
set(C_STANDARD_REQUIRED ON)

# Set installation settings
# -------------------------

# When invoking cmake install, install binaries to build directory
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install/" CACHE PATH
    "Default install path" FORCE)

# Set RPATH so that Linux looks for the libraries in the same folder. This needs
# to go before any add_executable() or add_library()
set(CMAKE_INSTALL_RPATH "$ORIGIN")

# Build options
# -------------

option(BUILD_GBA "Build GBA binaries in addition to PC binaries" OFF)

option(USE_DEVKITARM "Use devkitARM to build GBA binaries" ON)

# Link with libugba
# -----------------

# Get absolute path to libugba folder
get_filename_component(LIBUGBA_PATH "libugba" ABSOLUTE)

if(EXISTS ${LIBUGBA_PATH})
    add_subdirectory(${LIBUGBA_PATH} libugba)
else()
    message(FATAL_ERROR "ugba folder not found")
endif()

# Link with UMOD Player
# ---------------------

get_filename_component(UMOD_PLAYER_PATH "umod-player" ABSOLUTE)

if(EXISTS ${UMOD_PLAYER_PATH})
    add_subdirectory(${UMOD_PLAYER_PATH} umod_player)
else()
    message(FATAL_ERROR "UMOD Player repository not found")
endif()

# Add source code files
# ---------------------

# Macro that searches all the source files in the specified directory in 'dir'
# and its subdirectories, and saves them in 'var'
macro(search_source_files dir var)
    file(GLOB_RECURSE ${var} CONFIGURE_DEPENDS ${dir}/*.c ${dir}/*.h)
endmacro()

search_source_files(source FILES_SOURCE)
search_source_files(built_assets FILES_BUILT_ASSETS)
set(ALL_FILES_SOURCE
    ${FILES_SOURCE}
    ${FILES_BUILT_ASSETS}
)

get_filename_component(INCLUDE_PATH_SOURCE "source" ABSOLUTE)
get_filename_component(INCLUDE_PATH_BUILT_ASSETS "built_assets" ABSOLUTE)

set(INCLUDE_PATHS
    ${INCLUDE_PATH_SOURCE}
    ${INCLUDE_PATH_BUILT_ASSETS}
)

if(BUILD_GBA)
    add_subdirectory(gba)
endif()
add_subdirectory(sdl2)
