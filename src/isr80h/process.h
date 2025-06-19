#ifndef PROCESS_H
#define PROCESS_H

struct interrupt_frame;
void *isr80h_command6_proccess_load_start(struct interrupt_frame *frame);
void *isr80h_command7_invoke_system_command(struct interrupt_frame *frame);
void *isr80h_command8_get_program_arguments(struct interrupt_frame *frame);
void *isr80h_command0_exit(struct interrupt_frame *frame);

#endif