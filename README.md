## Process scheduling

**oss (operating system similator)** is the main process who forks off children user processes. All processes have access to a simulated system clock that is protected by critical sections implemented with message queues. A termination log is also implemented with message queues so that children user processes can send oss a message containing information when they terminate.

Each child user process will run for a random duration and during that time will continuously update the simulated system clock. When they are done running they will send a message to oss who will then fork off another child process.

The program (by default) will generate a log file called oss.log.

To build this program run:
```
make
```

To run this program:
```    
./oss
```

```
make clean
```

#### Below is my git log:

