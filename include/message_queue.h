#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#define PROC_CTRL_TBL_SZE 18

#include "global_structs.h"

struct process_ctrl_block {
    int pid;
    unsigned int is_realtime;
    struct clock cpu_time_used;
    struct clock sys_time_used;
    struct clock last_run_time_used;
    struct clock time_unblocked;
};

struct sysclock {
    long mtype;
    struct clock clock;
};

struct process_ctrl_table {
    long mtype;
    struct process_ctrl_block pcbs[PROC_CTRL_TBL_SZE];
};

struct termlog {
    long mtype;
    pid_t pid;
    struct clock termtime;
    unsigned int duration;
};

int get_message_queue();
void remove_message_queue(int msgqid);
void read_termlog(int msgqid, struct termlog* rbuf);
void update_termlog(int msgqid, struct termlog* sbuf);

#endif
