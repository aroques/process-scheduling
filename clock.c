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
    struct clock out;
    out.seconds = c1.seconds + c2.seconds;
    out.nanoseconds = c1.nanoseconds + c2.nanoseconds;
    return out;
}