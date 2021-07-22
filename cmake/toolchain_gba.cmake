# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (c) 2021 Antonio Niño Díaz

if(DEFINED ENV{DEVKITARM})
    set(DEVKITARM_BIN "$ENV{DEVKITARM}/bin/")
else()
    set(DEVKITARM_BIN "/opt/devkitpro/devkitARM/bin/")
endif()

if(DEFINED ENV{DEVKITPRO})
    set(DEVKITPRO_TOOLS "$ENV{DEVKITPRO}/tools/bin/")
else()
    set(DEVKITPRO_TOOLS "/opt/devkitpro/tools/bin/")
endif()

set(CMAKE_ASM_COMPILER "${DEVKITARM_BIN}arm-none-eabi-gcc")
set(CMAKE_C_COMPILER "${DEVKITARM_BIN}arm-none-eabi-gcc")
set(CMAKE_OBJCOPY "${DEVKITARM_BIN}arm-none-eabi-objcopy")
set(CMAKE_LINKER "${DEVKITARM_BIN}arm-none-eabi-ld")

set(CMAKE_SYSTEM_NAME "Game Boy Advance")
set(CMAKE_SYSTEM_PROCESSOR ARM)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_ASM_FLAGS "-x assembler-with-cpp")
