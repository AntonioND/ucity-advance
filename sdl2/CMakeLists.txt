# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020-2021 Antonio Niño Díaz

include(${CMAKE_SOURCE_DIR}/cmake/toolchain_host.cmake)

# Define library target
# ---------------------

add_executable(ucity-advance)
target_link_libraries(ucity-advance libugba)
target_link_libraries(ucity-advance umod_player)

# Source code, include directories and global definitions
# -------------------------------------------------------

target_sources(ucity-advance PRIVATE ${ALL_FILES_SOURCE})
target_include_directories(ucity-advance PRIVATE ${INCLUDE_PATHS})

install(
    TARGETS
        ucity-advance libugba
    DESTINATION
        .
)
