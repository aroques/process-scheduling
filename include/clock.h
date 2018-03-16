#ifndef CLOCK_H
#define CLOCK_H

struct clock {
    unsigned int seconds;
    unsigned int nanoseconds;
};

void increment_clock(struct clock* clock, int increment);
struct clock add_clocks(struct clock c1, struct clock c2);
unsigned int compare_clocks(struct clock c1, struct clock c2);

#endif
