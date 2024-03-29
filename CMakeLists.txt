# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (c) 2020-2022 Antonio Niño Díaz

cmake_minimum_required(VERSION 3.15)

# Name this project the same way as the folder it's in
get_filename_component(EXECUTABLE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

project(${EXECUTABLE_NAME})
enable_language(C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

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

# Look for an ARM toolchain
# -------------------------

if(NOT "${BUILD_GBA}" STREQUAL OFF)
    if("${ARM_GCC_PATH}" STREQUAL "")
        message(STATUS "ugba-testing: GBA toolchain not specified")
        ugba_search_toolchain_gba(BUILD_GBA)
    else()
        message(STATUS "ugba-testing: GBA toolchain has been specified")
        set(BUILD_GBA ON)
    endif()
endif()

if(BUILD_GBA)
    message(STATUS "ugba-testing: GBA toolchain: ${ARM_GCC_PATH}")
endif()

# Add UMOD Player to the project after the toolchain is selected
# --------------------------------------------------------------

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
