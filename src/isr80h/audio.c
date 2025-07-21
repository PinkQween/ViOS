#include "isr80h.h"
#include "drivers/output/audio/audio.h"
#include "task/task.h"
#include "status.h"
#include "kernel.h"
#include <stddef.h>
#include <stdint.h>

// Subcommand definitions
#define AUDIO_CMD_BEEP 0
#define AUDIO_CMD_PLAY_PCM 1
#define AUDIO_CMD_STOP 2
#define AUDIO_CMD_PAUSE 3
#define AUDIO_CMD_RESUME 4
#define AUDIO_CMD_VOLUME 5

void *isr80h_command_audio(struct interrupt_frame *frame)
{
    int subcmd = (int)task_get_stack_item(task_current(), 0);
    switch (subcmd)
    {
    case AUDIO_CMD_BEEP:
    {
        // Args: freq (Hz), duration (ms)
        int freq = (int)task_get_stack_item(task_current(), 1);
        int duration = (int)task_get_stack_item(task_current(), 2);
        if (freq <= 0 || duration <= 0)
            return ERROR(-EINVARG);
        virtual_audio_beep_frequency((uint32_t)freq);
        // Do not block or stop here; userland will handle sleep and stop
        return 0;
    }
    case AUDIO_CMD_PLAY_PCM:
    {
        // Args: pcm_ptr, size, sample_rate, format
        void *pcm_ptr = task_get_stack_item(task_current(), 1);
        size_t size = (size_t)task_get_stack_item(task_current(), 2);
        int sample_rate = (int)task_get_stack_item(task_current(), 3);
        // int format = (int)task_get_stack_item(task_current(), 4); // Unused for now
        if (!pcm_ptr || size == 0 || sample_rate <= 0)
            return ERROR(-EINVARG);
        // Use the generic audio API to play PCM (auto-select device)
        // For now, only support 8-bit mono, but let the backend decide
        // This assumes virtual_audio_control or a similar function can handle PCM
        // If not, this is a placeholder for future expansion
        // Example: virtual_audio_play_pcm(pcm_ptr, size, sample_rate, format);
        // (Not implemented yet)
        return ERROR(-EIO); // Not yet implemented in generic API
    }
    case AUDIO_CMD_STOP:
        virtual_audio_control(VIRTUAL_AUDIO_STOP);
        return 0;
    case AUDIO_CMD_PAUSE:
        virtual_audio_control(VIRTUAL_AUDIO_PAUSE);
        return 0;
    case AUDIO_CMD_RESUME:
        virtual_audio_control(VIRTUAL_AUDIO_PLAY);
        return 0;
    case AUDIO_CMD_VOLUME:
    {
        // int level = (int)task_get_stack_item(task_current(), 1);
        // // Use generic API if available, otherwise fallback
        // // Example: virtual_audio_set_volume(level);
        // // (Not implemented yet)
        return ERROR(-EIO); // Not yet implemented in generic API
    }
    default:
        return ERROR(-EINVARG);
    }
}