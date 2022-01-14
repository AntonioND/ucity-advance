# SPDX-License-Identifier: MIT
#
# Copyright (c) 2021-2022 Antonio Niño Díaz

ugba_toolchain_gba()

# Define library target
# ---------------------

set(GAME_NAME ucity-advance-gba)

add_executable(${GAME_NAME})
target_link_libraries(${GAME_NAME} libugba_gba)
target_link_libraries(${GAME_NAME} umod_player_gba)

# Source code, include directories and global definitions
# -------------------------------------------------------

target_sources(${GAME_NAME} PRIVATE ${ALL_FILES_SOURCE})
target_include_directories(${GAME_NAME} PRIVATE ${INCLUDE_PATHS})

# Build options
# -------------

gba_set_compiler_options(${GAME_NAME})

set(ARGS_C -Wall -Wextra -Wno-unused-parameter)

target_compile_options(${GAME_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:C>:${ARGS_C}>
)

target_link_options(${GAME_NAME} PRIVATE
    -flto
    -Wno-stringop-overflow -Wno-stringop-overread
)

# Generate GBA ROM from the ELF file
# ----------------------------------

make_gba_rom(${GAME_NAME} ${GAME_NAME} "UCITYADVANCE" "UCAD")

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${GAME_NAME}.gba
    DESTINATION
        .
)
