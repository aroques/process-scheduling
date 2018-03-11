#include <sys/stat.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "shared_memory.h"

int get_shared_memory() {
    int shmemid;

    shmemid = shmget(IPC_PRIVATE, getpagesize(), IPC_CREAT | S_IRUSR | S_IWUSR);

    if (shmemid == -1) {
        perror("shmget");
        exit(1);
    }
    
    return shmemid;
}

struct clock* attach_to_clock(int shmemid, unsigned int readonly) {
    struct clock* p;
    int shmflag;

    if (readonly) {
        shmflag = SHM_RDONLY;
    }
    else {
        shmflag = 0;
    }

    p = (struct clock*)shmat(shmemid, 0, shmflag);

    if (!p) {
        perror("shmat");
        exit(1);
    }

    return p;

}

void cleanup_shared_memory(int shmemid, struct clock* p) {
    detach_from_clock(p);
    deallocate_shared_memory(shmemid);
}

void detach_from_clock(struct clock* p) {
    if (shmdt(p) == -1) {
        perror("shmdt");
        exit(1);
    }
}

void deallocate_shared_memory(int shmemid) {
    if (shmctl(shmemid, IPC_RMID, 0) == 1) {
        perror("shmctl");
        exit(1);
    }
}

void increment_clock(struct clock* clock, int increment) {
    clock->nanoseconds += increment;
    if (clock->nanoseconds >= ONE_BILLION) {
        clock->seconds += 1;
        clock->nanoseconds -= ONE_BILLION;
    }
}
