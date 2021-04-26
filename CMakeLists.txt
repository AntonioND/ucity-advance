# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (c) 2020-2021 Antonio Niño Díaz

cmake_minimum_required(VERSION 3.15)

# Name this project the same way as the folder it's in
get_filename_component(EXECUTABLE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

project(${EXECUTABLE_NAME})

add_executable(${EXECUTABLE_NAME})

# Link with libugba
# -----------------

# Get absolute path to default libugba folder
if("${UGBA_PATH}" STREQUAL "")
    get_filename_component(UGBA_PATH ../ugba ABSOLUTE)
endif()

if(EXISTS ${UGBA_PATH})
    add_subdirectory(${UGBA_PATH} libugba)
else()
    message(FATAL_ERROR "ugba folder not found")
endif()

target_link_libraries(${EXECUTABLE_NAME} libugba)

# Link with UMOD Player
# ---------------------

# Feel free to remove the option of the two provided that you don't need.

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/audio)
    get_filename_component(UMOD_PLAYER_PATH ../umod-player ABSOLUTE)

    if(EXISTS ${UMOD_PLAYER_PATH})
        add_subdirectory(${UMOD_PLAYER_PATH} umod_player)
    else()
        message(FATAL_ERROR "UMOD Player repository not found")
    endif()

    target_link_libraries(${EXECUTABLE_NAME} umod_player)
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

target_sources(${EXECUTABLE_NAME} PRIVATE
    ${FILES_SOURCE}
    ${FILES_BUILT_ASSETS}
)

target_include_directories(${EXECUTABLE_NAME} PRIVATE
    source
    built_assets
)
