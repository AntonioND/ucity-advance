# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020-2022 Antonio Niño Díaz

ugba_toolchain_sdl2()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(
        # Enable most common warnings
        -Wall -Wextra

        # Disable this warning, which is enabled by default
        -Wformat-truncation=0
    )
    if(CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 9.3)
        add_compile_options(
            # Enable a bunch of warnings that aren't enabled with Wall or
            # Wextra
            -Wformat-overflow=2 -Wformat=2 -Wno-format-nonliteral
            -Wundef -Wunused -Wuninitialized -Wunknown-pragmas -Wshadow
            -Wlogical-op -Wduplicated-cond -Wswitch-enum -Wfloat-equal
            -Wcast-align -Walloc-zero -Winline
            -Wstrict-overflow=5 -Wstringop-overflow=4
            $<$<COMPILE_LANGUAGE:C>:-Wstrict-prototypes>
            $<$<COMPILE_LANGUAGE:C>:-Wold-style-definition>

            # Enable Wpedantic but disable warning about having strings that
            # are too long
            -Wpedantic -Wno-overlength-strings

            # Make sure we don't use too much stack. Windows doesn't like it
            # when the stack usage is too high, even when Linux doesn't
            # complain about it.
            -Wstack-usage=4096

            # TODO: Enable the following warnings?
            #-Wformat-truncation=1 -Wcast-qual -Wconversion
        )

        # Build option to enable Undefined Behaviour Sanitizer (UBSan)
        # --------------------------------------------------------
        #
        # This should only be enabled in debug builds. It makes the code far
        # slower, so it should only be used during development.
        option(ENABLE_UBSAN "Compile with UBSan support (GCC)" OFF)

        if(ENABLE_UBSAN)
            add_compile_options(
                -fsanitize=address      # Detect out-of-bounds accesses
                -fsanitize=undefined    # Detect C undefined behaviour

            )
            add_link_options(
                -fsanitize=address
                -fsanitize=undefined
            )
        endif()
    endif()
endif()

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
