#include <util.h>
#include <timer.h>
#include <isr.h>
#include <display.h>
#include "asm.h"
#include "memory.h"
static uint32_t scheduling_initialzed = 1;
volatile uint32_t timer_ticks = 0, pid = 0;

enum
{
    TASK_STATE_NOT_STARTED = 0,
    TASK_STATE_RUNNING = 1,
    TASK_STATE_SLEEPING = 2,
    TASK_STATE_STOPPED = 3,
    TASK_STATE_ENDED
};
struct _task
{
    uint32_t pid, state, stack;
    void (*thread)(void);
    struct _task *next;
};

struct _task *current_task;
struct _task *task_list;
uint32_t timer_callback(uint32_t esp)
{
    timer_ticks += 1;
    if((timer_ticks % 18 == 0) && (scheduling_initialzed))
    {
        printf("Current ESP : %u, ticks = %u\n", esp, timer_ticks);
    }
    return esp ;
}

void initialize_scheduling(void)
{
   CLEAR_INTERRUPT(); 
   task_list = (struct _task *)allocate_block32();
   memset(task_list, 0, 32);
   task_list->pid = pid++;
   task_list->thread = NULL;
   task_list->state = TASK_STATE_RUNNING;
   task_list->next = task_list;
   current_task = task_list;
   ENABLE_INTERRUPT();
}
