#ifndef CLOCK_H
#define CLOCK_H

struct clock {
    unsigned long seconds;
    unsigned long nanoseconds;
};

void increment_clock(struct clock* clock, int increment);
struct clock add_clocks(struct clock c1, struct clock c2);
unsigned int compare_clocks(struct clock c1, struct clock c2);
long double clock_to_seconds(struct clock c);
struct clock seconds_to_clock(long double seconds);
struct clock calculate_avg_time(struct clock clk, int divisor);
struct clock subtract_clocks(struct clock c1, struct clock c2);

#endif
