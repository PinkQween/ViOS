#include "io.h"
#include "task/task.h"
#include "keyboard/keyboard.h"
#include "kernel.h"
void* isr80h_command1_print(struct interrupt_frame* frame)
{
    kernel_debug_log("[isr80h_command1_print] start\n");
    void* user_space_msg_buffer = task_get_stack_item(task_current(), 0);
    kernel_debug_log("[isr80h_command1_print] got buffer\n");
    char buf[1024];
    copy_string_from_task(task_current(), user_space_msg_buffer, buf, sizeof(buf));
    kernel_debug_log("[isr80h_command1_print] copied string\n");

    print(buf);
    kernel_debug_log("[isr80h_command1_print] done\n");
    return 0;
}


void* isr80h_command2_getkey(struct interrupt_frame* frame)
{
    char c = keyboard_pop();
    return (void*)((uintptr_t)c);
}

void* isr80h_command3_putchar(struct interrupt_frame* frame)
{
    char c = (char)(uintptr_t) task_get_stack_item(task_current(), 0);
    terminal_writechar(c, 15);
    return 0;
}