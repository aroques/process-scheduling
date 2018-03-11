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

int get_duration();

int main (int argc, char *argv[]) {
    srand(time(NULL) ^ getpid());
    // Used to calculate work/time per round
    struct timeval tv_start, tv_stop;
    int total_diffusec = 0, total_diffnano = 0;

    // Shared memory structures
    int sysclock_id = atoi(argv[SYSCLOCK_ID_IDX]);
    int proc_ctrl_tbl_id = atoi(argv[PCT_ID_IDX]);
    struct sysclock sysclock;
    sysclock.mtype = 1;
    struct termlog termlog;
    termlog.mtype = 1;

    // Other variables
    int duration = get_duration();  // Total duration to run for
    int time_incremented = 0;       /* Total simulated time that this process has
                                        incremented simulated clock while running */
    int new_nano = 0;               // Number of nanoseconds to increment simulated clock

    gettimeofday(&tv_start, NULL);
    while(1) {
        // Receive
        //read_clock(sysclock_id, &sysclock);

        // Critical Section //

        // Get quantity of work
        gettimeofday(&tv_stop, NULL);

        // Calculate nano-seconds to increment sysclock
        total_diffusec = tv_stop.tv_usec - tv_start.tv_usec;
        total_diffnano = total_diffusec * 1000;
        new_nano = total_diffnano - time_incremented;

        time_incremented += new_nano;

        // Send
        if (time_incremented >= duration) {
            // Terminate and send message to master

            // Calculated corrected new_nano
            time_incremented -= new_nano;
            new_nano = duration - time_incremented;

            increment_clock(&sysclock.clock, new_nano);

            // Set termination log information
            termlog.termtime.seconds = sysclock.clock.seconds;
            termlog.termtime.nanoseconds = sysclock.clock.nanoseconds;
            termlog.pid = getpid();
            termlog.duration = duration;

            // Send
            //update_clock(sysclock_id, &sysclock);
            update_termlog(proc_ctrl_tbl_id, &termlog);
            break;
        }
        else {
            increment_clock(&sysclock.clock, new_nano);
            // Send
            //update_clock(sysclock_id, &sysclock);
        }
    }
    printf("hello world\n");
    return 0;  
}

int get_duration() {
    return rand() % 100;
}
