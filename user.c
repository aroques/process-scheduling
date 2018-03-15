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
void calculate_sys_time_used(struct process_ctrl_block* pcb);
void set_last_run_time(struct process_ctrl_block* pcb, unsigned int nanoseconds);

const unsigned int CHANCE_TERMINATE = 5;
const unsigned int CHANCE_ENTIRE_TIMESLICE = 20;
const unsigned int CHANCE_BLOCKED_ON_EVENT = 50;

int main (int argc, char *argv[]) {
    srand(time(NULL) ^ getpid());
    bool will_terminate, use_entire_timeslice;
    unsigned int nanosecs;

    // Get shared memory IDs
    int sysclock_id = atoi(argv[SYSCLOCK_ID_IDX]);
    int proc_ctrl_tbl_id = atoi(argv[PCT_ID_IDX]);
    int pid = atoi(argv[PID_IDX]);
    int scheduler_id = atoi(argv[SCHEDULER_IDX]);
    
    // Attach to shared memory
    struct clock* sysclock = attach_to_shared_memory(sysclock_id, 1);
    struct process_ctrl_table* pct = attach_to_shared_memory(proc_ctrl_tbl_id, 0);

    struct process_ctrl_block* pcb = &pct->pcbs[pid];
    
    struct msgbuf scheduler;
    while(1) {
        // Blocking receive - wait until scheduled
        receive_msg(scheduler_id, &scheduler, pid);
        // Received message from OSS telling me to run
        pcb->status = RUNNING;

        will_terminate = determine_if_terminate();
        
        if (will_terminate) {
            // Run for some random pct of time quantum
            nanosecs = pcb->time_quantum / get_random_pct();
            set_last_run_time(pcb, nanosecs);
            increment_clock(&pcb->cpu_time_used, nanosecs);

            break;
        }

        use_entire_timeslice = determine_if_use_entire_timeslice();

        if (use_entire_timeslice) {
            // Run for entire time slice and do not get blocked
            increment_clock(&pcb->cpu_time_used, pcb->time_quantum);
            set_last_run_time(pcb, pcb->time_quantum);

            pcb->status = READY;
        }
        else {
            // Blocked on an event
            pcb->status = BLOCKED;

            // Run for some random pct of time quantum
            nanosecs = pcb->time_quantum / get_random_pct(); 
            set_last_run_time(pcb, nanosecs);
            increment_clock(&pcb->cpu_time_used, nanosecs);

            pcb->time_blocked = get_event_wait_time();

            // Set the time when this process is unblocked
            pcb->time_unblocked.seconds = pcb->time_blocked.seconds + sysclock->seconds;
            pcb->time_unblocked.nanoseconds = pcb->time_blocked.nanoseconds + sysclock->nanoseconds;
        }
        
        // Add PROC_CTRL_TBL_SZE to message type to let OSS know we are done
        send_msg(scheduler_id, &scheduler, (pid + PROC_CTRL_TBL_SZE)); 
    }

    pcb->status = TERMINATED;

    pcb->time_finished.seconds = sysclock->seconds;
    pcb->time_finished.nanoseconds = sysclock->nanoseconds;
    
    calculate_sys_time_used(pcb);

    // Add PROC_CTRL_TBL_SZE to message type to let OSS know we are done
    send_msg(scheduler_id, &scheduler, (pid + PROC_CTRL_TBL_SZE)); 

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

void calculate_sys_time_used(struct process_ctrl_block* pcb) {
    pcb->sys_time_used.seconds = pcb->time_finished.seconds - pcb->time_scheduled.seconds;
    pcb->sys_time_used.nanoseconds = pcb->time_finished.seconds - pcb->time_scheduled.seconds; 
}

void set_last_run_time(struct process_ctrl_block* pcb, unsigned int nanoseconds) {
    pcb->last_run.seconds = 0;
    pcb->last_run.nanoseconds = 0;
    increment_clock(&pcb->last_run, nanoseconds); 
}