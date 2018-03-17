## Process scheduling

**OSS (operating system similator)** is a program that simulates an operating system scheduler.

OSS randomly generated users processes. Then places that user process in a scheduling queue. When a user process runs it has a chance to terminate, get blocked, or use run for its entire time quantum and not get blocked. 

There is 1 round robin queues and 1 multilevel feeback queue that has 3 levels. The round robin queue is for real-time processes. Realtime processes will remain in the Round Robin queue until they terminate. Non real-time processes will remain in one of the levels of the multilevel feedback queue. Each non real-time process that runs and does not get blocked is placed in the next level of the multi level feedback queue. For example, if a non real-time process in the level 1 queue is scheduled, runs, and does not get blocked, then it will be placed in the level 2 queue.

OSS will always schedule a process that is in the highest priority queue. Each queue has an associated time quantum that grows as its priority increases. The queue priorities and associated time quantums are defined from highest to lowest:
1. Round Robin Queue (TQ = BASE_TIME_QUANTUM)
2. Level 1 Queue (TQ = BASE_TIME_QUANTUM * pow(2, 1))
3. Level 2 Queue (TQ = BASE_TIME_QUANTUM * pow(2, 2))
4. Level 3 Queue (TQ = BASE_TIME_QUANTUM * pow(2, 3))

If BASE_TIME_QUANTUM is 10ms then real-time processes will run for 10ms and processes scheduled from the level 2 queue will run for 40 ms.

Therefore, processes that do not get blocked will run for a longer period of time when they are scheduled to run and processes that do get blocked will run for a shorter period of time when they are scheduled to run but they will also be given priority over processes that get blocked less often.

This prevents processes who are often blocked from starving.

The program will generate a log file called oss.log.

To build this program run:
```
make
```

To run this program:
```    
./oss
```

To cleanup run:
```
make clean
```

#### Below is my git log:

