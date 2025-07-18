#ifndef ISR80H_AUDIO_H
#define ISR80H_AUDIO_H

// Subcommand definitions
#define AUDIO_CMD_BEEP 0
#define AUDIO_CMD_PLAY_PCM 1
#define AUDIO_CMD_STOP 2
#define AUDIO_CMD_PAUSE 3
#define AUDIO_CMD_RESUME 4
#define AUDIO_CMD_VOLUME 5

void *isr80h_command_audio(struct interrupt_frame *frame);

#endif