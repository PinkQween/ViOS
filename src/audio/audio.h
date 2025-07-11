#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

#define AUDIO_MUTE_ON 1
#define AUDIO_MUTE_OFF 0

// Virtual audio control commands
#define VIRTUAL_AUDIO_VOLUME_UP     0x01
#define VIRTUAL_AUDIO_VOLUME_DOWN   0x02
#define VIRTUAL_AUDIO_MUTE_TOGGLE   0x03
#define VIRTUAL_AUDIO_PLAY          0x04
#define VIRTUAL_AUDIO_PAUSE         0x05
#define VIRTUAL_AUDIO_STOP          0x06
#define VIRTUAL_AUDIO_BEEP          0x07
#define VIRTUAL_AUDIO_RESET         0x08
#define VIRTUAL_AUDIO_BEEP_LOW      0x09
#define VIRTUAL_AUDIO_BEEP_HIGH     0x0A

typedef int AUDIO_MUTE_STATE;

struct process;

typedef int (*AUDIO_INIT_FUNCTION)();
struct audio
{
    AUDIO_INIT_FUNCTION init;
    char name[20];
    AUDIO_MUTE_STATE mute_state;
    struct audio *next;
};

void audio_init();
void audio_stop(struct process *process);
void audio_push(char c);
char audio_pop();
int audio_insert(struct audio *audio);
void audio_set_mute(struct audio *audio, AUDIO_MUTE_STATE state);
AUDIO_MUTE_STATE audio_get_mute(struct audio *audio);
void virtual_audio_control(uint8_t command);
void virtual_audio_beep_frequency(uint32_t frequency);

#endif
