#include <stdio.h>
#include <sys/wait.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <locale.h>
#include <sys/time.h>
#include <time.h>
#include <sys/queue.h>

#include "global_structs.h"
#include "global_constants.h"
#include "message_queue.h"
#include "helpers.h"
#include "shared_memory.h"
#include "queue.h"

void wait_for_all_children();
void add_signal_handlers();
void handle_sigint(int sig);
void handle_sigalrm(int sig);
void cleanup_and_exit();
void fork_child(char** execv_arr, int child_idx, int pid);
struct clock convertToClockTime(int nanoseconds);
unsigned int get_realtime();
unsigned int get_random_amt_of_ns_worked();
void increment_clock(struct clock* clock, int increment);

// Globals used in signal handler
int simulated_clock_id, proc_ctrl_tbl_id, scheduler_id;
struct clock* sysclock;                                 
struct process_ctrl_table* proc_ctrl_tbl;
int cleaning_up = 0;
pid_t* childpids;
FILE* fp;

int main (int argc, char* argv[]) {
    /*
     *  Setup program before entering main loop
     */
    set_timer(MAX_RUNTIME);                             // Set timer that triggers SIGALRM
    add_signal_handlers();
    setlocale(LC_NUMERIC, "");                          // For comma separated integers in printf
    srand(time(NULL) ^ getpid());

    int i, pid;
    const unsigned int TOTAL_RUNTIME = 3;               // Max seconds oss should run for
    unsigned int pcb_in_use[PROC_CTRL_TBL_SZE] = {0};   // Bit vector used to determine if process ctrl block is in use
    unsigned int proc_count = 0;                        // Number of concurrent children
    unsigned int num_procs_spawned = 0;                 // Total number of children spawned
    unsigned int ns_worked = 0;                         // Holds total time it took to schedule a process
    unsigned int ns_before_next_proc = 0;               // Holds nanoseconds before next processes is scheduled
    struct clock time_to_fork =                         // Holds time to schedule new process
        { .seconds = 0, .nanoseconds = 0 };
    unsigned int elapsed_seconds = 0;                   // Holds total real-time seconds the program has run      
    struct timeval tv_start, tv_stop;                   // Used to calculated real elapsed time
    gettimeofday(&tv_start, NULL);

    char* execv_arr[EXECV_SIZE];                        // Used to exec and pass data to user processes
    execv_arr[0] = "./user";
    execv_arr[EXECV_SIZE - 1] = NULL;
    
    // Setup shared memory
    simulated_clock_id = get_shared_memory();
    sysclock = (struct clock*) attach_to_shared_memory(simulated_clock_id, 0);
    sysclock->seconds = 0;
    sysclock->nanoseconds = 0;
    proc_ctrl_tbl_id = get_shared_memory();
    proc_ctrl_tbl = (struct process_ctrl_table*) attach_to_shared_memory(proc_ctrl_tbl_id, 0);
    scheduler_id = get_message_queue();
    struct msgbuf scheduler;
    sprintf(scheduler.mtext, "You've been scheduled!");

    childpids = malloc(sizeof(pid_t) * TOTAL_PROC_LIMIT);

    if ((fp = fopen("./oss.log", "w")) == NULL) {
        perror("fopen");
        exit(1);
    }

    // Round robin queue
    struct Queue roundRobin = { .front = 0, .rear = -1, .itemCount = 0 };
    // Multi-level feedback queues
    struct Queue level1 = { .front = 0, .rear = -1, .itemCount = 0 };
    struct Queue level2 = { .front = 0, .rear = -1, .itemCount = 0 };
    struct Queue level3 = { .front = 0, .rear = -1, .itemCount = 0 };

    struct Queue queue_arr[NUM_QUEUES] = {
        roundRobin, 
        level1, 
        level2,
        level3
    };

    /*
     *  Main loop
     */

    //ns_before_next_proc = rand() % MAX_NS_BEFORE_NEW_PROC; 
    //time_to_fork = convertToClockTime(ns_before_next_proc); // Will be 0-2 seconds

    while ( (num_procs_spawned < TOTAL_PROC_LIMIT) && (elapsed_seconds < TOTAL_RUNTIME) ) {

        ns_before_next_proc = rand() % MAX_NS_BEFORE_NEW_PROC; 
        time_to_fork = convertToClockTime(ns_before_next_proc); // Will be 0-2 seconds
        time_to_fork.seconds += sysclock->seconds;         // Increment to current time
        time_to_fork.nanoseconds += sysclock->nanoseconds;

        // Increment sysclock until time to fork
        while (sysclock->seconds <= time_to_fork.seconds && sysclock->nanoseconds <=  time_to_fork.nanoseconds) {
            ns_worked = get_random_amt_of_ns_worked();
            increment_clock(sysclock, ns_worked);
        }

        for (i = 1; i < PROC_CTRL_TBL_SZE + 1; i++) {
            if (pcb_in_use[i] == 0) {
                // Create process control block
                struct process_ctrl_block pcb = {
                    .pid = i,
                    .is_realtime = get_realtime(),
                    .time_quantum = 10000000, // 10 ms in nanoseconds
                    .cpu_time_used.seconds = 0, .cpu_time_used.nanoseconds = 0,
                    .sys_time_used.seconds = 0, .sys_time_used.nanoseconds = 0,
                    .last_run_time_used.seconds = 0, .last_run_time_used.nanoseconds = 0,
                    .time_unblocked.seconds = 0, .time_unblocked.nanoseconds = 0,
                    .time_scheduled.seconds = sysclock->seconds, .time_scheduled.nanoseconds = sysclock->nanoseconds,
                    .time_finished.seconds = 0, .time_finished.nanoseconds = 0
                };

                // Add PCB to process control table
                proc_ctrl_tbl->pcbs[i] = pcb;

                // Mark PCB in use
                pcb_in_use[i] = 1;
                
                // Fork and place in queue
                fork_child(execv_arr, num_procs_spawned, pcb.pid);
                insert(&roundRobin, pcb.pid);
                printf("OSS: Generating process with PID %d at putting it in queue %d at time %d:%'d\n",
                    pcb.pid, 1, sysclock->seconds, sysclock->nanoseconds);
                fprintf(fp, "OSS: Generating process with PID %d at putting it in queue %d at time %d:%'d\n",
                    pcb.pid, 1, sysclock->seconds, sysclock->nanoseconds);

                num_procs_spawned += 1;

                // Schedule
                pid = dequeue(&roundRobin);
                send_msg(scheduler_id, &scheduler, pid);
                printf("OSS: Dispatching process with PID %d from queue %d at time %d:%'d\n", 
                    pid, 1, sysclock->seconds, sysclock->nanoseconds);
                fprintf(fp, "OSS: Dispatching process with PID %d from queue %d at time %d:%'d\n", 
                    pid, 1, sysclock->seconds, sysclock->nanoseconds);

                // Receive
                receive_msg(scheduler_id, &scheduler, (pid + PROC_CTRL_TBL_SZE)); // Add PROC_CTRL_TBL_SZE to message type
                printf("OSS: Receiving that process with PID %d ran for %d:%'d\n", 
                    pid, sysclock->seconds, sysclock->nanoseconds);
                fprintf(fp, "OSS: Receiving that process with PID %d ran for %d:%'d\n", 
                    pid, sysclock->seconds, sysclock->nanoseconds);

                // Put back in queue
                insert(&roundRobin, pid);
                printf("OSS: Putting process with PID %d into queue %d\n", 
                    pid, 1);
                fprintf(fp, "OSS: Putting process with PID %d into queue %d\n", 
                    pid, 1);

                break;
            }
        }
        
        
        waitpid(-1, NULL, WNOHANG); // Cleanup any zombies as we go
        
        // Calculate total elapsed seconds
        gettimeofday(&tv_stop, NULL);
        elapsed_seconds = tv_stop.tv_sec - tv_start.tv_sec;
    }

    // Print information before exiting
    printf("OSS: Exiting because 100 processes have been spawned or because %d seconds have been passed\n", TOTAL_RUNTIME);
    printf("OSS: Simulated clock time: %d:%'d\n",
            sysclock->seconds, sysclock->nanoseconds);
    printf("OSS: %d processes spawned\n", num_procs_spawned);
    fprintf(fp, "OSS: Exiting because 100 processes have been spawned or because %d seconds have been passed\n", TOTAL_RUNTIME);
    fprintf(fp, "OSS: Simulated clock time: %d:%'d\n",
            sysclock->seconds, sysclock->nanoseconds);
    fprintf(fp, "OSS: %d processes spawned\n", num_procs_spawned);
    
    cleanup_and_exit();

    return 0;

}

void increment_clock(struct clock* clock, int increment) {
    clock->nanoseconds += increment;
    if (clock->nanoseconds >= ONE_BILLION) {
        clock->seconds += 1;
        clock->nanoseconds -= ONE_BILLION;
    }
}

unsigned int get_realtime() {
    return event_occured(PCT_REALTIME);
}

unsigned int get_random_amt_of_ns_worked() {
    return (rand() % 10000) + 100;
}

struct clock convertToClockTime(int nanoseconds) {
    struct clock clk = { .seconds = 0, .nanoseconds = 0 };
    if (nanoseconds >= ONE_BILLION) {
        nanoseconds -= ONE_BILLION;
        clk.seconds = 1;
    }
    clk.nanoseconds = nanoseconds;
    return clk;
}

void fork_child(char** execv_arr, int child_idx, int pid) {
    if ((childpids[child_idx] = fork()) == 0) {
        // Child so...
        char sysclock_id[10];
        char pct_id[10];
        char pid_str[5];
        char schedulerid[10];
        sprintf(sysclock_id, "%d", simulated_clock_id);
        sprintf(pct_id, "%d", proc_ctrl_tbl_id);
        sprintf(pid_str, "%d", pid);
        sprintf(schedulerid, "%d", scheduler_id);
        execv_arr[SYSCLOCK_ID_IDX] = sysclock_id;
        execv_arr[PCT_ID_IDX] = pct_id;
        execv_arr[PID_IDX] = pid_str;
        execv_arr[SCHEDULER_IDX] = schedulerid;

        execvp(execv_arr[0], execv_arr);

        perror("Child failed to execvp the command!");
        exit(1);
    }

    if (childpids[child_idx] == -1) {
        perror("Child failed to fork!\n");
        exit(1);
    }
}

void wait_for_all_children() {
    pid_t childpid;
    printf("OSS: Waiting for all children to exit\n");
    fprintf(fp, "OSS: Waiting for all children to exit\n");
    while  ( (childpid = wait(NULL) ) > 0);
}

void terminate_children() {
    printf("OSS: Sending SIGTERM to all children\n");
    fprintf(fp, "OSS: Sending SIGTERM to all children\n");
    int length = sizeof(childpids)/sizeof(childpids[0]);
    int i;
    for (i = 0; i < length; i++) {
        if (kill(childpids[i], 0) > -1) {
            // That process exits
            if (kill(childpids[i], SIGTERM) == -1) {
                perror("kill");
                exit(1);
            }
        }
    }
    free(childpids);
}

void add_signal_handlers() {
    struct sigaction act;
    act.sa_handler = handle_sigint; // Signal handler
    sigemptyset(&act.sa_mask);      // No other signals should be blocked
    act.sa_flags = 0;               // 0 so do not modify behavior
    if (sigaction(SIGINT, &act, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    act.sa_handler = handle_sigalrm; // Signal handler
    sigemptyset(&act.sa_mask);       // No other signals should be blocked
    if (sigaction(SIGALRM, &act, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
}

void handle_sigint(int sig) {
    printf("\nOSS: Caught SIGINT signal %d\n", sig);
    fprintf(fp, "\nOSS: Caught SIGINT signal %d\n", sig);
    if (cleaning_up == 0) {
        cleaning_up = 1;
        cleanup_and_exit();
    }
}

void handle_sigalrm(int sig) {
    printf("\nOSS: Caught SIGALRM signal %d\n", sig);
    fprintf(fp, "\nOSS: Caught SIGALRM signal %d\n", sig);
    if (cleaning_up == 0) {
        cleaning_up = 1;
        cleanup_and_exit();
    }

}

void cleanup_and_exit() {
    terminate_children();
    wait_for_all_children();
    printf("OSS: Removing message queues and shared memory\n");
    fprintf(fp, "OSS: Removing message queues and shared memory\n");
    remove_message_queue(scheduler_id);
    cleanup_shared_memory(simulated_clock_id, sysclock);
    cleanup_shared_memory(proc_ctrl_tbl_id, proc_ctrl_tbl);
    fclose(fp);
    exit(0);
}

