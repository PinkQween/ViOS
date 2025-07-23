
#include "serial.h"

void *isr80h_command_putchar_serial(struct interrupt_frame *frame)
{
    char c = (char)(int)task_get_stack_item(task_current(), 0);
    simple_serial_putc(c);
    return 0;
}

void *isr80h_command_print_serial(struct interrupt_frame *frame)
{
    void *user_space_msg_buffer = task_get_stack_item(task_current(), 0);
    char buf[1024];
    copy_string_from_task(task_current(), user_space_msg_buffer, buf, sizeof(buf));
    simple_serial_puts(buf);
    return 0;
}