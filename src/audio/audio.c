#include "audio.h"
#include "status.h"
#include "kernel.h"
#include "task/process.h"
#include "task/task.h"
#include "sb16.h"
#include "io/io.h"
static struct audio *audio_list_head = 0;
static struct audio *audio_list_last = 0;

void audio_init()
{
    audio_insert(sb16_audio_init());
}

int audio_insert(struct audio *audio)
{
    int res = 0;
    if (audio->init == 0)
    {
        res = -EINVARG;
        goto out;
    }

    if (audio_list_last)
    {
        audio_list_last->next = audio;
        audio_list_last = audio;
    }
    else
    {
        audio_list_head = audio;
        audio_list_last = audio;
    }

    res = audio->init();
out:
    return res;
}

static int audio_get_tail_index(struct process *process)
{
    return process->audio.tail % sizeof(process->audio.buffer);
}

void audio_set_mute(struct audio *audio, AUDIO_MUTE_STATE state)
{
    audio->mute_state = state;
}

AUDIO_MUTE_STATE audio_get_mute(struct audio *audio)
{
    return audio->mute_state;
}

void audio_stop(struct process *process)
{
    process->audio.tail -= 1;
    int real_index = audio_get_tail_index(process);
    process->audio.buffer[real_index] = 0x00;
}

void audio_push(char c)
{
    struct process *process = process_current();
    if (!process)
    {
        return;
    }

    if (c == 0)
    {
        return;
    }

    int real_index = audio_get_tail_index(process);
    process->audio.buffer[real_index] = c;
    process->audio.tail++;
}

char audio_pop()
{
    if (!task_current())
    {
        return 0;
    }

    struct process *process = task_current()->process;
    int real_index = process->audio.head % sizeof(process->audio.buffer);
    char c = process->audio.buffer[real_index];
    if (c == 0x00)
    {
        // Nothing to pop return zero.
        return 0;
    }

    process->audio.buffer[real_index] = 0;
    process->audio.head++;
    return c;
}

void virtual_audio_control(uint8_t command)
{
    struct audio *audio_device = audio_list_head;
    
    if (!audio_device) {
        return; // No audio device available
    }
    
    switch (command) {
        case VIRTUAL_AUDIO_VOLUME_UP:
            sb16_set_master_volume(200);
            sb16_set_pcm_volume(200);
            break;
            
        case VIRTUAL_AUDIO_VOLUME_DOWN:
            sb16_set_master_volume(100);
            sb16_set_pcm_volume(100);
            break;
            
        case VIRTUAL_AUDIO_MUTE_TOGGLE:
            sb16_set_master_volume(0);
            sb16_set_pcm_volume(0);
            break;
            
        case VIRTUAL_AUDIO_PLAY:
            sb16_speaker_on();
            break;
            
        case VIRTUAL_AUDIO_PAUSE:
            sb16_stop_playback();
            break;
            
        case VIRTUAL_AUDIO_STOP:
            sb16_stop_playback();
            sb16_stop_beep();
            // Disable PC speaker
            {
                uint8_t port61 = inb(0x61);
                outb(0x61, port61 & 0xFC);
            }
            break;
            
        case VIRTUAL_AUDIO_BEEP:
            // Use PC speaker as fallback for reliable audio
            // Enable PC speaker
            {
                uint8_t port61 = inb(0x61);
                outb(0x61, port61 | 0x03);
                
                // Configure PIT channel 2 for 1000 Hz
                outb(0x43, 0xB6);
                uint16_t divisor = 1193180 / 1000;
                outb(0x42, divisor & 0xFF);
                outb(0x42, (divisor >> 8) & 0xFF);
            }
            break;
            
        case VIRTUAL_AUDIO_BEEP_LOW:
            // Use PC speaker for 500 Hz
            {
                uint8_t port61 = inb(0x61);
                outb(0x61, port61 | 0x03);
                
                // Configure PIT channel 2 for 500 Hz
                outb(0x43, 0xB6);
                uint16_t divisor = 1193180 / 500;
                outb(0x42, divisor & 0xFF);
                outb(0x42, (divisor >> 8) & 0xFF);
            }
            break;
            
        case VIRTUAL_AUDIO_BEEP_HIGH:
            // Use PC speaker for 2000 Hz
            {
                uint8_t port61 = inb(0x61);
                outb(0x61, port61 | 0x03);
                
                // Configure PIT channel 2 for 2000 Hz
                outb(0x43, 0xB6);
                uint16_t divisor = 1193180 / 2000;
                outb(0x42, divisor & 0xFF);
                outb(0x42, (divisor >> 8) & 0xFF);
            }
            break;
            
        case VIRTUAL_AUDIO_RESET:
            // Reset the audio device
            sb16_reset();
            // Re-enable speaker after reset
            sb16_speaker_on();
            sb16_set_master_volume(200);
            sb16_set_pcm_volume(200);
            break;
            
        default:
            // Unknown command, ignore
            break;
    }
}

void virtual_audio_beep_frequency(uint32_t frequency)
{
    struct audio *audio_device = audio_list_head;
    
    if (!audio_device) {
        return; // No audio device available
    }
    
    // Play beep at specified frequency
    sb16_play_beep(frequency);
}
