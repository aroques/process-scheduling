#ifndef GLOBAL_CONSTANTS_H
#define GLOBAL_CONSTANTS_H

#define ONE_BILLION 1000000000
#define TWO_BILLION 2000000000
#define TEN_MILLION 10000000
#define NUM_QUEUES 4

const unsigned int EXECV_SIZE = 6;
const unsigned int SYSCLOCK_ID_IDX = 1;
const unsigned int PCT_ID_IDX = 2;
const unsigned int PID_IDX = 3;
const unsigned int SCHEDULER_IDX = 4;

const unsigned int TOTAL_PROC_LIMIT = 100;

const unsigned int MAX_RUNTIME = 20; // In seconds

const unsigned int MAX_NS_BEFORE_NEW_PROC = TWO_BILLION;
const unsigned int PCT_REALTIME = 4;
const unsigned int BASE_TIME_QUANTUM = TEN_MILLION; // 10 ms == 10 million nanoseconds

#endif
