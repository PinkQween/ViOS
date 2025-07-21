#include <ViOS/ViOS.h>
#include <stdio.h>

#define BEEP_FREQ 440       // Frequency in Hz (A4 note)
#define BEEP_DURATION 30000 // 30 seconds in milliseconds
#define OFF_DURATION 300    // 300 milliseconds

int main(void)
{
    printf("Started\n");
    // Initial wait: 3.75 seconds (3750 ms)
    vios_sys_sleep(3750);
    printf("Starting beeping...\n");

    while (1)
    {
        // Beep for 30 seconds
        vios_sys_audio(AUDIO_CMD_BEEP, BEEP_FREQ, BEEP_DURATION);
        printf("Beeping for 30 seconds...\n");

        // Sleep for the beep duration (multitasking-friendly)
        vios_sys_sleep(BEEP_DURATION);

        // Turn audio off
        vios_sys_audio(AUDIO_CMD_STOP);
        vios_sys_sleep(OFF_DURATION);
        printf("Audio off for 300 ms, resuming beeping...\n");
    }

    return 0;
}