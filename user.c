#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/time.h>
#include <locale.h>

#include "global_constants.h"
#include "helpers.h"
#include "message_queue.h"
#include "shared_memory.h"

unsigned int determine_if_terminate();
unsigned int determine_if_use_entire_timeslice();
unsigned int get_random_pct();

const unsigned int CHANCE_TERMINATE = 5;
const unsigned int CHANCE_ENTIRE_TIMESLICE = 20;
const unsigned int CHANCE_BLOCKED_ON_EVENT = 50;

int main (int argc, char *argv[]) {
    srand(time(NULL) ^ getpid());
    unsigned int will_terminate, use_entire_timeslice;

    // Get shared memory IDs
    int sysclock_id = atoi(argv[SYSCLOCK_ID_IDX]);
    int proc_ctrl_tbl_id = atoi(argv[PCT_ID_IDX]);
    int pid = atoi(argv[PID_IDX]);
    int scheduler_id = atoi(argv[SCHEDULER_IDX]);
    
    // Attach to shared memory
    struct clock* sysclock = attach_to_shared_memory(sysclock_id, 1);
    struct process_ctrl_table* proc_ctrl_tbl = attach_to_shared_memory(proc_ctrl_tbl_id, 0);
    
    struct msgbuf scheduler;

    while(1) {
        // Blocking receive
        receive_msg(scheduler_id, pid, &scheduler);
        // Has been scheduled
        will_terminate = determine_if_terminate();

        use_entire_timeslice = determine_if_use_entire_timeslice();
        
        // Let oss know we're done
        send_msg(scheduler_id, pid, &scheduler);
    }

    return 0;  
}

unsigned int determine_if_terminate() {
    return event_occured(CHANCE_TERMINATE);
}

unsigned int determine_if_use_entire_timeslice() {
    return event_occured(CHANCE_ENTIRE_TIMESLICE);
}

unsigned int determine_if_block_on_event() {
    return event_occured(CHANCE_BLOCKED_ON_EVENT);
}

unsigned int get_random_pct() {
    return (rand() % 99) + 1;
}

struct clock get_event_wait_time() {
    struct clock event_wait_time;
    event_wait_time.seconds = rand() % 6
    event_wait_time.nanoseconds = rand() % 1001;
    return event_wait_time;
}