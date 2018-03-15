#ifndef CLOCK_H
#define CLOCK_H

struct clock {
    unsigned int seconds;
    unsigned int nanoseconds;
};

void increment_clock(struct clock* clock, int increment);

#endif
