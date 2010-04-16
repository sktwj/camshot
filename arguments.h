/*  The arguments handling.
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

#ifndef __ARGUMENTS_H
#define __ARGUMENTS_H

#include <stdint.h>

#define MAX_LEN_VIDEO_DEV 128
#define MAX_LEN_OUTDIR 2048
#define MAX_LEN_NAMEDPIPE 2048

#define DEFAULT_VIDEO_DEV "/dev/video0"
#define DEFAULT_OUTDIR "/tmp/"
#define DEFAULT_WIDTH 320
#define DEFAULT_HEIGHT 240

#define STR_FORMAT_LEN 4

typedef enum format_e
{
	FORMAT_JPG,
	FORMAT_PNG,
	FORMAT_BMP,
	FORMAT_RGB,
	FORMAT_RAW,
	FORMAT_LAST
} format_t;

extern char str_formats[FORMAT_LAST][STR_FORMAT_LEN];

extern char psz_video_dev[MAX_LEN_VIDEO_DEV];
extern char psz_output_dir[MAX_LEN_OUTDIR];
extern char psz_named_pipe[MAX_LEN_NAMEDPIPE];
extern int b_verbose,
           b_printinfo,
           b_named_pipe,
           b_shared_mem;

extern int shared_mem_key;

extern uint32_t req_width, 
                req_height;

extern format_t e_outfmt;

/* processes the arguments given to the program */
extern int process_arguments(int argc, char **argv);

#endif
