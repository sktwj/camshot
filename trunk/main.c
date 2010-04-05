/*  The main file.
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

/*
TODO-Important:
 - add mmap support

TODO
 - add png support
 - add jpg support
*/

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include <sys/time.h>

#include "arguments.h"
#include "camera.h"
#include "image.h"

pthread_mutex_t capture_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cond_mutex    = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition     = PTHREAD_COND_INITIALIZER;

void *capture_func( void *ptr);

int main(int argc, char **argv)
{
	int total_buffers;
    pthread_t capture_thread;
    int i,j;
    char in_str[16];

	process_arguments(argc, argv);

    /* open the camera device */
	if( (camera_fd = open(psz_video_dev, O_RDWR)) < 0 )
	{
		perror("open()");
		exit(-1);
	}

    get_caps();
    get_format();

	if( b_verbose ) printf("Device opened.\n");

    if( b_printinfo )
    {
        printf("Device info:\n");
        print_caps();
        print_format();
        close(camera_fd);
        exit(EXIT_SUCCESS);    
    }

	if( b_verbose )
	{
		printf("Video device:\t\t%s\n", psz_video_dev);
		print_caps();
        print_format();
		printf("Ouput directory:\t%s\n", psz_output_dir);
		printf("Image format:\t\t%s\n",str_formats[e_outfmt]);
		printf("\n");
		printf("Opening device %s\n", psz_video_dev);
	}

    if( req_width && req_height )
    {
        if( b_verbose )
            printf("Trying to set resolution to %ux%u.\n", req_width, req_height);

        if( set_width_height(req_width,req_height) == -1 )
            printf("Unable to set the desired resolution.\n");
        else
            if( b_verbose )
                printf("Resolution set to %ux%u\n", req_width, req_height);
    } else {
        get_format();
        req_width = camera_format.fmt.pix.width;
        req_height = camera_format.fmt.pix.height;
    }

	total_buffers = req_mmap_buffers(5);

	/* start the capture */
	streaming_on();

    /* let the camera self adjust by 'ignoring' 200 complete buffer queues */
    printf("Letting the camera automaticaly adjust the picture:");
    
    for(i=0; i<AUTO_ADJUST_TURNS; i++)
    {
        for(j=0; j<total_buffers; j++)
        {
            int ready_buf = dequeue_buffer();
            /* don't queue the last buffers */
            if( i<AUTO_ADJUST_TURNS-1 )
                queue_buffer(ready_buf);
        }

        printf(".");
        fflush(stdout);
    }

    printf("Done.\n");

    pthread_create(&capture_thread, NULL, &capture_func, NULL);

    while( in_str[0] != 'q' )
    {
        printf("Command (h for help): ");
        fflush(stdout);

        if( fgets(in_str, 16, stdin) == NULL )
        {
            printf("Got NULL! Try again.\n");
            continue;
        }
        

        switch(in_str[0])
        {
            case 'x':
                pthread_mutex_lock(&cond_mutex);
                pthread_cond_signal(&condition);
                pthread_mutex_unlock(&cond_mutex);
                break;
            case 'h':
                printf("\nCommands:\n");
                printf("\tx\tCapture a picture from camera.\n");
                printf("\th\tPrints this help.\n");
                printf("\tq\tQuits the program.\n");
                printf("\n");
                break;
            case 'q':
            case '\n':
                break;
            default:
                fprintf(stderr, "Unknown command %c\n", in_str[0]);
                break;
        }
    }


    /* this is the actual capture/store block - will go to a separate thread */
    /*unsigned char *rgb_buffer = (unsigned char *)malloc(req_width*req_height*3);
	for(int i=0; i<total_buffers; i++)
	{
        // get the idx of ready buffer
        queue_buffer(i);
		int ready_buf = dequeue_buffer();

        if( b_verbose )
        {
            printf("Buffer %d ready. Length: %uB\n", ready_buf, image_buffers[i].length);
        }

        // convert data to rgb
        if( convert_yuv_to_rgb_buffer((unsigned char *)(image_buffers[i].start), 
                                      rgb_buffer, req_width, req_height) == 0 )
        {
            if( b_verbose )
            {
                printf("\tConverted to rgb.\n");
            }
        }

        // create the file name
        char cur_name[64];
        sprintf(cur_name, "%simage_%d.bmp", psz_output_dir, i);

        // make the bmp image
        make_bmp(rgb_buffer, 
                 cur_name, 
                 req_width, 
                 req_height);
	}*/

	/* Clean up */
	streaming_off();
	unmap_buffers();
	close(camera_fd);
	if( b_verbose ) printf("Device closed.\n");

	return 0;
}

void *capture_func(void *ptr)
{
    unsigned char *rgb_buffer = (unsigned char *)malloc(req_width*req_height*3);
    int ready_buf;
    char cur_name[64];
    struct timeval timestamp;

    for(;;)
    {
        /* Wait for the start condition */
        pthread_mutex_lock(&cond_mutex);
        pthread_cond_wait(&condition, &cond_mutex);
        pthread_mutex_unlock(&cond_mutex);

        /* queue one buffer */
        queue_buffer(0);

        /* get the idx of ready buffer */
		ready_buf = dequeue_buffer();

        if( b_verbose )
        {
            printf("Buffer %d ready. Length: %uB\n", ready_buf, image_buffers[0].length);
        }

        /* convert data to rgb */
        if( convert_yuv_to_rgb_buffer((unsigned char *)(image_buffers[0].start), 
                                      rgb_buffer, req_width, req_height) == 0 )
        {
            if( b_verbose )
            {
                printf("\tConverted to rgb.\n");
            }
        }

        timestamp = query_buffer(0);

        /* make the image */

        switch( e_outfmt )
        {
            case FORMAT_BMP:
                /* create the file name */
                sprintf(cur_name, "%scamshot_%lu.bmp", psz_output_dir, timestamp.tv_sec);
                make_bmp(rgb_buffer, 
                         cur_name, 
                         req_width, 
                         req_height);
                break;
            case FORMAT_RGB:
                /* create the file name */
                sprintf(cur_name, "%scamshot_%lu.rgb", psz_output_dir, timestamp.tv_sec);
                make_rgb(rgb_buffer, 
                         cur_name, 
                         req_width, 
                         req_height);
                break;
            default:
                fprintf(stderr, "Not supported format requested!\n");
                break;
        }   
    }

    return NULL;
}


