# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (c) 2021-2022 Antonio Niño Díaz

if(USE_DEVKITARM)
    if(DEFINED ENV{DEVKITARM})
        set(ARM_GCC_PATH "$ENV{DEVKITARM}/bin/")
    else()
        set(ARM_GCC_PATH "/opt/devkitpro/devkitARM/bin/")
    endif()
else()
    set(ARM_GCC_PATH "")
endif()

set(CMAKE_ASM_COMPILER "${ARM_GCC_PATH}arm-none-eabi-gcc")
set(CMAKE_C_COMPILER "${ARM_GCC_PATH}arm-none-eabi-gcc")
set(CMAKE_LINKER "${ARM_GCC_PATH}arm-none-eabi-ld")
set(CMAKE_AR "${ARM_GCC_PATH}arm-none-eabi-ar")
set(CMAKE_OBJCOPY "${ARM_GCC_PATH}arm-none-eabi-objcopy")

set(CMAKE_SYSTEM_NAME "Game Boy Advance")
set(CMAKE_SYSTEM_PROCESSOR ARM)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_ASM_FLAGS "-x assembler-with-cpp")
