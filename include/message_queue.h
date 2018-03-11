#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include "global_structs.h"

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
