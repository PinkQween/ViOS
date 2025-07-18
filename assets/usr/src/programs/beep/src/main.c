#include <ViOS/syscall.h>

#define BEEP_FREQ 440       // Frequency in Hz (A4 note)
#define BEEP_DURATION 30000 // 30 seconds in milliseconds
#define OFF_DURATION 300    // 300 milliseconds

int main(void)
{
    // Initial wait: 3.75 seconds (3750 ms)
    vios_sys_sleep(3750);

    while (1)
    {
        // Beep for 30 seconds
        vios_sys_audio(AUDIO_CMD_BEEP, BEEP_FREQ, BEEP_DURATION);

        // Turn audio off for 300 ms
        vios_sys_audio(AUDIO_CMD_STOP);
        vios_sys_sleep(OFF_DURATION);

        // Resume beeping for 30 seconds
        vios_sys_audio(AUDIO_CMD_BEEP, BEEP_FREQ, BEEP_DURATION);

        // Turn audio off for 300 ms before repeating
        vios_sys_audio(AUDIO_CMD_STOP);
        vios_sys_sleep(OFF_DURATION);
    }

    return 0;
}