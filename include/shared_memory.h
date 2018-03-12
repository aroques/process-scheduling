#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#define PROC_CTRL_TBL_SZE 18
#define ONE_BILLION 1000000000

#include "global_structs.h"

struct process_ctrl_block {
    int pid;
    unsigned int is_realtime;
    unsigned int time_quantum;
    struct clock cpu_time_used;
    struct clock sys_time_used;
    struct clock last_run_time_used;
    struct clock time_unblocked;
    struct clock time_scheduled;
    struct clock time_finished;
};

struct process_ctrl_table {
    struct process_ctrl_block pcbs[PROC_CTRL_TBL_SZE];
};

int get_shared_memory();
void* attach_to_shared_memory(int shmemid, unsigned int readonly);
void cleanup_shared_memory(int shmemid, void* p);
void detach_from_shared_memory(void* p);
void deallocate_shared_memory(int shmemid);

#endif
