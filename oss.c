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

void wait_for_all_children();
char* get_msgqid(int msgqid);
void add_signal_handlers();
void handle_sigint(int sig);
void handle_sigalrm(int sig);
void cleanup_and_exit();
void fork_child(char** execv_arr, int child_idx, int pid);
struct clock convertToClockTime(int nanoseconds);
unsigned int get_realtime();
unsigned int get_random_amt_of_ns_worked();

// Globals used in signal handler
int simulated_clock_id, proc_ctrl_tbl_id;
int cleaning_up = 0;
pid_t* childpids;
FILE* fp;

int main (int argc, char* argv[]) {
    /*
     *  Setup program before entering main loop
     */
    const int TOTAL_RUNTIME = 3;                        // Max seconds oss should run for
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
    struct clock* sysclock;                            // Holds simulated system time
    struct process_ctrl_table proc_ctrl_tbl = { .mtype = 1 };
    simulated_clock_id = get_shared_memory();
    sysclock = attach_to_clock(simulated_clock_id, 0);
    sysclock->seconds = 0;
    sysclock->nanoseconds = 0;
    proc_ctrl_tbl_id = get_message_queue();

    childpids = malloc(sizeof(pid_t) * TOTAL_PROC_LIMIT);

    if ((fp = fopen("./oss.log", "w")) == NULL) {
        perror("fopen");
        exit(1);
    }

    set_timer(MAX_RUNTIME);                             // Set timer that triggers SIGALRM
    add_signal_handlers();
    setlocale(LC_NUMERIC, "");                          // For comma separated integers in printf
    srand(time(NULL) ^ getpid());

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

        int i;
        for (i = 0; i < PROC_CTRL_TBL_SZE; i++) {
            if (pcb_in_use[i] == 0) {
                // Create process control block
                struct process_ctrl_block pcb = {
                    .pid = i,
                    .is_realtime = get_realtime(),
                    .cpu_time_used.seconds = 0, .cpu_time_used.nanoseconds = 0,
                    .sys_time_used.seconds = 0, .sys_time_used.nanoseconds = 0,
                    .last_run_time_used.seconds = 0, .last_run_time_used.nanoseconds = 0,
                    .time_unblocked.seconds = 0, .time_unblocked.nanoseconds = 0
                };

                // Add PCB to process control table
                proc_ctrl_tbl.pcbs[i] = pcb;

                // Mark PCB in use
                pcb_in_use[i] = 1;

                // Then fork
                fork_child(execv_arr, num_procs_spawned, pcb.pid);

                // Put in a queue

                // Schedule

                printf("Master: Creating new child pid %d at my time %d:%'d\n",
                    childpids[num_procs_spawned],
                    sysclock->seconds, sysclock->nanoseconds);
                fprintf(fp, "Master: Creating new child pid %d at my time %d:%'d\n",
                    childpids[num_procs_spawned],
                    sysclock->seconds, sysclock->nanoseconds);

                num_procs_spawned += 1;
                
                break;
            }
        }
        
        
        waitpid(-1, NULL, WNOHANG); // Cleanup any zombies as we go
        
        // Calculate total elapsed seconds
        gettimeofday(&tv_stop, NULL);
        elapsed_seconds = tv_stop.tv_sec - tv_start.tv_sec;
    }

    // Print information before exiting
    printf("Master: Exiting because 100 processes have been spawned or because %d seconds have been passed\n", TOTAL_RUNTIME);
    printf("Master: Simulated clock time: %d:%'d\n",
            sysclock->seconds, sysclock->nanoseconds);
    printf("Master: %d processes spawned\n", num_procs_spawned);
    fprintf(fp, "Master: Exiting because 100 processes have been spawned or because %d seconds have been passed\n", TOTAL_RUNTIME);
    fprintf(fp, "Master: Simulated clock time: %d:%'d\n",
            sysclock->seconds, sysclock->nanoseconds);
    fprintf(fp, "Master: %d processes spawned\n", num_procs_spawned);
    
    cleanup_and_exit();

    return 0;

}

unsigned int get_realtime() {
    unsigned int percent = (rand() % 100) + 1;
    if (percent <= PCT_REALTIME) {
        return 1;
    }
    else {
        return 0;
    }
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
        sprintf(sysclock_id, "%d", simulated_clock_id);
        sprintf(pct_id, "%d", proc_ctrl_tbl_id);
        sprintf(pid_str, "%d", pid);
        execv_arr[SYSCLOCK_ID_IDX] = sysclock_id;
        execv_arr[PCT_ID_IDX] = pct_id;
        execv_arr[PID_IDX] = pid_str;

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
    printf("Master: Waiting for all children to exit\n");
    fprintf(fp, "Master: Waiting for all children to exit\n");
    while  ( (childpid = wait(NULL) ) > 0);
}

void terminate_children() {
    printf("Master: Sending SIGTERM to all children\n");
    fprintf(fp, "Master: Sending SIGTERM to all children\n");
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
    printf("\nMaster: Caught SIGINT signal %d\n", sig);
    fprintf(fp, "\nMaster: Caught SIGINT signal %d\n", sig);
    if (cleaning_up == 0) {
        cleaning_up = 1;
        cleanup_and_exit();
    }
}

void handle_sigalrm(int sig) {
    printf("\nMaster: Caught SIGALRM signal %d\n", sig);
    fprintf(fp, "\nMaster: Caught SIGALRM signal %d\n", sig);
    if (cleaning_up == 0) {
        cleaning_up = 1;
        cleanup_and_exit();
    }

}

void cleanup_and_exit() {
    terminate_children();
    wait_for_all_children();
    printf("Master: Removing message queues\n");
    fprintf(fp, "Master: Removing message queues\n");
    remove_message_queue(simulated_clock_id);
    remove_message_queue(proc_ctrl_tbl_id);
    fclose(fp);
    exit(0);
}

