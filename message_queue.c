#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>

#include "message_queue.h"

int get_message_queue() {
    int msgqid;

    msgqid = msgget(IPC_PRIVATE, IPC_CREAT | 0666);

    if (msgqid == -1) {
        perror("msgget");
        exit(1);
    }

    return msgqid;
}

void receive_msg(int msgqid, struct msgbuf* mbuf, int mtype) {
    if (msgrcv(msgqid, mbuf, sizeof(mbuf->mtext), mtype, 0) == -1) {
        perror("msgrcv");
        exit(1);
    }
}

void send_msg(int msgqid, struct msgbuf* mbuf, int mtype) {
    mbuf->mtype = mtype;
    if (msgsnd(msgqid, mbuf, sizeof(mbuf->mtext), IPC_NOWAIT) < 0) {
        perror("msgsnd");
        exit(1);
    }
}

void remove_message_queue(int msgqid) {
    if (msgctl(msgqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }
}
