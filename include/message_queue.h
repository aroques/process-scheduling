#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include "global_structs.h"

#define MSGSZ 2

struct msgbuf {
    long mtype;
    char mtext[MSGSZ];
};

int get_message_queue();
void remove_message_queue(int msgqid);
void receive_msg(int msgqid, int mtype, struct msgbuf* s);
void send_msg(int msgqid, int mtype, struct msgbuf* s);

#endif
