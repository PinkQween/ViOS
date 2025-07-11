#include "audio.h"
#include "task/task.h"
#include "audio/audio.h"
#include <stdint.h>

void *isr80h_command12_audio_push(struct interrupt_frame *frame)
{
    char c = (char)(int)task_get_stack_item(task_current(), 0);
    audio_push(c);
    return 0;
}

void *isr80h_command13_audio_pop(struct interrupt_frame *frame)
{
    char c = audio_pop();
    return (void *)((int)c);
}

void *isr80h_command14_audio_control(struct interrupt_frame *frame)
{
    uint8_t command = (uint8_t)(int)task_get_stack_item(task_current(), 0);
    virtual_audio_control(command);
    return 0;
}
