#ifndef ISR80H_AUDIO_H
#define ISR80H_AUDIO_H

struct interrupt_frame;
void *isr80h_command12_audio_push(struct interrupt_frame *frame);
void *isr80h_command13_audio_pop(struct interrupt_frame *frame);
void *isr80h_command14_audio_control(struct interrupt_frame *frame);

#endif
