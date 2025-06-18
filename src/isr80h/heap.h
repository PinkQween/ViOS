#ifndef HEAP_H
#define HEAP_H

struct interrupt_frame;
void *isr80h_command4_malloc(struct interrupt_frame *frame);
void *isr80h_command5_free(struct interrupt_frame *frame);

#endif