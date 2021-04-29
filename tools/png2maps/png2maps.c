// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <png.h>

#if !defined(PNG_SIMPLIFIED_READ_SUPPORTED)
# error "This code needs libpng 1.6"
#endif

// Load a PNG file into a RGBA buffer
int Read_PNG(const char *filename, unsigned char **_buffer,
             int *_width, int *_height)
{
    png_image image;

    // Only the image structure version number needs to be set
    memset(&image, 0, sizeof image);
    image.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_file(&image, filename))
    {
        printf("%s(): png_image_begin_read_from_file(): %s\n",
               __func__, image.message);
        return 1;
    }

    image.format = PNG_FORMAT_RGBA;

    png_bytep buffer;
    buffer = malloc(PNG_IMAGE_SIZE(image));

    if (buffer == NULL)
    {
        png_image_free(&image);
        printf("%s(): Not enough memory\n", __func__);
        return 1;
    }

    if (!png_image_finish_read(&image, NULL, buffer, 0, NULL))
    {
        printf("%s(): png_image_finish_read(): %s\n", __func__, image.message);
        free(buffer);
        return 1;
    }

    *_buffer = buffer;
    *_width = image.width;
    *_height = image.height;

    return 0;
}

typedef struct {
    int r, g, b;
} color_info;

typedef struct {
    color_info color[16];
    int numcolors;
} palette_info;

palette_info palettes[16];
int used_palettes = 0;

int Palette_Get_Or_Add_Color(palette_info *p, int r, int g, int b)
{
    r &= 0xF8;
    g &= 0xF8;
    b &= 0xF8;

    for (int i = 0; i < p->numcolors; i++)
    {
        color_info *c = &(p->color[i]);
        if ((c->r == r) && (c->g == g) && (c->b == b))
            return i;
    }

    if (p->numcolors < 16)
    {
        color_info *c = &(p->color[p->numcolors]);
        c->r = r;
        c->g = g;
        c->b = b;
        p->numcolors++;
        return p->numcolors - 1;
    }

    return -1;
}

int Palette_Is_Subset(palette_info *whole, palette_info *subset)
{
    for (size_t i = 0; i < subset->numcolors; i++)
    {
        color_info *cs = &(subset->color[i]);

        int color_present = 0;

        for (size_t j = 0; j < whole->numcolors; j++)
        {
            color_info *cw = &(whole->color[j]);

            if ((cs->r == cw->r) && (cs->g == cw->g) && (cs->b == cw->b))
            {
                color_present = 1;
                break;
            }
        }

        if (color_present == 0)
            return 0;
    }

    return 1;
}

int Palette_Get_Color_Index(palette_info *p, int r, int g, int b)
{
    r &= 0xF8;
    g &= 0xF8;
    b &= 0xF8;

    for (size_t i = 0; i < p->numcolors; i++)
    {
        color_info *c = &(p->color[i]);
        if ((c->r == r) && (c->g == g) && (c->b == b))
            return i;
    }

    return -1;
}

typedef struct {
    int pixel[8 * 8];
    color_info original[8 * 8];
    int palette;
} tile_info;

tile_info tile[2048];
int total_tiles = 0;

int Load_And_Convert_Tileset(const char *in_png, const char *base_name,
                             const char *out_h, const char *out_c)
{
    printf("%s()\n", __func__);
    printf("  %s\n", in_png);
    //printf("  %s\n", base_name);
    //printf("  %s\n", out_h);
    //printf("  %s\n", out_c);

    int ret = -1;

    FILE *fh = NULL;
    FILE *fc = NULL;

    unsigned char *buffer = NULL;
    int width, height;

    if (Read_PNG(in_png, &buffer, &width, &height) != 0)
    {
        printf("Failed to read %s\n", in_png);
        goto cleanup;
    }

    printf("  Size: %dx%d\n", width, height);

    total_tiles = (width / 8) * (height / 8);

    printf("  Total tiles: %d\n", total_tiles);

    if (total_tiles > 2048)
    {
        printf("Too many tiles!\n");
        total_tiles = 2048;
    }

    for (int c = 0; c < total_tiles; c++)
    {
        // Create palette out of this tile

        palette_info p = { 0 };
        Palette_Get_Or_Add_Color(&p, 255, 0, 255); // Make color 0 magenta

        for (int j = 0; j < 8; j++)
        {
            for (int i = 0; i < 8; i++)
            {
                int xx = ((c % (width / 8)) * 8) + i;
                int yy = (c / (width / 8) * 8) + j;

                int index = (xx + (yy * width)) * 4;

                int r = buffer[index + 0];
                int g = buffer[index + 1];
                int b = buffer[index + 2];

                int color = Palette_Get_Or_Add_Color(&p, r, g, b);
                if (color == -1)
                {
                    printf("%s:%d: This shouldn't happen.\n", __func__, __LINE__);
                    goto cleanup;
                }
            }
        }

        // Check if this palette already exists

        int pal_index = -1;

        for (int i = 0; i < used_palettes; i++)
        {
            palette_info *search = &palettes[i];
            if (Palette_Is_Subset(search, &p) != 0)
            {
                pal_index = i;
                //printf("%d: Found palette: %d\n", c, pal_index);
                break;
            }
        }

        // If the palette doesn't exist, create it

        if (pal_index == -1)
        {
            printf("%d: New palette: %d\n", c, used_palettes);

            if (used_palettes >= 16)
            {
                printf("Not enough available palettes!\n");
                goto cleanup;
            }

            palette_info *dst = &palettes[used_palettes];
            memcpy(dst, &p, sizeof(palette_info));

            pal_index = used_palettes;

            used_palettes++;
        }

        // Save the palette that this tile uses

        tile[c].palette = pal_index;

        // Generate tile data based on the selected palette

        palette_info *selected_pal = &palettes[pal_index];

        for (int j = 0; j < 8; j++)
        {
            for (int i = 0; i < 8; i++)
            {
                int xx = ((c % (width / 8)) * 8) + i;
                int yy = (c / (width / 8) * 8) + j;

                int index = (xx + (yy * width)) * 4;

                int r = buffer[index + 0];
                int g = buffer[index + 1];
                int b = buffer[index + 2];

                int color = Palette_Get_Color_Index(selected_pal, r, g, b);
                if (color == -1)
                {
                    printf("%s:%d: This shouldn't happen.\n", __func__, __LINE__);
                    goto cleanup;
                }

                tile[c].pixel[j * 8 + i] = color;
                tile[c].original[j * 8 + i].r = r;
                tile[c].original[j * 8 + i].g = g;
                tile[c].original[j * 8 + i].b = b;
            }
        }
    }

    // Open files

    fh = fopen(out_h, "w");
    if (fh == NULL)
    {
        printf("Can't open %s\n", out_h);
        goto cleanup;
    }

    fc = fopen(out_c, "w");
    if (fc == NULL)
    {
        printf("Can't open %s\n", out_c);
        goto cleanup;
    }

    fprintf(fh, "#ifndef %s_H__\n", base_name);
    fprintf(fh, "#define %s_H__\n\n", base_name);
    fprintf(fh, "#include <stdint.h>\n\n");

    fprintf(fc, "#include <stdint.h>\n\n");

    // Save palette

    fprintf(fh, "extern const uint16_t %s_pal[];\n", base_name);
    fprintf(fh, "#define %s_pal_size   %u // bytes\n",
            base_name, used_palettes * 16 * 2);
    fprintf(fh, "#define %s_pal_start  0\n", base_name);
    fprintf(fh, "#define %s_pal_number %u\n\n", base_name, used_palettes);

    fprintf(fc, "const uint16_t %s_pal[] = {\n", base_name);
    for (int p = 0; p < used_palettes; p++)
    {
        for (int i = 0; i < 16; i++)
        {
            if ((i % 8) == 0)
                fprintf(fc, "    ");

            int r = palettes[p].color[i].r >> 3;
            int g = palettes[p].color[i].g >> 3;
            int b = palettes[p].color[i].b >> 3;

            uint16_t color = (b << 10) | (g << 5) | r;
            fprintf(fc, "0x%04X", color);

            if ((i % 8) == 7)
                fprintf(fc, ",\n");
            else
                fprintf(fc, ", ");
        }
    }
    fprintf(fc, "};\n\n");

    // Save tileset

    fprintf(fh, "extern const uint16_t %s_tiles[];\n", base_name);
    fprintf(fh, "#define %s_tiles_size   %u // bytes\n",
            base_name, total_tiles * 8 * 8 / 2);
    fprintf(fh, "#define %s_tiles_start  0\n", base_name);
    fprintf(fh, "#define %s_tiles_number %u\n\n", base_name, total_tiles);

    fprintf(fc, "const uint16_t %s_tiles[] = {\n", base_name);

    int spacer = 0;

    for (int t = 0; t < total_tiles; t++)
    {
        for (int i = 0; i < 8 * 8 / 4; i++)
        {
            if ((spacer % 8) == 0)
                fprintf(fc, "    ");

            int p0 = tile[t].pixel[i * 4 + 0];
            int p1 = tile[t].pixel[i * 4 + 1];
            int p2 = tile[t].pixel[i * 4 + 2];
            int p3 = tile[t].pixel[i * 4 + 3];

            uint16_t entry = (p3 << 12) | (p2 << 8) | (p1 << 4) | p0;

            fprintf(fc, "0x%04X", entry);

            if ((spacer % 8) == 7)
                fprintf(fc, ",\n");
            else
                fprintf(fc, ", ");

            spacer++;
        }
    }
    fprintf(fc, "};\n");

    // End files

    fprintf(fh, "#endif // %s_H__\n", base_name);

    // Done

    ret = 0;

cleanup:
    if (fc)
        fclose(fc);
    if (fh)
        fclose(fh);
    free(buffer);

    return ret;
}

int Load_And_Convert_Map(const char *in_png, const char *base_name,
                         const char *out_h, const char *out_c)
{
    printf("%s()\n", __func__);
    printf("  %s\n", in_png);
    printf("  %s\n", base_name);
    printf("  %s\n", out_h);
    printf("  %s\n", out_c);

    int ret = -1;

    FILE *fh = NULL;
    FILE *fc = NULL;

    unsigned char *buffer = NULL;
    int width, height;

    if (Read_PNG(in_png, &buffer, &width, &height) != 0)
    {
        printf("Failed to read %s\n", in_png);
        goto cleanup;
    }

    printf("  Size: %dx%d\n", width, height);

    // Open files

    fh = fopen(out_h, "w");
    if (fh == NULL)
    {
        printf("Can't open %s\n", out_h);
        goto cleanup;
    }

    fc = fopen(out_c, "w");
    if (fc == NULL)
    {
        printf("Can't open %s\n", out_c);
        goto cleanup;
    }

    fprintf(fh, "#ifndef %s_H__\n", base_name);
    fprintf(fh, "#define %s_H__\n\n", base_name);
    fprintf(fh, "#include <stdint.h>\n\n");

    fprintf(fc, "#include <stdint.h>\n\n");

    // Save map

    int tw = width / 8;
    int th = height / 8;

    fprintf(fh, "extern const uint16_t %s_map[];\n", base_name);
    fprintf(fh, "#define %s_map_size   %u // bytes\n",
            base_name, tw * th * 2);
    fprintf(fh, "#define %s_map_width  %u\n", base_name, tw);
    fprintf(fh, "#define %s_map_height %u\n\n", base_name, th);

    fprintf(fc, "const uint16_t %s_map[] = {\n", base_name);

    int spacer = 0;

    for (int j = 0; j < th; j++)
    {
        for (int i = 0; i < tw; i++)
        {
            if ((spacer % 8) == 0)
                fprintf(fc, "    ");

            int tile_index = -1;
            uint16_t entry = 0;

            // Look for this tile in the tile array

            for (int t = 0; t < total_tiles; t++)
            {
                int is_same_tile = 1;

                for (int y = 0; y < 8; y++)
                {
                    for (int x = 0; x < 8; x++)
                    {
                        int r = tile[t].original[y * 8 + x].r;
                        int g = tile[t].original[y * 8 + x].g;
                        int b = tile[t].original[y * 8 + x].b;

                        int yy = j * 8 + y;
                        int xx = i * 8 + x;

                        int index = (yy * width + xx) * 4;

                        int rr = buffer[index + 0];
                        int gg = buffer[index + 1];
                        int bb = buffer[index + 2];

                        if ((r != rr) || (g != gg) || (b != bb))
                        {
                            is_same_tile = 0;
                            goto different;
                        }
                    }
                }
different:

                if (is_same_tile)
                {
                    tile_index = t;
                    break;
                }
            }

            // Save to file

            entry = tile_index | (tile[tile_index].palette << 12);

            fprintf(fc, "0x%04X", entry);

            if ((spacer % 8) == 7)
                fprintf(fc, ",\n");
            else
                fprintf(fc, ", ");

            spacer++;
        }
    }

    fprintf(fc, "};\n");

    // End files

    fprintf(fh, "#endif // %s_H__\n", base_name);

    // Done

    ret = 0;

cleanup:
    if (fc)
        fclose(fc);
    if (fh)
        fclose(fh);
    free(buffer);

    return ret;
}

const char *Get_File_Name(const char *path)
{
    size_t l = strlen(path);
    for ( ; l > 0; l--)
    {
        if ((path[l] == '\\') || (path[l] == '/'))
            return &(path[l + 1]);
    }

    return path;
}

void Remove_Extension(char *path)
{
    size_t l = strlen(path);
    for ( ; l > 0; l--)
    {
        if (path[l] == '.')
        {
            path[l] = '\0';
            return;
        }
    }
}

#define MAX_PATHLEN     (2048)

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s /path/base_name tileset.png map1.png <map2.png> <map3.png> ...\n",
               argv[0]);
        return 2;
    }

    const char *out_base_name = argv[1];
    const char *in_tileset_path = argv[2];

    int ret = 1;

    {
        const char *png_name = Get_File_Name(in_tileset_path);

        char *base_name = strdup(png_name);
        Remove_Extension(base_name);

        char out_h[MAX_PATHLEN];
        snprintf(out_h, sizeof(out_h), "%s%s.h", out_base_name, base_name);
        char out_c[MAX_PATHLEN];
        snprintf(out_c, sizeof(out_c), "%s%s.c", out_base_name, base_name);

        ret = Load_And_Convert_Tileset(in_tileset_path, base_name, out_h, out_c);

        free(base_name);

        if (ret != 0)
            return -1;
    }

    argc -= 3;
    argv += 3;

    while (argc > 0)
    {
        const char *png_name = Get_File_Name(argv[0]);

        char *base_name = strdup(png_name);
        Remove_Extension(base_name);

        char out_h[MAX_PATHLEN];
        snprintf(out_h, sizeof(out_h), "%s%s.h", out_base_name, base_name);
        char out_c[MAX_PATHLEN];
        snprintf(out_c, sizeof(out_c), "%s%s.c", out_base_name, base_name);

        ret = Load_And_Convert_Map(argv[0], base_name, out_h, out_c);

        free(base_name);

        if (ret != 0)
            return -2;

        argc--;
        argv++;
    }

    return 0;
}
