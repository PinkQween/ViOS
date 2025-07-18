#include "drivers/io/timing/timer.h"
#include "task/task.h"
#include "kernel.h"

// System call handler for sleep (in milliseconds)
void *isr80h_command10_sleep(struct interrupt_frame *frame)
{
    int ms = (int)task_get_stack_item(task_current(), 0);
    if (ms < 0)
    {
        return (void *)-1; // Return error for negative values
    }
    timer_sleep_ms(ms);
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
    usleep((unsigned int)usec);
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
    nanosleep((unsigned int)nsec);
    return 0;
}