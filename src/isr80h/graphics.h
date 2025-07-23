#ifndef ISR80H_GRAPHICS_H
#define ISR80H_GRAPHICS_H

struct interrupt_frame;

void *isr80h_command_flush(struct interrupt_frame *frame);
void *isr80h_command_draw_pixel(struct interrupt_frame *frame);

#endif