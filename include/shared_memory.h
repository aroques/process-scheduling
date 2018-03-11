#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#define ONE_BILLION 1000000000

#include "global_structs.h"

int get_shared_memory();
struct clock* attach_to_clock(int shmemid, unsigned int readonly);
void increment_clock(struct clock* clock, int increment);
void cleanup_shared_memory(int shmemid, struct clock* p);
void detach_from_clock(struct clock* p);
void deallocate_shared_memory(int shmemid);

#endif
