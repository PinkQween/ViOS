#ifndef HEAP_H
#define HEAP_H

struct interrupt_frame;
void *isr80h_command7_malloc(struct interrupt_frame *frame);
void *isr80h_command8_free(struct interrupt_frame *frame);

#endif