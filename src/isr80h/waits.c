#include "waits.h"

void *isr80h_command10_sleep(struct interrupt_frame *frame)
{
    int ms = (int)task_get_stack_item(task_current(), 0);
    if (ms < 0)
    {
        return (void *)-1; // Return error for negative values
    }
    sleep_ms(ms);
    return 0;
}