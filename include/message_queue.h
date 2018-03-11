#ifndef MEESAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#define PROC_CTRL_TBL_SZE 20
#define ONE_BILLION 1000000000

struct clock {
    unsigned int seconds;
    unsigned int nanoseconds;
};

struct process_ctrl_block {
    pid_t pid;
    unsigned int priority;
    struct clock cpu_time_used;
    struct clock sys_time_used;
    struct clock last_run_time_used;
};

// Message Queues
struct sysclock {
    long mtype;
    struct clock clock;
};

struct process_ctrl_table {
    long mtype;
    struct process_ctrl_block pcb[PROC_CTRL_TBL_SZE];
};

struct termlog {
    long mtype;
    pid_t pid;
    struct clock termtime;
    unsigned int duration;
};

int get_message_queue();
void remove_message_queue(int msgqid);
void read_clock(int msgqid, struct sysclock* rbuf);
void update_clock(int msgqid, struct sysclock* sbuf);
void read_termlog(int msgqid, struct termlog* rbuf);
void update_termlog(int msgqid, struct termlog* sbuf);
void increment_clock(struct clock* clock, int increment);

#endif
