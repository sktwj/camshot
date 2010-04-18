#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#include <stdint.h>

#include "shmem.h"
#include "image.h"
#include "arguments.h"

int main(int argc, char **argv)
{
    int shmid;
    int shared_mem_key;
    int shm_sem;
    uint8_t *p_shm;

    b_verbose = 1;

    if(argc != 4)
    {
        printf("Usage: %s shm_id W H\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    shared_mem_key = atoi(argv[1]);

    if((shmid = shmget(shared_mem_key, atoi(argv[2])*atoi(argv[3])*3, 0666)) < 0) {
        perror("Error getting shared memory id");
        exit(EXIT_FAILURE);
    }

    if((p_shm = (uint8_t *)shmat(shmid, NULL, 0)) == (void *) -1) {
        perror("Error getting shared memory ptr");
        exit(EXIT_FAILURE);
    }       

    shm_sem = semget((key_t)shared_mem_key, 1, 0666);
    sem_set(&shm_sem);

    sem_down(&shm_sem);
        make_bmp(p_shm,"./test_shm.bmp",atoi(argv[2]),atoi(argv[3]));
    sem_up(&shm_sem);

    return 0;
}
