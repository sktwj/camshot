/*  The IPC semaphore handling.
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

#ifndef __SHMEM_H
#define __SHMEM_H

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

/* initializes semaphore *id */
extern int sem_set(int *id);

/* deinitializes semaphore *id */
extern int sem_del(int *id);

/* decreases the semaphore */
extern int sem_down(int *id);

/* sets the semaphore back to 1 */
extern int sem_up(int *id);

#endif

