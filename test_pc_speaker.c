// Simple PC speaker test - add this to kernel.c for testing

static void play_pc_speaker_melody(void) {
    // C major scale using PC speaker
    int frequencies[] = {261, 294, 329, 349, 392, 440, 494, 523}; // C, D, E, F, G, A, B, C
    int note_duration = 300; // milliseconds
    int pause_duration = 50;
    
    for (int i = 0; i < 8; i++) {
        pc_speaker_beep(frequencies[i], note_duration);
        sleep_ms(pause_duration);
    }
    
    // Add a longer pause and repeat
    sleep_ms(500);
    
    // Play it again to confirm looping
    for (int i = 0; i < 8; i++) {
        pc_speaker_beep(frequencies[i], note_duration);
        sleep_ms(pause_duration);
    }
}

// Replace the audio initialization with this simple version:
static void kernel_init_audio_simple() {
    // Test 1: Single beep
    pc_speaker_beep(1000, 1000);  // 1 second beep at 1000Hz
    sleep_ms(500);
    
    // Test 2: Scale
    play_pc_speaker_melody();
    
    // Test 3: Continuous loop would go here
    // For now, just one more beep
    pc_speaker_beep(800, 500);
}
