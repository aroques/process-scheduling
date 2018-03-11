#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>

#include "message_queue.h"

int get_message_queue() {
    int qid;

    qid = msgget(IPC_PRIVATE, IPC_CREAT | 0666);

    if (qid == -1) {
        perror("msgget");
        exit(1);
    }

    return qid;
}

void remove_message_queue(int msgqid) {
    if (msgctl(msgqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }
}

void read_clock(int msgqid, struct sysclock* rbuf) {
    if (msgrcv(msgqid, rbuf, sizeof(rbuf->clock), 1, 0) == -1) {
        perror("msgrcv");
        exit(1);
    }
}

void read_termlog(int msgqid, struct termlog* rbuf) {
    if (msgrcv(msgqid, rbuf, sizeof(*rbuf), 1, 0) == -1) {
        perror("msgrcv");
        exit(1);
    }
}

void update_clock(int msgqid, struct sysclock* sbuf) {
    if (msgsnd(msgqid, sbuf, sizeof(sbuf->clock), IPC_NOWAIT) < 0) {
        perror("msgsnd");
        exit(1);
    }
}

void update_termlog(int msgqid, struct termlog* sbuf) {
    if (msgsnd(msgqid, sbuf, sizeof(*sbuf), IPC_NOWAIT) < 0) {
        perror("msgsnd");
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
