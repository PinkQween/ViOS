#ifndef ISR80H_PROCESS_H
#define ISR80H_PROCESS_H

#include "task/process.h"
#include "task/task.h"
#include "string/string.h"
#include "memory/heap/kheap.h"
#include "status.h"
#include "config.h"
#include "kernel.h"

struct interrupt_frame;
void *isr80h_command_process_load_start(struct interrupt_frame *frame);
void *isr80h_command_invoke_system_command(struct interrupt_frame *frame);
void *isr80h_command_get_program_arguments(struct interrupt_frame *frame);
void *isr80h_command_exit(struct interrupt_frame *frame);

#endif