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

#include <stdio.h>
#include <linux/videodev2.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include "arguments.h"
#include "camera.h"

int camera_fd;
struct image_buffer *image_buffers;
struct v4l2_format camera_format;

static int total_buffers;
static struct v4l2_capability caps;

int req_mmap_buffers(int num_buffers)
{
	int i=0;
	struct v4l2_requestbuffers reqbuf;

	if( b_verbose )
	{
		printf("Requesting %d buffers.\n", num_buffers);
	}

	memset(&reqbuf, 0, sizeof(reqbuf));

	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	reqbuf.count = num_buffers;

	if( ioctl(camera_fd, VIDIOC_REQBUFS, &reqbuf) < 0 )
	{
		if( errno == EINVAL )
			printf("Video capturing or mapped memory streaming not supported!\n");
		else
			perror("req_mmap_buffers (VIDIOC_REQBUFS)");

		exit(EXIT_FAILURE);
	}

	total_buffers = reqbuf.count;

	/* exit if there is not enough memory for 2 buffers */
	if( total_buffers < 2 )
	{
		printf("ERROR: Unable to allocate memory for at least 2 image buffers.\n");
		exit(EXIT_FAILURE);
	}

	/* allocate the memory needed for our structure */
	image_buffers = (struct image_buffer *)calloc(total_buffers, sizeof(*image_buffers));
	assert(image_buffers != NULL);

	for(i=0; i<total_buffers; i++)
	{
        query_map_buffer(i);
	}

	if( b_verbose )
	{
		printf("Mapped %u buffers.\n", total_buffers);
	}

	return total_buffers;
}

void unmap_buffers(void)
{
	int i;
	for(i=0; i<total_buffers; i++)
	{
		munmap(image_buffers[i].start, image_buffers[i].length);
	}
}

void queue_buffer(uint32_t idx)
{
	struct v4l2_buffer qbuf;
	qbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	qbuf.memory = V4L2_MEMORY_MMAP;
	qbuf.index = idx;

	if( ioctl(camera_fd, VIDIOC_QBUF, &qbuf) < 0 )
	{
        switch(errno)
        {
            case EINVAL:
                printf("Type not supported, index out of bounds or no buffers allocated.\n");
                break;
            default:
                perror("queue_buffer (VIDIOC_QBUF)");
                break;
        }
		
	}
}

uint32_t dequeue_buffer(void)
{
	struct v4l2_buffer qbuf;
	qbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	qbuf.memory = V4L2_MEMORY_MMAP;

	if( ioctl(camera_fd, VIDIOC_DQBUF, &qbuf) < 0 )
	{
		perror("dequeue_buffer (VIDIOC_DQBUF)");
	}

	return qbuf.index;
}

/* queues all buffers and starts the streaming */
void streaming_on(void)
{
	int i;
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	for(i=0; i<total_buffers; i++)
		queue_buffer(i);

	if( ioctl(camera_fd, VIDIOC_STREAMON, &type) < 0 )
		perror("streaming_on (VIDIOC_STREAMON)");
}

void streaming_off(void)
{
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if( ioctl(camera_fd, VIDIOC_STREAMOFF, &type) < 0 )
		perror("streaming_off (VIDIOC_STREAMOFF)");
}

void get_caps(void)
{
    memset(&caps, 0, sizeof(caps));
	if( ioctl(camera_fd, VIDIOC_QUERYCAP, &caps) == -1 )
	{
		perror("get_caps (VIDIOC_QUERYCAP)");
	}
}

void print_caps(void)
{
	printf("\tDriver: %s\n", caps.driver);
	printf("\tCard: %s\n", caps.card);
	printf("\tBus: %s\n", caps.bus_info);
}

void get_format(void)
{
    memset(&camera_format, 0, sizeof(camera_format));

    /* set the type - needed so we can get the structure */
    camera_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if( ioctl(camera_fd, VIDIOC_G_FMT, &camera_format) == -1 )
    {
        perror("get_format (VIDIOC_G_FMT)");
    }
}

int check_pixelformat(void)
{
    switch(camera_format.fmt.pix.pixelformat)
    {
        case V4L2_PIX_FMT_YUYV:
            return V4L2_PIX_FMT_YUYV;
        default:
            return -1;
    }

    return -1;
}

void print_pixelformat(FILE *fd)
{
    switch( check_pixelformat() )
    {
        case V4L2_PIX_FMT_YUYV:
            fprintf(fd,"YUYV");
            break;
        default:
            fprintf(fd,"Unsupported pixelformat! Please do write to the mailing list!");
            break;
    }
}

void print_format(void)
{
    printf("\tWidth: %u\n", camera_format.fmt.pix.width);
    printf("\tHeight: %u\n", camera_format.fmt.pix.height);
    printf("\tPixel format: "); print_pixelformat(stdout); printf("\n");
    printf("\tBytes per line: %u\n", camera_format.fmt.pix.bytesperline);
}

int set_width_height(uint32_t w, uint32_t h)
{
    get_format();

    camera_format.fmt.pix.width = w;
    camera_format.fmt.pix.height = h;

    if( ioctl(camera_fd, VIDIOC_S_FMT, &camera_format) == -1 )
    {
        perror("get_format (VIDIOC_G_FMT)");
        return -1;
    }   

    return 0;    
}

void query_map_buffer(uint32_t idx)
{
    struct v4l2_buffer buffer;
	buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buffer.memory = V4L2_MEMORY_MMAP;
	buffer.index = idx;

	if( ioctl(camera_fd, VIDIOC_QUERYBUF, &buffer) < 0 )
	{
		perror("req_mmap_buffers (VIDIOC_QUERYBUF)");
		exit(EXIT_FAILURE);
	}

	image_buffers[idx].length = buffer.length;
	image_buffers[idx].start = mmap(NULL, buffer.length,
					PROT_READ | PROT_WRITE,
					MAP_SHARED,
					camera_fd, buffer.m.offset);

	if( image_buffers[idx].start == MAP_FAILED )
	{
		perror("mmap");
		exit(EXIT_FAILURE);
	}    
}

struct timeval query_buffer(uint32_t idx)
{
        struct v4l2_buffer buffer;
	buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buffer.memory = V4L2_MEMORY_MMAP;
	buffer.index = idx;

	if( ioctl(camera_fd, VIDIOC_QUERYBUF, &buffer) < 0 )
	{
		perror("req_mmap_buffers (VIDIOC_QUERYBUF)");
		exit(EXIT_FAILURE);
	}

    return buffer.timestamp;
}
