# SPDX-License-Identifier: MIT
#
# Copyright (c) 2021 Antonio Niño Díaz

include(${CMAKE_SOURCE_DIR}/cmake/toolchain_gba.cmake)

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

target_compile_definitions(${GAME_NAME} PRIVATE
    __GBA__ $<$<CONFIG:RELEASE>:NDEBUG>)

set(ARGS_C
    -Wall -Wextra -Wno-unused-parameter
    -mthumb -mthumb-interwork
    -mcpu=arm7tdmi -mtune=arm7tdmi
    -ffunction-sections -fdata-sections
)

target_compile_options(${GAME_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:C>:${ARGS_C}>
)

target_link_options(${GAME_NAME} PRIVATE
    -flto
    -Wno-stringop-overflow -Wno-stringop-overread
    -specs=gba.specs
    -mthumb -mthumb-interwork
    -Wl,-Map,mapfile.map -Wl,--gc-sections
)

install(
    TARGETS
        ${GAME_NAME}
    DESTINATION
        .
)

# Generate GBA ROM from the ELF file
# ----------------------------------

add_custom_command(
    OUTPUT ${GAME_NAME}.gba
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${GAME_NAME}> ${GAME_NAME}.gba
    COMMAND ${DEVKITPRO_TOOLS}gbafix ${GAME_NAME}.gba
    DEPENDS ${GAME_NAME}
)

add_custom_target(ucity_advance_gba_rom ALL
    DEPENDS ${GAME_NAME}.gba
)
