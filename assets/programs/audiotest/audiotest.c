#include "vios.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

// Audio system call numbers
#define SYSTEM_COMMAND12_AUDIO_PUSH 12
#define SYSTEM_COMMAND13_AUDIO_POP 13
#define SYSTEM_COMMAND14_AUDIO_CONTROL 14

// Audio control commands
#define VIRTUAL_AUDIO_VOLUME_UP 0x01
#define VIRTUAL_AUDIO_VOLUME_DOWN 0x02
#define VIRTUAL_AUDIO_MUTE_TOGGLE 0x03
#define VIRTUAL_AUDIO_PLAY 0x04
#define VIRTUAL_AUDIO_PAUSE 0x05
#define VIRTUAL_AUDIO_STOP 0x06

void audio_push(char c)
{
    vios_audio_push(c);
}

char audio_pop()
{
    return vios_audio_pop();
}

void audio_control(int command)
{
    vios_audio_control(command);
}

int main(int argc, char **argv)
{
    print("=== SoundBlaster16 + Virtual Audio Layer Test ===");
    print("This program demonstrates the integrated audio layer.");
    
    // Test audio buffer operations
    print("Testing audio buffer operations...");
    
    // Push some audio data
    print("Pushing audio data: 'A', 'B', 'C'");
    audio_push('A');
    audio_push('B');
    audio_push('C');
    
    // Pop and display audio data
    print("Popping audio data:");
    char c1 = audio_pop();
    char c2 = audio_pop();
    char c3 = audio_pop();
    
    // Display the popped characters using vios_print
    vios_print("Retrieved: ", 10, 200, 255, 255, 255, 1);
    vios_putchar(c1, 120, 200, 255, 255, 255, 1);
    vios_putchar(',', 140, 200, 255, 255, 255, 1);
    vios_putchar(c2, 160, 200, 255, 255, 255, 1);
    vios_putchar(',', 180, 200, 255, 255, 255, 1);
    vios_putchar(c3, 200, 200, 255, 255, 255, 1);
    
    // Test audio control commands (will use SoundBlaster if available)
    print("Testing audio control commands...");
    
    print("Sending VOLUME_UP command (SoundBlaster integration)");
    audio_control(VIRTUAL_AUDIO_VOLUME_UP);
    vios_sleep(1);
    
    print("Sending VOLUME_DOWN command (SoundBlaster integration)");
    audio_control(VIRTUAL_AUDIO_VOLUME_DOWN);
    vios_sleep(1);
    
    print("Sending MUTE_TOGGLE command (SoundBlaster integration)");
    audio_control(VIRTUAL_AUDIO_MUTE_TOGGLE);
    vios_sleep(1);
    
    print("Sending PLAY command (SoundBlaster integration)");
    audio_control(VIRTUAL_AUDIO_PLAY);
    vios_sleep(1);
    
    print("Sending PAUSE command (SoundBlaster integration)");
    audio_control(VIRTUAL_AUDIO_PAUSE);
    vios_sleep(1);
    
    print("Sending STOP command (SoundBlaster integration)");
    audio_control(VIRTUAL_AUDIO_STOP);
    vios_sleep(1);
    
    print("SoundBlaster16 + Virtual Audio Layer Test Complete!");
    print("Press any key to continue...");
    vios_getkeyblock();
    
    return 0;
}
