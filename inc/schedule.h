#ifndef __SCHED__
#define __SCHED__

#include "util.h"
extern uint32_t timer_ticks;
void initialize_scheduling(void);
uint32_t get_pid(void);
uint32_t kthread_create(void (*thread)(void));
#endif
