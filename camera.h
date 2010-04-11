/*  The v4l2 camera handling.
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

#ifndef __CAMERA_H
#define __CAMERA_H

#include <stdint.h>
#include <sys/time.h>

#define AUTO_ADJUST_TURNS 10

/* Structure definitions */
struct image_buffer
{
	void *start;
	uint32_t length;
};

/* Global variables */
extern struct image_buffer *image_buffers;
extern int camera_fd;
extern struct v4l2_format camera_format;

/* Function prototypes */

/* Requests num_buffers buffers and maps them.
 * Returns the actual amount of mapped buffers. */
extern int req_mmap_buffers(int num_buffers);

/* Unmaps the previously mapped buffers. */
extern void unmap_buffers(void);

/* Queues the buffer idx */
extern void queue_buffer(uint32_t idx);

/* Dequeues a buffer */
extern uint32_t dequeue_buffer(void);

/* Turn on capture */
extern void streaming_on(void);

/* Turn off capture */
extern void streaming_off(void);

/* Gets the video device capabilities */
extern void get_caps(void);

/* Prints the video device capabilities */
extern void print_caps(void);

/* Gets the camera format data */
extern void get_format(void);

/* Checks the pixelformat -1 if unsupported. */
extern int check_pixelformat(void);

/* Prints the pixelformat */
extern void print_pixelformat(FILE *fd);

/* Sets the capture width & height */
extern int set_width_height(uint32_t w, uint32_t h);

/* Prints the camera format data */
extern void print_format(void);

/* Queries the buffer and mmaps it. 
 * Used after streaming_off has been called to reinit the buffers */
extern void query_map_buffer(uint32_t idx);

/* Only queries the buffer (no mmaping) and returns the timestamp structure */
extern struct timeval query_buffer(uint32_t idx);
#endif
