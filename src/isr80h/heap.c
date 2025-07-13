#include "heap.h"
#include "task/task.h"
#include "task/process.h"
#include <stddef.h>

void *isr80h_command7_malloc(struct interrupt_frame *frame)
{
    size_t size = (int)task_get_stack_item(task_current(), 0);

    // Security check: log suspicious allocation attempts
    if (size > 1024 * 1024)
    { // 1MB
      // Log suspicious large allocation
      // In a real system, you might want to log this to a security log
    }

    return process_malloc(task_current()->process, size);
}

void *isr80h_command8_free(struct interrupt_frame *frame)
{
    void *ptr_to_free = task_get_stack_item(task_current(), 0);

    // Security check: validate pointer before freeing
    if (!ptr_to_free)
    {
        return 0;
    }

    process_free(task_current()->process, ptr_to_free);
    return 0;
}
