#include "io.h"
#include "task/task.h"
#include "graphics/graphics.h"
#include "keyboard/keyboard.h"
#include "rtc/rtc.h"

void *isr80h_command1_print(struct interrupt_frame *frame)
{
    void *user_space_msg_buffer = task_get_stack_item(task_current(), 0);
    int x = (int)task_get_stack_item(task_current(), 1);
    int y = (int)task_get_stack_item(task_current(), 2);
    int r = (int)task_get_stack_item(task_current(), 3);
    int g = (int)task_get_stack_item(task_current(), 4);
    int b = (int)task_get_stack_item(task_current(), 5);
    int s = (int)task_get_stack_item(task_current(), 6);
    char buf[1024];
    copy_string_from_task(task_current(), user_space_msg_buffer, buf, sizeof(buf));

    DrawAtariString(buf, x, y, r, g, b, s);
    return 0;
}

void *isr80h_command2_getkey(struct interrupt_frame *frame)
{
    char c = keyboard_pop();
    return (void *)((int)c);
}

void *isr80h_command3_putchar(struct interrupt_frame *frame)
{
    char c = (char)(int)task_get_stack_item(task_current(), 0);
    int x = (int)(task_current(), 1);
    int y = (int)(task_current(), 2);
    int r = (int)(task_current(), 3);
    int g = (int)(task_current(), 4);
    int b = (int)(task_current(), 5);
    int s = (int)(task_current(), 6);

    DrawAtariChar(c, x, y, r, g, b, s);
    return 0;
}

void *isr80h_command9_sleep(struct interrupt_frame *frame)
{
    int seconds = (int)task_get_stack_item(task_current(), 0);
    if (seconds < 0)
    {
        return (void *)-1; // Return error for negative values
    }
    sleep_seconds(seconds);
    return 0;
}