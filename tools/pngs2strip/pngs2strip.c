// SPDX-License-Identifier: GPL-3.0-only
//
// Copyright (c) 2021, Antonio Niño Díaz

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <png.h>

#if !defined(PNG_SIMPLIFIED_READ_SUPPORTED) || \
    !defined(PNG_SIMPLIFIED_WRITE_SUPPORTED)
# error "This code needs libpng 1.6"
#endif

// Save a RGBA buffer into a PNG file
int Save_PNG(const char *filename, unsigned char *buffer,
             int width, int height, int is_rgba)
{
    png_image image;
    memset(&image, 0, sizeof(image));

    image.version = PNG_IMAGE_VERSION;
    image.width = width;
    image.height = height;
    image.flags = 0;
    image.colormap_entries = 0;

    if (is_rgba)
        image.format = PNG_FORMAT_RGBA;
    else
        image.format = PNG_FORMAT_RGB;

    int row_stride = is_rgba ? (width * 4) : (width * 3);

    if (!png_image_write_to_file(&image, filename, 0, buffer, row_stride, NULL))
    {
        printf("%s(): png_image_write_to_file(): %s",
               __func__, image.message);
        return 1;
    }

    return 0;
}

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
    uint32_t pixel[8 * 8];
} tile_info;

tile_info tile[512];
int total_tiles = 0;

int Load_And_Convert_Image(const char *in_png)
{
    printf("Reading %s\n", in_png);

    int ret = -1;

    unsigned char *buffer = NULL;
    int width, height;

    if (Read_PNG(in_png, &buffer, &width, &height) != 0)
    {
        printf("Failed to read %s\n", in_png);
        goto cleanup;
    }

    printf("  Size: %dx%d\n", width, height);

    if (((width % 8) != 0) || ((height % 8) != 0))
    {
        printf("Invalid size\n");
        goto cleanup;
    }

    int tiles_w = width / 8;
    int tiles_h = height / 8;
    int tiles = tiles_w * tiles_h;

    printf("  Total tiles: %d\n", tiles);

    int skipped = 0;

    for (int t = 0; t < tiles; t++)
    {
        if (total_tiles == 512)
        {
            printf("Too many tiles!\n");
            goto cleanup;
        }

        int base_x = (t % tiles_w) * 8;
        int base_y = (t / tiles_w) * 8;

        int skip = 0;

        for (int j = 0; j < 8; j++)
        {
            for (int i = 0; i < 8; i++)
            {
                int index = (((base_y + j) * width) + (base_x + i)) * 4;
                uint32_t r = buffer[index + 0];
                uint32_t g = buffer[index + 1];
                uint32_t b = buffer[index + 2];
                uint32_t color = (b << 16) | (g << 8) | r;
                // Tiles with magenta are skipped
                if (color == 0xFF00FF)
                    skip = 1;
                tile[total_tiles].pixel[j * 8 + i] = color;
            }
        }

        if (skip)
            skipped++;
        else
            total_tiles++;
    }

    if (skipped > 0)
        printf("  Skipped: %d\n", skipped);

    // Done

    ret = 0;

cleanup:
    free(buffer);

    return ret;
}

int Save_Combined_Image(const char *out_png)
{
    printf("Saving to %s\n", out_png);

    int ret = -1;

    unsigned char *buffer = malloc(total_tiles * 8 * 8 * 4);
    if (buffer == NULL)
    {
        printf("Failed to allocate buffer to create %s\n", out_png);
        goto cleanup;
    }

    for (int t = 0; t < total_tiles; t++)
    {
        int base_y = t * 8;

        for (int j = 0; j < 8; j++)
        {
            for (int i = 0; i < 8; i++)
            {
                int index = (((base_y + j) * 8) + i) * 4;
                uint32_t color = tile[t].pixel[j * 8 + i];
                buffer[index + 0] = color & 0xFF;
                buffer[index + 1] = (color >> 8) & 0xFF;
                buffer[index + 2] = (color >> 16) & 0xFF;
                buffer[index + 3] = 0xFF;
            }
        }
    }

    ret = Save_PNG(out_png, buffer, 8, total_tiles * 8, 1);

    // Done
    ret = 0;
cleanup:

    free(buffer);

    return ret;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s out.png in1.png <in2.png> <in3.png> ...\n",
               argv[0]);
        return 2;
    }

    const char *out_path  = argv[1];

    argc -= 2;
    argv += 2;

    while (argc > 0)
    {
        int ret = Load_And_Convert_Image(argv[0]);
        if (ret != 0)
            return -2;

        argc--;
        argv++;
    }

    return Save_Combined_Image(out_path);
}
