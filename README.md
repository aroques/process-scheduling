## Process scheduling

**OSS (operating system similator)** is a program that simulates an operating system scheduler.

OSS randomly generated users processes. Then places that user process in a scheduling queue. When a user process runs it has a chance to terminate, get blocked, or use run for its entire time quantum and not get blocked. 

There is 1 round robin queues and 1 multilevel feeback queue that has 3 levels. The round robin queue is for real-time processes. Realtime processes will remain in the Round Robin queue until they terminate. Non real-time processes will remain in one of the levels of the multilevel feedback queue. Each non real-time process that runs and does not get blocked is placed in the next level of the multi level feedback queue. For example, if a non real-time process in the level 1 queue is scheduled, runs, and does not get blocked, then it will be placed in the level 2 queue.

OSS will always schedule a process that is in the highest priority queue. Each queue has an associated time quantum that grows as its priority increases. The queues (from highest to lowest priority) and their associated time quantum are defined as follows:
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

61496bf 2018-03-17 16:01:48 -0500  Update README.md  
ab17342 2018-03-17 15:54:37 -0500  Describe program in README  
06fcf72 2018-03-17 15:32:03 -0500  Untrack .vscode  
d12c116 2018-03-17 15:30:26 -0500  Reimplement queue as FIFO, improve statistics report  
fc4a2ce 2018-03-17 14:50:40 -0500  Debugged statistics and program termination  
ead5ac2 2018-03-16 20:34:47 -0500  Add print clock function  
6632de5 2018-03-16 20:18:12 -0500  Increase size of int and doubles  
92b52d0 2018-03-16 19:55:31 -0500  Add clock functions, develop statistics report  
64d1a5e 2018-03-16 15:46:33 -0500  Add more logic to OSS  
e96f2f4 2018-03-15 15:41:45 -0500  Remove unnessecary parentheses  
24ec3ae 2018-03-15 15:37:13 -0500  Add statistics to OSS program  
53c3343 2018-03-15 11:06:58 -0500  Add process status handling to OSS program  
a2befa0 2018-03-14 20:27:25 -0500  Add termination step to user program  
03dadda 2018-03-14 19:56:03 -0500  Add main logic to user  
fbbdeb0 2018-03-14 19:55:46 -0500  Add main logic to user  
5a98293 2018-03-13 10:14:23 -0500  Report last run time in oss  
8519061 2018-03-13 09:27:21 -0500  Add multilevel feedback queues  
2d846ee 2018-03-13 09:20:48 -0500  Add more logic to oss program  
183bdb8 2018-03-13 09:00:23 -0500  Add queue  
0ec1ded 2018-03-12 20:16:27 -0500  Successfully send test message from oss to user  
33fd417 2018-03-11 19:02:18 -0500  Further develop user program  
3c471b8 2018-03-11 18:24:54 -0500  Start developing user program  
6c66e00 2018-03-11 17:57:27 -0500  Add message queue  
f03a5dd 2018-03-11 17:38:04 -0500  Change process control table from message queue to shared memory  
793ca29 2018-03-11 16:18:27 -0500  Change logical clock from message queue to shared memory  
04f99c4 2018-03-11 16:18:18 -0500  Change logical clock from message queue to shared memory  
dede264 2018-03-10 19:53:44 -0600  Develop oss program  
43fd9b0 2018-03-10 19:13:19 -0600  Rework oss.c for process scheduling project  
acee9bc 2018-03-10 13:00:51 -0600  Initial commit  
