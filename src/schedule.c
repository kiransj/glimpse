#include <util.h>
#include <timer.h>
#include <isr.h>
#include <display.h>
#include <paging.h>
#include <asm.h>
#include <malloc.h>
#include <timer.h>
static uint32_t scheduling_initialzed = 0;
volatile uint32_t timer_ticks = 0;
uint32_t pid = 0;

enum
{
    TASK_STATE_NOT_STARTED = 0,
    TASK_STATE_RUNNING,
    TASK_STATE_SLEEPING,
    TASK_STATE_WAITING_CPU,
    TASK_STATE_STOPPED,
    TASK_STATE_ENDED
};
struct _task
{
    uint32_t pid;
    uint32_t state;
    uint32_t stack;

    /*Tells when to wake up is the thread is sleeping*/
    uint32_t wake_up_cycle;
    uint32_t total_cycles, prev_tick_count;

    void (*thread)(void);
    struct _task *next;
    char threadName[16];
};

struct _task *current_task;
struct _task *task_list;

void print_ktask_list(void)
{
    struct _task *task = current_task;
    do
    {
        printf("Pid(%d), state(%d), total_time(%d ms), name(%s)\n", task->pid, task->state, task->total_cycles*10, task->threadName);
        task = task->next;
    }
    while(task != current_task);

    return ;
}

#define NUM_CYCLES(x) (x)
void sleep(uint32_t milliSeconds)
{
    uint32_t cycles = NUM_CYCLES(milliSeconds);
    if(scheduling_initialzed)
    {
        CLEAR_INTERRUPT();
        current_task->state = TASK_STATE_SLEEPING;
        current_task->wake_up_cycle = timer_ticks + cycles;
        ENABLE_INTERRUPT();
        asm volatile("int $0x80");
    }
    else
    {
        /*Is scheduling is not enabled then have busy loop*/
        LOG_WARN("sleep Busy Loop!!!");
        uint32_t wake_up = timer_ticks + cycles;
        while(wake_up > timer_ticks);
    }
}

/*These 2 functions are called with interrupt disabled.
 * These two should not be called from C*/
uint32_t schedule(uint32_t esp)
{
    if(scheduling_initialzed)
    {
#ifdef DEBUG
        uint32_t old_pid = current_task->pid;
#endif
        current_task->total_cycles += (timer_ticks - current_task->prev_tick_count) + 1;
        current_task->stack = esp;
        switch(current_task->state)
        {
            case TASK_STATE_RUNNING:
                    current_task->state = TASK_STATE_WAITING_CPU;
                    break;
            default:
                    break;
        }
        do
        {
            current_task = current_task->next;
            if(TASK_STATE_SLEEPING == current_task->state)
            {
                if(current_task->wake_up_cycle < timer_ticks)
                {
                    current_task->wake_up_cycle = 0;
                    current_task->state = TASK_STATE_RUNNING;
                }
            }
            else
            {
                current_task->state = TASK_STATE_RUNNING;
            }
        }
        while(TASK_STATE_RUNNING != current_task->state);
        current_task->prev_tick_count = timer_ticks;
        esp = current_task->stack;
        LOG_INFO("old_pid = %x, new_pid = %x", old_pid, current_task->pid);
    }
    return esp;
}

uint32_t timer_callback(uint32_t esp)
{
    uint32_t ret_esp = 0;
    timer_ticks += 1;
    ret_esp = schedule(esp);
    return ret_esp;
}

void initialize_scheduling(void)
{
   task_list = (struct _task *)kmalloc(sizeof(struct _task));
   memset(task_list, 0, sizeof(struct _task));
   task_list->pid = pid++;
   task_list->thread = NULL;
   task_list->state = TASK_STATE_RUNNING;
   task_list->next = task_list;
   strcpy(task_list->threadName, "idle_task");
   current_task = task_list;
   scheduling_initialzed = 1;

    /*Start the timer 1 milli seconds*/
    init_timer(1000);
}

uint32_t get_pid(void)
{
    return current_task->pid;
}

uint32_t kthread_create(void (*thread)(void), char threadName[16])
{
    CLEAR_INTERRUPT();
    uint32_t *stack, address;
    struct _task *task = (struct _task *)kmalloc(sizeof(struct _task));
    memset(task, 0, sizeof(struct _task));
    address = task->stack = get_mapped_page(0x1000) + 0x1000;
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
    strcpy(task->threadName, threadName);
	task->stack = (uint32_t) stack;
	task->thread = thread;
    task->next = current_task->next;
    current_task->next = task;
    ENABLE_INTERRUPT();
    return task->pid;
}
