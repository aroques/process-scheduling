#include "clock.h"

#define ONE_BILLION 1000000000

void increment_clock(struct clock* clock, int increment) {
    clock->nanoseconds += increment;
    if (clock->nanoseconds >= ONE_BILLION) {
        clock->seconds += 1;
        clock->nanoseconds -= ONE_BILLION;
    }
}

struct clock add_clocks(struct clock c1, struct clock c2) {
    struct clock out = {
        .seconds = 0,
        .nanoseconds = 0
    };
    out.seconds = c1.seconds + c2.seconds;
    increment_clock(&out, c1.nanoseconds + c2.nanoseconds);
    return out;
}

unsigned int compare_clocks(struct clock c1, struct clock c2) {
    if (c1.seconds > c2.seconds) {
        return 1;
    }
    if ((c1.seconds == c2.seconds) && (c1.nanoseconds > c2.nanoseconds)) {
        return 1;
    }
    if ((c1.seconds == c2.seconds) && (c1.nanoseconds == c2.nanoseconds)) {
        return 0;
    }
    return -1;
}

double clock_to_seconds(struct clock c) {
    double seconds = c.seconds;
    seconds += (c.nanoseconds / ONE_BILLION);
    return seconds;
}

struct clock seconds_to_clock(double seconds) {
    struct clock clk = { .seconds = (int)seconds };
    seconds -= clk.seconds;
    clk.nanoseconds = seconds * ONE_BILLION;
    return clk;
}

struct clock calculate_avg_time(struct clock clk, int divisor) {
    double seconds = clock_to_seconds(clk);
    double avg_seconds = seconds / divisor;
    return seconds_to_clock(avg_seconds);
}

struct clock subtract_clocks(struct clock c1, struct clock c2) {
    double seconds1 = clock_to_seconds(c1);
    double seconds2 = clock_to_seconds(c1);
    double result = seconds1 - seconds2;
    return seconds_to_clock(result);
}