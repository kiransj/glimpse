#include <util.h>
#include <timer.h>
#include <isr.h>
#include <display.h>
#include <paging.h>
#include "asm.h"
#include "memory.h"
static uint32_t scheduling_initialzed = 0;
volatile uint32_t timer_ticks = 0;

uint32_t pid = 0;

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
        uint32_t old_pid = current_task->pid;
        printf("Current ESP : %u, ticks = %u\n", esp, timer_ticks);
        current_task->stack = esp;
        current_task = current_task->next;
        esp = current_task->stack;
        printf("old_pid = %x, new_pid = %x\n", old_pid, current_task->pid);
    }
    return esp;
}

void initialize_scheduling(void)
{
   task_list = (struct _task *)allocate_block32();
   memset(task_list, 0, 32);
   task_list->pid = pid++;
   task_list->thread = NULL;
   task_list->state = TASK_STATE_RUNNING;
   task_list->next = task_list;
   current_task = task_list;
   scheduling_initialzed = 1;
}

uint32_t get_pid(void)
{
    return current_task->pid;
}
uint32_t kthread_create(void (*thread)(void))
{
    CLEAR_INTERRUPT();
    uint32_t *stack;
    struct _task *task = (struct _task *)allocate_block32();
    task->stack = get_mapped_page(0x1000) + 0x1000;
    stack = (uint32_t*)task->stack;

    *--stack = 0x202; //EFLAGS
    *--stack = 0x08; //CS
    *--stack = (uint32_t)thread;

	// pusha
	*--stack = 0;		// EDI
	*--stack = 0;		// ESI
	*--stack = 0;		// EBP
	*--stack = 0;		// NULL
	*--stack = 0;		// EBX
	*--stack = 0;		// EDX
	*--stack = 0;		// ECX
	*--stack = 0;		// EAX

	// data segments
	*--stack = 0x10;	// DS
	*--stack = 0x10;	// ES
	*--stack = 0x10;	// FS
	*--stack = 0x10;	// GS
    task->state = TASK_STATE_NOT_STARTED;
    task->pid = pid++;

	task->stack = (uint32_t) stack;
	task->thread = thread;
    task->next = current_task->next;
    current_task->next = task;    
    ENABLE_INTERRUPT();
    return task->pid;
}
