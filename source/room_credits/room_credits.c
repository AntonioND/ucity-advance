// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021 Antonio Niño Díaz

#include <ugba/ugba.h>

#include "main.h"
#include "room_game/draw_common.h"
#include "room_game/room_game.h"

// Assets

#include "maps/menus/credits_bg_bin.h"
#include "maps/menus/menus_palette_bin.h"
#include "maps/menus/menus_palette_gbc_bin.h"
#include "maps/menus/menus_tileset_bin.h"
#include "maps/menus/menus_tileset_gbc_bin.h"

#define BG_CREDITS_PALETTE          (0)
#define BG_CREDITS_TILES_BASE       MEM_BG_TILES_BLOCK_ADDR(3)
#define BG_CREDITS_MAP_BASE         MEM_BG_MAP_BLOCK_ADDR(28)

#define CHAR_MICRO      "\x83"
#define CHAR_COPYRIGHT  "\x8D"
#define CHAR_NTILDE     "\x8F"

static const char *credits_pages[] = {

    // Page 1

    "\n"
    "  " CHAR_MICRO "City " CHAR_COPYRIGHT " 2017-2018, 2021\n"
    "\n"
    "  Antonio Ni" CHAR_NTILDE "o Diaz\n"
    "\n"
    "\n"
    "    AntonioND/SkyLyrac\n"
    "\n"
    "    antonio_nd@outlook.com\n"
    "\n"
    "    www.skylyrac.net\n"
    "    github.com/AntonioND\n"
    "\n"
    "\n"
    "  Licenses:\n"
    "\n"
    "    Code: GPL-3.0-only\n"
    "    Art: CC-BY-NC-SA-4.0\n"
    "    Music: Various\n"
    "                           1/5\n",

    // Page 2

    "\n"
    "  Music:\n"
    "\n"
    "  'no energy'\n"
    "  roberts/beta team\n"
    "  ID: 120127\n"
    "  Mod Archive Distr. license\n"
    "\n"
    "  balthasar\n"
    "  andy / banal\n"
    "  ID: 76460\n"
    "  Mod Archive Distr. license\n"
    "\n"
    "  The Mssing Lnk.\n"
    "  optic & dascon of trsi\n"
    "  ID: 113254\n"
    "  Mod Archive Distr. license\n"
    "\n"
    "\n"
    "                           2/5\n",

    // Page 3

    "\n"
    "  Music:\n"
    "\n"
    "  jointpeople\n"
    "  Anadune & Floppy\n"
    "  ID: 115447\n"
    "  Mod Archive Distr. license\n"
    "\n"
    "  Improvisation\n"
    "  Unknown\n"
    "  ID: 44379\n"
    "  Mod Archive Distr. license\n"
    "\n"
    "  tecjaz\n"
    "  AE\n"
    "  ID: 59910\n"
    "  Mod Archive Distr. license\n"
    "\n"
    "\n"
    "                           3/5\n",

    // Page 4

    "\n"
    "  Music:\n"
    "\n"
    "  Schwinging the swing\n"
    "  Cybelius of sonic pc\n"
    "  ID: 54272\n"
    "  Mod Archive Distr. license\n"
    "\n"
    "\n"
    "  Other music and SFXs:\n"
    "\n"
    "    AntonioND/SkyLyrac\n"
    "    CC-BY-NC-SA-4.0\n"
    "\n"
    "\n"
    "  Valentina, thanks for your\n"
    "  patience and support during\n"
    "  this project.\n"
    "\n"
    "                           4/5\n",

    // Page 5

    "\n"
    "  Other thanks:\n"
    "\n"
    "    devkitPro\n"
    "\n"
    "    SDL 2\n"
    "\n"
    "    libpng\n"
    "\n"
    "    Aseprite\n"
    "\n"
    "    Tiled\n"
    "\n"
    "    JFXR\n"
    "\n"
    "    Open ModPlug Tracker\n"
    "\n"
    "    The Mod Archive\n"
    "\n"
    "                           5/5\n",
};

static size_t current_page = 0;

static int Room_Credits_PageExists(size_t num)
{
    size_t nelem = sizeof(credits_pages) / sizeof(credits_pages[0]);
    if (num >= nelem)
        return 0;

    return 1;
}

static void Room_Credits_PagePrint(size_t num)
{
    if (Room_Credits_PageExists(num) == 0)
        return;

    // Reload the map
    SWI_CpuSet_Copy16(credits_bg_bin, (void *)BG_CREDITS_MAP_BASE,
                      credits_bg_bin_size);

    const char *text = credits_pages[num];

    uintptr_t addr = BG_CREDITS_MAP_BASE;

    while (1)
    {
        int c = (uint8_t)*text++;
        if (c == '\0')
        {
            break;
        }
        else if (c == '\n')
        {
            // This needs to be uintptr_t, not uint32_t, so that the pointer
            // isn't clamped down to 32 bits by mistake.
            uintptr_t mask = 32 * 2; // 32 columns
            addr = (addr + mask) & ~(mask - 1);
            continue;
        }

        uint16_t *ptr = (uint16_t *)addr;

        *ptr = MAP_REGULAR_TILE(c) | MAP_REGULAR_PALETTE(BG_CREDITS_PALETTE);

        addr += 2;
    }
}

void Room_Credits_Load(void)
{
    // Load frame map
    // --------------

    // Load the tiles
    if (Room_Game_Graphics_New_Get())
    {
        SWI_CpuSet_Copy16(menus_tileset_bin, (void *)BG_CREDITS_TILES_BASE,
                          menus_tileset_bin_size);
    }
    else
    {
        SWI_CpuSet_Copy16(menus_tileset_gbc_bin, (void *)BG_CREDITS_TILES_BASE,
                          menus_tileset_gbc_bin_size);
    }

    // Load the map
    SWI_CpuSet_Copy16(credits_bg_bin, (void *)BG_CREDITS_MAP_BASE,
                      credits_bg_bin_size);

    // Setup background
    BG_RegularInit(1, BG_REGULAR_256x256, BG_16_COLORS,
                   BG_CREDITS_TILES_BASE, BG_CREDITS_MAP_BASE);
    BG_RegularScrollSet(1, 0, 0);

    // Update room state
    // -----------------

    current_page = 0;
    Room_Credits_PagePrint(current_page);

    // Setup display mode

    DISP_ModeSet(1);
    DISP_LayersEnable(0, 1, 0, 0, 0);

    // Load palettes
    // -------------

    // Load frame palettes
    if (Room_Game_Graphics_New_Get())
    {
        SWI_CpuSet_Copy16(menus_palette_bin,
                          &MEM_PALETTE_BG[BG_CREDITS_PALETTE],
                          menus_palette_bin_size);
    }
    else
    {
        SWI_CpuSet_Copy16(menus_palette_gbc_bin,
                          &MEM_PALETTE_BG[BG_CREDITS_PALETTE],
                          menus_palette_gbc_bin_size);
    }

    MEM_PALETTE_BG[0] = RGB15(31, 31, 31);
}

void Room_Credits_Unload(void)
{
    Game_Clear_Screen();
}

void Room_Credits_Handle(void)
{
    uint16_t keys_pressed = KEYS_Pressed();

    if (keys_pressed & KEY_A)
    {
        if (Room_Credits_PageExists(current_page + 1))
        {
            current_page++;
            Room_Credits_PagePrint(current_page);
        }
        else
        {
            Game_Room_Prepare_Switch(ROOM_MAIN_MENU);
            return;
        }
    }

    if (keys_pressed & KEY_RIGHT)
    {
        if (Room_Credits_PageExists(current_page + 1))
        {
            current_page++;
            Room_Credits_PagePrint(current_page);
        }
    }
    if (keys_pressed & KEY_LEFT)
    {
        if (current_page > 0)
        {
            current_page--;
            Room_Credits_PagePrint(current_page);
        }
    }

    if (keys_pressed & KEY_B)
    {
        Game_Room_Prepare_Switch(ROOM_MAIN_MENU);
        return;
    }
}
