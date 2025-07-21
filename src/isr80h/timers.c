#include "drivers/io/timing/timer.h"
#include "task/task.h"
#include "kernel.h"

// System call handler for sleep (in milliseconds)
void *isr80h_command10_sleep(struct interrupt_frame *frame)
{
    int ms = (int)task_get_stack_item(task_current(), 0);
    if (ms < 0)
    {
        return (void *)-1;
    }
    struct task *t = task_current();
    t->sleeping = 1;
    t->wakeup_tick = timer_get_ticks() + (unsigned long)ms;
    task_next(); // Yield to next task
    return 0;
}

// System call handler for usleep (in microseconds)
void *isr80h_command12_usleep(struct interrupt_frame *frame)
{
    int usec = (int)task_get_stack_item(task_current(), 0);
    if (usec < 0)
    {
        return (void *)-1;
    }
    struct task *t = task_current();
    t->sleeping = 1;
    t->wakeup_tick = timer_get_ticks() + (unsigned long)(usec / 1000);
    task_next();
    return 0;
}

// System call handler for nanosleep (in nanoseconds)
void *isr80h_command13_nanosleep(struct interrupt_frame *frame)
{
    int nsec = (int)task_get_stack_item(task_current(), 0);
    if (nsec < 0)
    {
        return (void *)-1;
    }
    struct task *t = task_current();
    t->sleeping = 1;
    t->wakeup_tick = timer_get_ticks() + (unsigned long)(nsec / 1000000);
    task_next();
    return 0;
}