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

#include "shmem.h"

int sem_set(int *id)
{
    union semun sem_union;
    sem_union.val = 1;

    if( semctl(*id, 0, SETVAL, sem_union) == -1 )
    {
        perror("sem_set");
        return -1;
    }

    return 0;
}

int sem_del(int *id)
{
    union semun sem_union;
    if( semctl(*id, 0, IPC_RMID, sem_union) == -1 )
    {
        perror("sem_del");
        return -1;
    }

    return 0;
}

int sem_down(int *id)
{
    struct sembuf sem_b;

    sem_b.sem_num = 0;
    sem_b.sem_op = -1;
    sem_b.sem_flg = SEM_UNDO;
    
    if( semop(*id, &sem_b, 1) == -1 )
    {
        perror("sem_down");
        return -1;
    }

    return 0;
}

int sem_up(int *id)
{
    struct sembuf sem_b;

    sem_b.sem_num = 0;
    sem_b.sem_op = 1;
    sem_b.sem_flg = SEM_UNDO;
    
    if( semop(*id, &sem_b, 1) == -1 )
    {
        perror("sem_up");
        return -1;
    }
    
    return 0;
}

