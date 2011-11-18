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

    int (*thread)(void);
    struct _task *next, *prev;
    char threadName[16];

    /*This is used to free the stack later*/
    uint32_t stack_address, stack_size;
    uint32_t returnValue;
};

struct _task *current_task;
struct _task *task_list;

void print_ktask_list(void)
{
    struct _task *task = task_list;
    do
    {
        printf("Pid(%d), state(%d), total_time(%d ms), name(%s)\n", task->pid, task->state, task->total_cycles*10, task->threadName);
        task = task->next;
    }
    while(task != NULL);

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
        yeild();
//        asm volatile("int $0x80");
    }
    else
    {
        /*Is scheduling is not enabled then have busy loop*/
        LOG_WARN("sleep Busy Loop!!!");
        uint32_t wake_up = timer_ticks + cycles;
        while(wake_up > timer_ticks);
    }
}

void remove_task(struct _task *task)
{
    struct _task *tmp;
    
    printf("Task '%s' ended with returnValue : %d\n", task->threadName, task->returnValue);
    tmp = task->prev;
    tmp->next = task->next;
    if(task->next != NULL)
        task->next->prev = tmp;

    /*Free the stack allocated*/
    free_mapped_page(task->stack_address, task->stack_size);
    /*Delete the task*/
    kfree(task);    


    print_ktask_list();
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
            current_task = (NULL == current_task->next) ? task_list : current_task->next;
            switch(current_task->state)
            {
                case TASK_STATE_SLEEPING:
                    /*Check if its time to wake up this task*/
                    if(current_task->wake_up_cycle <= timer_ticks)
                    {
                        current_task->wake_up_cycle = 0;
                        current_task->state = TASK_STATE_RUNNING;
                    }
                    break;
                case TASK_STATE_NOT_STARTED:
                    {
                        /*If you want to do something before starting a
                         * task add it here*/
                        current_task->state = TASK_STATE_RUNNING;
                    }
                    break;
                case TASK_STATE_WAITING_CPU:
                    current_task->state = TASK_STATE_RUNNING;
                    break;
                case TASK_STATE_ENDED:
                    {
                        struct _task *tmp = current_task;
                        current_task = (NULL == current_task->next) ? task_list : current_task->next;
                        remove_task(tmp);
                        continue;
                    }

                default:
                    break;
            }
        }
        while(TASK_STATE_RUNNING != current_task->state);
        current_task->prev_tick_count = timer_ticks;
        esp = current_task->stack;
    //    LOG_INFO("old_pid = %x, new_pid = %x", old_pid, current_task->pid);
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
   task_list->next = NULL;
   task_list->prev = NULL;
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

void start_thread_function(void)
{
    current_task->returnValue = current_task->thread();
    current_task->state = TASK_STATE_ENDED;
    yeild();
    /*Control will neever come here*/
}
uint32_t kthread_create(int (*thread)(void), char threadName[16])
{
    uint32_t *stack;
    struct _task *task = (struct _task *)kmalloc(sizeof(struct _task));
    memset(task, 0, sizeof(struct _task));

    task->stack_size = 0x1000;
    task->stack_address = get_mapped_page(task->stack_size);
    task->stack = task->stack_address + task->stack_size;
    stack = (uint32_t*)task->stack;

    *--stack = 0x202; //EFLAGS
    *--stack = 0x08; //CS
    *--stack = (uint32_t)start_thread_function;

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
    task->prev = current_task;
    CLEAR_INTERRUPT();
    if(NULL != current_task->next)
            current_task->next->prev = task;
    current_task->next = task;
    ENABLE_INTERRUPT();

    return task->pid;
}
