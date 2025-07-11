#ifndef SB16_H
#define SB16_H

#include <stdint.h>
#include <stdbool.h>
#include "audio.h"

// Sound Blaster 16 I/O Port Addresses
#define SB16_BASE_PORT          0x220
#define SB16_MIXER_ADDR         (SB16_BASE_PORT + 0x04)
#define SB16_MIXER_DATA         (SB16_BASE_PORT + 0x05)
#define SB16_RESET              (SB16_BASE_PORT + 0x06)
#define SB16_READ_DATA          (SB16_BASE_PORT + 0x0A)
#define SB16_WRITE_DATA         (SB16_BASE_PORT + 0x0C)
#define SB16_WRITE_STATUS       (SB16_BASE_PORT + 0x0C)
#define SB16_READ_STATUS        (SB16_BASE_PORT + 0x0E)
#define SB16_INT_ACK_16         (SB16_BASE_PORT + 0x0F)

// Sound Blaster 16 Commands
#define SB16_CMD_SET_OUTPUT_RATE    0x41
#define SB16_CMD_SET_INPUT_RATE     0x42
#define SB16_CMD_SPEAKER_ON         0xD1
#define SB16_CMD_SPEAKER_OFF        0xD3
#define SB16_CMD_PAUSE_DMA          0xD0
#define SB16_CMD_CONTINUE_DMA       0xD4
#define SB16_CMD_GET_VERSION        0xE1
#define SB16_CMD_AUTO_INIT_DMA      0xB0
#define SB16_CMD_SINGLE_DMA         0xB0
#define SB16_CMD_PROG_16BIT_DMA     0xB0

// DMA and IRQ Configuration
#define SB16_DMA_CHANNEL_8      1
#define SB16_DMA_CHANNEL_16     5
#define SB16_IRQ_LINE           5

// Sample rates and formats
#define SB16_SAMPLE_RATE_11025  11025
#define SB16_SAMPLE_RATE_22050  22050
#define SB16_SAMPLE_RATE_44100  44100

#define SB16_FORMAT_MONO_8      0x00
#define SB16_FORMAT_STEREO_8    0x20
#define SB16_FORMAT_MONO_16     0x10
#define SB16_FORMAT_STEREO_16   0x30

// Audio buffer configuration
#define SB16_BUFFER_SIZE        4096
#define SB16_MAX_BUFFERS        4

// Sound Blaster 16 device structure
typedef struct {
    uint16_t base_port;
    uint8_t irq;
    uint8_t dma_channel_8;
    uint8_t dma_channel_16;
    uint8_t version_major;
    uint8_t version_minor;
    bool initialized;
    bool speaker_on;
    uint32_t sample_rate;
    uint8_t format;
    bool playing;
} sb16_device_t;

// Audio buffer structure
typedef struct {
    uint8_t *data;
    uint32_t size;
    uint32_t position;
    bool in_use;
} sb16_buffer_t;

// Audio context structure
typedef struct {
    sb16_device_t device;
    sb16_buffer_t buffers[SB16_MAX_BUFFERS];
    uint32_t current_buffer;
    uint32_t buffer_count;
    bool double_buffering;
    
    // Playback state
    bool loop_playback;
    uint32_t bytes_played;
    uint32_t total_samples;
    
    // Mixer settings
    uint8_t master_volume;
    uint8_t pcm_volume;
    uint8_t line_volume;
    uint8_t mic_volume;
} sb16_context_t;

// Audio interface
struct audio *sb16_audio_init();
bool sb16_reset(void);
bool sb16_detect(void);
uint16_t sb16_get_version(void);

// Speaker control
void sb16_speaker_on(void);
void sb16_speaker_off(void);

// Sample rate and format
bool sb16_set_sample_rate(uint32_t rate);
bool sb16_set_format(uint8_t format);

// DMA and playback
bool sb16_start_playback(uint8_t *data, uint32_t size, bool loop);
void sb16_stop_playback(void);
bool sb16_is_playing(void);

// Buffer management
bool sb16_create_buffer(uint32_t buffer_id, uint32_t size);
void sb16_destroy_buffer(uint32_t buffer_id);
bool sb16_load_buffer(uint32_t buffer_id, uint8_t *data, uint32_t size);
bool sb16_play_buffer(uint32_t buffer_id, bool loop);

// Mixer functions
void sb16_set_master_volume(uint8_t volume);
void sb16_set_pcm_volume(uint8_t volume);
void sb16_set_line_volume(uint8_t volume);
void sb16_set_mic_volume(uint8_t volume);

// Tone generation
void sb16_generate_tone(uint32_t frequency, uint32_t duration_ms);
void sb16_play_beep(uint32_t frequency);
void sb16_stop_beep(void);
void sb16_simple_playback(uint8_t *data, uint32_t size);

// Audio layer integration
void sb16_audio_control(uint8_t command);
void sb16_audio_process_buffer(struct process *process);

// Interrupt handling
void sb16_interrupt_handler(void);

// Utility functions
bool sb16_wait_for_ready(void);
bool sb16_write_dsp(uint8_t data);
uint8_t sb16_read_dsp(void);
void sb16_write_mixer(uint8_t reg, uint8_t value);
uint8_t sb16_read_mixer(uint8_t reg);

// Internal functions
void _sb16_setup_dma(uint8_t *buffer, uint32_t size, bool is_16bit);
void _sb16_program_dma_controller(uint8_t channel, uint32_t address, uint32_t size);
uint8_t* _sb16_generate_sine_wave(uint32_t frequency, uint32_t sample_rate, uint32_t duration_samples);
uint8_t* _sb16_generate_square_wave(uint32_t frequency, uint32_t sample_rate, uint32_t duration_samples);

// Global context
extern sb16_context_t g_sb16_context;

#endif // SB16_H
