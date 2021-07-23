# SPDX-License-Identifier: MIT
#
# Copyright (c) 2021 Antonio Niño Díaz

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(
        # Force all integers to be 2's complement to prevent the compiler from
        # doing optimizations because of undefined behaviour.
        -fwrapv

        # Force usage of extern for external variables
        -fno-common

        # Enable most common warnings
        -Wall -Wextra

        # Disable this warning, which is enabled by default
        -Wformat-truncation=0
    )
    if(CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 9.3)
        add_compile_options(
            # Enable a bunch of warnings that aren't enabled with Wall or Wextra
            -Wformat-overflow=2 -Wformat=2 -Wno-format-nonliteral
            -Wundef -Wunused -Wuninitialized -Wunknown-pragmas -Wshadow
            -Wlogical-op -Wduplicated-cond -Wswitch-enum -Wfloat-equal
            -Wcast-align -Walloc-zero -Winline
            -Wstrict-overflow=5 -Wstringop-overflow=4
            $<$<COMPILE_LANGUAGE:C>:-Wstrict-prototypes>
            $<$<COMPILE_LANGUAGE:C>:-Wold-style-definition>

            # Make sure we don't use too much stack. Windows doesn't like it
            # when the stack usage is too high, even when Linux doesn't complain
            # about it.
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
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    add_compile_definitions(
        # Silence warnings
        _USE_MATH_DEFINES
        _CRT_SECURE_NO_WARNINGS
    )
    add_compile_options(
        # Enable parallel compilation
        /MP
    )
endif()
