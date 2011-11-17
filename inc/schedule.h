#ifndef __SCHED__
#define __SCHED__

#include "util.h"
void initialize_scheduling(void);
uint32_t get_pid(void);
uint32_t kthread_create(void (*thread)(void), char threadName[16]);
void sleep(uint32_t cycles);
void print_ktask_list(void);
#endif
