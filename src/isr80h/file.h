#ifndef ISR80H_FILE_H
#define ISR80H_FILE_H

struct interrupt_frame;
void *isr80h_command10_read(struct interrupt_frame *frame);

#endif