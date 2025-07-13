#ifndef PROCESS_H
#define PROCESS_H

#include "task/process.h"
#include "task/task.h"
#include "string/string.h"
#include "memory/heap/kheap.h"
#include "status.h"
#include "config.h"
#include "kernel.h"
#include "string/string.h"

struct interrupt_frame;
void *isr80h_command1_process_load_start(struct interrupt_frame *frame);
void *isr80h_command2_invoke_system_command(struct interrupt_frame *frame);
void *isr80h_command11_get_program_arguments(struct interrupt_frame *frame);
void *isr80h_command0_exit(struct interrupt_frame *frame);

#endif