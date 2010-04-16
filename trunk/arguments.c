/*  Arguments handling.
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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "arguments.h"

char psz_video_dev[MAX_LEN_VIDEO_DEV];
char psz_output_dir[MAX_LEN_OUTDIR];
char psz_named_pipe[MAX_LEN_NAMEDPIPE];
int b_verbose = 0,
    b_printinfo = 0,
    b_named_pipe = 0,
    b_shared_mem = 0;

int shared_mem_key;

uint32_t req_width, 
         req_height;

format_t e_outfmt;
char str_formats[FORMAT_LAST][STR_FORMAT_LEN];

static void print_help(int argc, char **argv)
{
	printf("\n\t\t%s - Linux console webcam screenshot utility\n\n", argv[0]);
	printf("Usage: %s [OPTIONS]\n", argv[0]);
	printf("\nOptions:\n");
	printf("\t--help\t\t-h\tPrints this help.\n");
	printf("\t--device dev\t-d dev\tWorks with the dev device (default /dev/video0)\n");
	printf("\t--outdir dir\t-o dir\tSaves all output in dir (default /tmp)\n");
	printf("\t--format fmt\t-f fmt\tSaves images in the fmt format (see below)\n");
    printf("\t--info\t\t-i\tPrints info about the video device and stops.\n");
    printf("\t--width num\t-W num\tSets the desired width (if the camera supports it)\n");
    printf("\t--height num\t-H num\tSets the desired height (if the camera supports it)\n");
    printf("\t--pipe file\t-p file\tUses named pipe file as output for images\n");
    printf("\t--shm id\t-s id\tUses shared memory as output for images. See below for more info\n");

	printf("\nImage formats (fmt):\n");
/*	printf("\tpng - png compressed image (default)\n"); */
	printf("\trgb - raw rqb binary file\n");
	printf("\tbmp - bmp image (default)\n");
    printf("\nOption --shm id / -s id:\n");
    printf("\tIf you choose this option camshot will store the images from the camera to a region of memory which it will share with a key id so a users process can use this region for its input. The writes to this region are protected  by a semaphore with the same key as the memory region. It is important that the user process uses this semaphore while reading the buffer!\n");
	printf("\nAuthor:\n");
	printf("\tThis software was written by Gabriel Zabusek (gabriel.zabusek@gmail.com)\n");
}

int process_arguments(int argc, char **argv)
{
	int processing_done = 0;
	
	/* set the default values */
	strncpy(psz_video_dev, DEFAULT_VIDEO_DEV, MAX_LEN_VIDEO_DEV);
	strncpy(psz_output_dir, DEFAULT_OUTDIR, MAX_LEN_OUTDIR);
	e_outfmt = FORMAT_BMP;
	strncpy(str_formats[FORMAT_JPG], "jpg", STR_FORMAT_LEN);
	strncpy(str_formats[FORMAT_PNG], "png", STR_FORMAT_LEN);
	strncpy(str_formats[FORMAT_RGB], "rgb", STR_FORMAT_LEN);
	strncpy(str_formats[FORMAT_BMP], "bmp", STR_FORMAT_LEN);
	strncpy(str_formats[FORMAT_RAW], "raw", STR_FORMAT_LEN);
    req_width = DEFAULT_WIDTH;
    req_height = DEFAULT_HEIGHT;

	while( !processing_done )
	{
		static struct option opts[] = \
		{
			{"help", no_argument, NULL, 'h'},
			{"device", required_argument, NULL, 'd'},
			{"outdir", required_argument, NULL, 'o'},
			{"verbose", no_argument, NULL, 'v'},
			{"format", required_argument, NULL, 'f'},
            {"info", no_argument, NULL, 'i'},
            {"width", required_argument, NULL, 'W'},
            {"height", required_argument, NULL, 'H'},
            {"pipe", required_argument, NULL, 'p'},
            {"shm", required_argument, NULL, 's'},
			{0,0,0,0}
		};

		int current_opt = getopt_long(argc, argv, "hd:o:vf:iW:H:p:s:", opts, NULL);

		switch(current_opt)
		{
			case 'h':
				print_help(argc, argv);
				exit(0);
				break;
			case 'd':
				strncpy(psz_video_dev, optarg, MAX_LEN_VIDEO_DEV);
				break;
			case 'o':
				strncpy(psz_output_dir, optarg, MAX_LEN_OUTDIR);
				break;
			case 'v':
				b_verbose = 1;
				break;
			case 'f':
				if( !strncmp(str_formats[FORMAT_PNG], optarg, STR_FORMAT_LEN) )
					e_outfmt = FORMAT_PNG;
				else if( !strncmp(str_formats[FORMAT_RGB], optarg, STR_FORMAT_LEN) )
					e_outfmt = FORMAT_RGB;
				else if( !strncmp(str_formats[FORMAT_BMP], optarg, STR_FORMAT_LEN) )
					e_outfmt = FORMAT_BMP;
				break;
            case 'i':
                b_printinfo = 1;
                break;
            case 'W':
                req_width = atoi(optarg);
                break;
            case 'H':
                req_height = atoi(optarg);
                break;
            case 'p':
                strncpy(psz_named_pipe, optarg, MAX_LEN_NAMEDPIPE);
                b_named_pipe = 1;
                break;
            case 's':
                shared_mem_key = atoi(optarg);
                b_shared_mem = 1;
                break;
			case '?':
				print_help(argc, argv);
				exit(-1);
			case EOF:
				processing_done = 1;
				break;
		}
	}

	return 0;
}
