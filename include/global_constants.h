#ifndef GLOBAL_CONSTANTS_H
#define GLOBAL_CONSTANTS_H

#define TWO_BILLION 2000000000

const int EXECV_SIZE = 4;
const int SYSCLOCK_ID_IDX = 1;
const int PCT_ID_IDX = 2;

const int PROC_LIMIT = 20;
const int TOTAL_PROC_LIMIT = 100;

const int MAX_RUNTIME = 20;                         // In seconds

const int MAX_NS_BEFORE_NEW_PROC = TWO_BILLION;
const int PCT_REALTIME = 4;

#endif
