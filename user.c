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
#include "clock.h"

bool determine_if_terminate();
bool determine_if_use_entire_timeslice();
unsigned int get_random_pct();
struct clock get_event_wait_time();

const unsigned int CHANCE_TERMINATE = 5;
const unsigned int CHANCE_ENTIRE_TIMESLICE = 20;
const unsigned int CHANCE_BLOCKED_ON_EVENT = 50;

int main (int argc, char *argv[]) {
    srand(time(NULL) ^ getpid());
    bool will_terminate, use_entire_timeslice;
    unsigned int amt_work;
    struct clock event_wait_time;

    // Get shared memory IDs
    int sysclock_id = atoi(argv[SYSCLOCK_ID_IDX]);
    int proc_ctrl_tbl_id = atoi(argv[PCT_ID_IDX]);
    int pid = atoi(argv[PID_IDX]);
    int scheduler_id = atoi(argv[SCHEDULER_IDX]);
    
    // Attach to shared memory
    struct clock* sysclock = attach_to_shared_memory(sysclock_id, 1);
    struct process_ctrl_table* pct = attach_to_shared_memory(proc_ctrl_tbl_id, 0);
    
    struct msgbuf scheduler;
    while(1) {
        // Blocking receive - wait until scheduled
        receive_msg(scheduler_id, &scheduler, pid);
        // Received message from OSS telling me to run
        pct->pcbs[pid].status = RUNNING;

        will_terminate = determine_if_terminate();
        
        if (will_terminate) {
            // I'm terminating
            pct->pcbs[pid].status = TERMINATED;
            
            // Run for some random pct of time quantum
            amt_work = pct->pcbs[pid].time_quantum / get_random_pct(); 
            increment_clock(&pct->pcbs[pid].last_run, amt_work);

            // Let OSS know were done 
            send_msg(scheduler_id, &scheduler, (pid + PROC_CTRL_TBL_SZE)); // Add PROC_CTRL_TBL_SZE to message type to let OSS know we are done
            
            break;
        }

        use_entire_timeslice = determine_if_use_entire_timeslice();

        if (use_entire_timeslice) {
            // Run for entire time slice and do not get blocked
            increment_clock(&pct->pcbs[pid].last_run, pct->pcbs[pid].time_quantum);
            pct->pcbs[pid].status = READY;
        }
        else {
            // Blocked on an event
            pct->pcbs[pid].status = BLOCKED;

            // Run for some random pct of time quantum
            amt_work = pct->pcbs[pid].time_quantum / get_random_pct(); 
            increment_clock(&pct->pcbs[pid].last_run, amt_work);

            event_wait_time = get_event_wait_time();

            // Set the time when this process is unblocked
            pct->pcbs[pid].time_unblocked.seconds = event_wait_time.seconds + sysclock->seconds;
            pct->pcbs[pid].time_unblocked.nanoseconds = event_wait_time.nanoseconds + sysclock->nanoseconds;
        }
        
        // Let oss know we're done
        send_msg(scheduler_id, &scheduler, (pid + PROC_CTRL_TBL_SZE)); // Add PROC_CTRL_TBL_SZE to message type to let OSS know we are done
    }

    return 0;  
}

bool determine_if_terminate() {
    return event_occured(CHANCE_TERMINATE);
}

bool determine_if_use_entire_timeslice() {
    return event_occured(CHANCE_ENTIRE_TIMESLICE);
}

bool determine_if_block_on_event() {
    return event_occured(CHANCE_BLOCKED_ON_EVENT);
}

unsigned int get_random_pct() {
    return (rand() % 99) + 1;
}

struct clock get_event_wait_time() {
    struct clock event_wait_time;
    event_wait_time.seconds = rand() % 6;
    event_wait_time.nanoseconds = rand() % 1001;
    return event_wait_time;
}