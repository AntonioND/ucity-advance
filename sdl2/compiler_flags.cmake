# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020-2022 Antonio Niño Díaz

macro(compiler_flags_sdl2 target_name)

    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target_name} PRIVATE
            # Enable most common warnings
            -Wall -Wextra

            # Disable this warning, which is enabled by default
            -Wformat-truncation=0
        )
        if(CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 9.3)
            target_compile_options(${target_name} PRIVATE
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
                target_compile_options(${target_name} PRIVATE
                    -fsanitize=address      # Detect out-of-bounds accesses
                    -fsanitize=undefined    # Detect C undefined behaviour

                )
            endif()
        endif()
    endif()

endmacro()


macro(linker_flags_sdl2 target_name)

    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        if(CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 9.3)
            if(ENABLE_UBSAN)
                target_link_options(${target_name} PRIVATE
                    -fsanitize=address
                    -fsanitize=undefined
                )
            endif()
        endif()
    endif()

endmacro()
