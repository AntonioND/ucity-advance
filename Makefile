# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (c) 2021 Antonio Niño Díaz

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitPro)
endif

# User options
# ------------

TARGET      := $(notdir $(CURDIR))

BUILDDIR    := build
SOURCEDIR   := source
INCLUDEDIRS := source
BUILTASSETS := built_assets

LIBUGBA     := $(CURDIR)/../ugba/libugba
UMOD_PLAYER := $(CURDIR)/../umod-player/player

LIBS        := -lugba -lumod
LIBDIRS     := $(LIBUGBA) $(UMOD_PLAYER)

# Look for source files
# ---------------------

CFILES      := $(shell find $(SOURCEDIR) -type f -name '*.c')
CPPFILES    := $(shell find $(SOURCEDIR) -type f -name '*.cpp')
SFILES      := $(shell find $(SOURCEDIR) -type f -name '*.s')

OBJS        := $(patsubst $(SOURCEDIR)/%.c,$(BUILDDIR)/%.o,$(CFILES)) \
               $(patsubst $(SOURCEDIR)/%.cpp,$(BUILDDIR)/%.o,$(CPPFILES)) \
               $(patsubst $(SOURCEDIR)/%.s,$(BUILDDIR)/%.o,$(SFILES))

DEPS        := $(OBJS:.o=.d)

# Files related to assets

ASSETCFILES := $(shell find $(BUILTASSETS) -type f -name '*.c')
INCLUDEDIRS += $(BUILTASSETS)
OBJS        += $(patsubst $(BUILTASSETS)/%.c,$(BUILDDIR)/assets/%.o,$(ASSETCFILES))

# Includes

INCLUDES    += $(foreach dir,$(INCLUDEDIRS),-I$(dir))

# Libraries

INCLUDES    += $(foreach dir,$(LIBDIRS),-I$(dir)/include)
LIBPATHS    := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

# Toolchain
# ---------

export PATH := $(DEVKITPRO)/tools/bin:$(DEVKITPRO)/devkitARM/bin:$(PATH)

PREFIX      := arm-none-eabi-

CC          := $(PREFIX)gcc
CXX         := $(PREFIX)g++
AS          := $(PREFIX)as
OBJCOPY     := $(PREFIX)objcopy

# Use C++ compiler to link if there are C++ files

ifeq ($(strip $(CPPFILES)),)
    LD      := $(CC)
else
    LD      := $(CXX)
endif

# Other tools

MKDIR       := mkdir

# Toolchain options
# -----------------

WARNFLAGS   := -Wall -Wextra -Wunused-parameter

ARCH        := -mthumb -mthumb-interwork

CFLAGS      := -g -O2 -flto -mcpu=arm7tdmi -mtune=arm7tdmi \
               $(WARNFLAGS) $(ARCH) $(INCLUDES) -D__GBA__

CXXFLAGS    := $(CFLAGS) -fno-rtti -fno-exceptions

ASFLAGS     := -g $(ARCH)
LDFLAGS     := -g -flto $(ARCH) -Wl,-Map,$(BUILDDIR)/$(TARGET).map

# Build options
# -------------

ifeq ($(V),1)
    QUIET   :=
else
    QUIET   := @
endif

# Build targets
# -------------

.PHONY: all clean

all : $(TARGET).gba

$(TARGET).gba: $(TARGET).elf
	@echo "  OBJCOPY $@"
	$(QUIET)$(OBJCOPY) -O binary $< $@
	@echo "  GBAFIX  $@"
	$(QUIET)gbafix $@

$(TARGET).elf: $(OBJS)
	@echo "  LD      $@"
	$(QUIET)$(LD) $(LDFLAGS) -specs=gba.specs $(OBJS) $(LIBPATHS) $(LIBS) -o $@

$(BUILDDIR)/%.o: $(SOURCEDIR)/%.cpp
	@echo "  CXX     $<"
	@$(MKDIR) -p $(@D)
	$(QUIET)$(CXX) -MMD -MP $(CXXFLAGS) -c $< -o

$(BUILDDIR)/%.o: $(SOURCEDIR)/%.c
	@echo "  CC      $<"
	@$(MKDIR) -p $(@D)
	$(QUIET)$(CC) -MMD -MP $(CFLAGS) -c $< -o $@

$(BUILDDIR)/assets/%.o: $(BUILTASSETS)/%.c
	@echo "  CC      $<"
	@$(MKDIR) -p $(@D)
	$(QUIET)$(CC) -MMD -MP $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: $(SOURCEDIR)/%.s
	@echo "  AS      $<"
	@$(MKDIR) -p $(@D)
	$(QUIET)$(CC) -MMD -MP -x assembler-with-cpp $(ASFLAGS) -c $< -o $@

clean:
	@echo "  CLEAN"
	@rm -fr $(BUILDDIR) $(TARGET).elf $(TARGET).gba

# Include dependency files if they exist
# --------------------------------------

-include $(DEPS)
