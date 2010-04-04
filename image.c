/*  The image handling.
 *
 *      This file is part of the camshot utility.
 *
 *  Copyright (c) 2010 Gabriel Zabusek <gabriel.zabusek@gmail.com>
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      3 of the License, or (at your option) any later version.
 */

#include <stdio.h>

#include "image.h"
#include "arguments.h"

static int convert_yuv_to_rgb_pixel(int y, int u, int v);

/* Copyright 2007 (c) Logitech. All Rights Reserved. (yuv -> rgb conversion) */
int convert_yuv_to_rgb_pixel(int y, int u, int v)
{
	unsigned int pixel32 = 0;
	unsigned char *pixel = (unsigned char *)&pixel32;
	int r, g, b;

	r = y + (1.370705 * (v-128));
	g = y - (0.698001 * (v-128)) - (0.337633 * (u-128));
	b = y + (1.732446 * (u-128));

	if(r > 255) r = 255;
	if(g > 255) g = 255;
	if(b > 255) b = 255;
	if(r < 0) r = 0;
	if(g < 0) g = 0;
	if(b < 0) b = 0;

	pixel[0] = r * 220 / 256;
	pixel[1] = g * 220 / 256;
	pixel[2] = b * 220 / 256;

	return pixel32;
}


int convert_yuv_to_rgb_buffer(unsigned char *yuv, 
                              unsigned char *rgb, 
                              unsigned int width, 
                              unsigned int height)
{
	unsigned int in, out = 0;
	unsigned int pixel_16;
	unsigned char pixel_24[3];
	unsigned int pixel32;
	int y0, u, y1, v;

	for(in = 0; in < width * height * 2; in += 4) {
		pixel_16 =
			yuv[in + 3] << 24 |
			yuv[in + 2] << 16 |
			yuv[in + 1] <<  8 |
			yuv[in + 0];

		y0 = (pixel_16 & 0x000000ff);
		u  = (pixel_16 & 0x0000ff00) >>  8;
		y1 = (pixel_16 & 0x00ff0000) >> 16;
		v  = (pixel_16 & 0xff000000) >> 24;

		pixel32 = convert_yuv_to_rgb_pixel(y0, u, v);
		pixel_24[0] = (pixel32 & 0x000000ff);
		pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
		pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;

		rgb[out++] = pixel_24[0];
		rgb[out++] = pixel_24[1];
		rgb[out++] = pixel_24[2];

		pixel32 = convert_yuv_to_rgb_pixel(y1, u, v);
		pixel_24[0] = (pixel32 & 0x000000ff);
		pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
		pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;

		rgb[out++] = pixel_24[0];
		rgb[out++] = pixel_24[1];
		rgb[out++] = pixel_24[2];
	}

	return 0;
}

#ifndef __GNUC__
#error "No gnu C compiler! Things would not work properly. Write to mailing list!"
#endif

#define swap(a,b)   { \
                    __typeof__(a) tmp = a; \
                    a=b; \
                    b=tmp; \
                    }
                   

int make_bmp(uint8_t *rgb_buffer, 
             const char *file_name,
             uint32_t width,
             uint32_t height)
{
    FILE *fd;
    size_t data_size;
    bmpfile_magic_t bmpfile_magic;
    bmpfile_header_t bmpfile_header;
    bmp_dib_v3_header_t bmpdib_header;
    uint32_t i,j;

    if( b_verbose )
        printf("Making bmp: make_bmp(%s)\n", file_name);

    /* create the file first */
    fd = fopen(file_name, "wb");

    if( fd == NULL )
    {
        perror("make_bmp");
        return -1;
    }

    data_size = width*height*3;

    /* fill in the magic */
    bmpfile_magic.magic[0] = BMP_MAGIC_0;
    bmpfile_magic.magic[1] = BMP_MAGIC_1;

    /* fill in the header */
    bmpfile_header.filesz = sizeof(bmpfile_magic_t) +
                            sizeof(bmpfile_header_t) +
                            sizeof(bmp_dib_v3_header_t) +
                            width*height*3;
    bmpfile_header.creator1 = BMP_CREATOR1;
    bmpfile_header.creator2 = BMP_CREATOR2;
    bmpfile_header.bmp_offset = sizeof(bmpfile_magic_t) +
                                sizeof(bmpfile_header_t) +
                                sizeof(bmp_dib_v3_header_t);

    /* fill in the dib V3 header */
    bmpdib_header.header_sz = sizeof(bmp_dib_v3_header_t);
    bmpdib_header.width = width;
    bmpdib_header.height = height;
    bmpdib_header.nplanes = 1;
    bmpdib_header.bitspp = 24;
    bmpdib_header.compress_type = 0;
    bmpdib_header.bmp_bytesz = data_size;
    bmpdib_header.hres = BMP_DEFAULT_HRES;
    bmpdib_header.vres = BMP_DEFAULT_VRES;
    bmpdib_header.ncolors = 0;
    bmpdib_header.nimpcolors = 0;

    /* this is what is happening here:
     * i) lines are swapped - bottom with top, bottom-1 with top+1, etc...
     * ii) the RED and BLUE bytes are swapped for each pixes
     * iii) all this due to wierd bmp specs... */
    for(i=0; i<width; i++)
    {
        uint32_t curpix = i*3;
        for(j=0; j < height/2; j++)
        {            
            /* swap R for B */
            swap(rgb_buffer[j*(width*3) + curpix],
                 rgb_buffer[(height-j-1)*(width*3) + curpix+2]);
            /* leave G */
            swap(rgb_buffer[j*(width*3) + curpix + 1],
                 rgb_buffer[(height-j-1)*(width*3) + curpix+1]);
            /* swap B for R */
            swap(rgb_buffer[j*(width*3) + curpix + 2],
                 rgb_buffer[(height-j-1)*(width*3) + curpix]);
        }
    }

    if( fwrite(&bmpfile_magic, sizeof(bmpfile_magic_t), 1, fd) < 1)
    {
        goto ERROR;
    }

    if( fwrite(&bmpfile_header, sizeof(bmpfile_header_t), 1, fd) < 1)
    {
        goto ERROR;
    }

    if( fwrite(&bmpdib_header, sizeof(bmp_dib_v3_header_t), 1, fd) < 1)
    {
        goto ERROR;
    }

    if( fwrite(rgb_buffer, 1, data_size, fd) < data_size )
        goto ERROR;

    fclose(fd);
    return 0;

ERROR:
    fprintf(stderr, "Error writing to the file.\n");
    fclose(fd);
    return -1;
}

int make_rgb(uint8_t *rgb_buffer, 
             const char *file_name,
             uint32_t width,
             uint32_t height)
{
    FILE *fd;
    size_t data_size;

    if( b_verbose )
        printf("Making rgb: make_rgb(%s)\n", file_name);

    /* create the file first */
    fd = fopen(file_name, "wb");

    if( fd == NULL )
    {
        perror("make_rgb");
        return -1;
    }

    data_size = width*height*3;

    if( fwrite(rgb_buffer, 1, data_size, fd) < data_size )
        {
        fprintf(stderr, "Error writing the %s file.\n", file_name);
        return -1;   
    }

    fclose(fd);
    return 0;
}


