#include "graphics.h"
#include "drivers/output/vigfx/vigfx.h"
#include "task/task.h"

void *isr80h_command_flush(struct interrupt_frame *frame) {
    gpu_flush_screen();

    return 0;
}

void *isr80h_command_draw_pixel(struct interrupt_frame *frame) {
    int x = (int)task_get_stack_item(task_current(), 0);
    int y = (int)task_get_stack_item(task_current(), 1);
    int r = (int)task_get_stack_item(task_current(), 2);
    int g = (int)task_get_stack_item(task_current(), 3);
    int b = (int)task_get_stack_item(task_current(), 4);
    int a = (int)task_get_stack_item(task_current(), 5);

    gpu_draw_alpha(x, y, r, g, b, a);
    return 0;
}