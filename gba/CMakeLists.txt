# SPDX-License-Identifier: MIT
#
# Copyright (c) 2021-2022 Antonio Niño Díaz

ugba_toolchain_gba()

# Define library target
# ---------------------

set(GAME_NAME ucity-advance-gba)

set(ELF_FILE ${GAME_NAME}.elf)

add_executable(${ELF_FILE})
target_link_libraries(${ELF_FILE} libugba_gba)
target_link_libraries(${ELF_FILE} umod_player_gba)

# Source code, include directories and global definitions
# -------------------------------------------------------

target_sources(${ELF_FILE} PRIVATE ${ALL_FILES_SOURCE})
target_include_directories(${ELF_FILE} PRIVATE ${INCLUDE_PATHS})

# Build options
# -------------

gba_set_compiler_options(${ELF_FILE})

set(ARGS_C -Wall -Wextra -Wno-unused-parameter)

target_compile_options(${ELF_FILE} PRIVATE
    $<$<COMPILE_LANGUAGE:C>:${ARGS_C}>
)

target_link_options(${ELF_FILE} PRIVATE
    -flto
    -Wno-stringop-overflow -Wno-stringop-overread
)

# Generate GBA ROM from the ELF file
# ----------------------------------

# The game code starts with "2" for compatibility with the EverDrive GBA. It is
# used to specify that the save type is SRAM.
make_gba_rom(${ELF_FILE} ${GAME_NAME} "UCITYADVANCE" "2UCT")

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${GAME_NAME}.gba
    DESTINATION
        .
)
