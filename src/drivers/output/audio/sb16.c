#include "sb16.h"
#include "audio.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "drivers/io/io.h"
#include "string/string.h"
#include "status.h"
#include "math/fpu_math.h"
#include "drivers/io/timing/timer.h"
#include "idt/idt.h"
#include "task/process.h"
#include "task/task.h"
#include "config.h"

// Global context
sb16_context_t g_sb16_context;

// DMA controller ports
#define DMA_ADDR_0 0x00
#define DMA_COUNT_0 0x01
#define DMA_ADDR_1 0x02
#define DMA_COUNT_1 0x03
#define DMA_ADDR_2 0x04
#define DMA_COUNT_2 0x05
#define DMA_ADDR_3 0x06
#define DMA_COUNT_3 0x07
#define DMA_STATUS_CMD 0x08
#define DMA_REQUEST 0x09
#define DMA_MASK_REG 0x0A
#define DMA_MODE_REG 0x0B
#define DMA_CLEAR_FF 0x0C
#define DMA_TEMP_REG 0x0D
#define DMA_MASTER_CLR 0x0E
#define DMA_CLEAR_MASK 0x0F

// DMA page registers
#define DMA_PAGE_0 0x87
#define DMA_PAGE_1 0x83
#define DMA_PAGE_2 0x81
#define DMA_PAGE_3 0x82
#define DMA_PAGE_5 0x8B
#define DMA_PAGE_6 0x89
#define DMA_PAGE_7 0x8A

// Static buffer for continuous beep
static uint8_t *beep_buffer = NULL;
static uint32_t beep_buffer_size = 0;
static bool beep_playing = false;

// Forward declarations
static int sb16_device_init();
bool sb16_init(void);

// Simple microsecond delay function
static void sleep_us(int us)
{
    // Simple busy wait for microseconds
    for (volatile int i = 0; i < us * 10; i++)
    {
        // Just wait
    }
}

// Enable IRQ 5 for Sound Blaster
static void sb16_enable_irq(void)
{
    // Read current mask from master PIC
    uint8_t mask = inb(0x21);
    // Clear bit 5 to enable IRQ 5
    mask &= ~(1 << 5);
    // Write back the mask
    outb(0x21, mask);
}

// Disable IRQ 5 for Sound Blaster
static void sb16_disable_irq(void)
{
    // Read current mask from master PIC
    uint8_t mask = inb(0x21);
    // Set bit 5 to disable IRQ 5
    mask |= (1 << 5);
    // Write back the mask
    outb(0x21, mask);
}

// Sound Blaster interrupt handler wrapper
static void sb16_interrupt_wrapper(struct interrupt_frame *frame)
{
    sb16_interrupt_handler();
}

// SoundBlaster audio device structure
static struct audio sb16_audio_device = {
    .name = {"SoundBlaster16"},
    .init = sb16_device_init};

// Internal initialization function
static int sb16_device_init()
{
    return sb16_init() ? 0 : -1;
}

// Public audio interface
struct audio *sb16_audio_init()
{
    return &sb16_audio_device;
}

bool sb16_init(void)
{
    // Reset the DSP
    if (!sb16_reset())
        return false;

    // Detect Sound Blaster
    if (!sb16_detect())
        return false;

    // Register interrupt handler for IRQ 5 (interrupt 0x25)
    int irq_interrupt = 0x20 + SB16_IRQ_LINE; // 0x25 for IRQ 5
    if (idt_register_interrupt_callback(irq_interrupt, sb16_interrupt_wrapper) != 0)
    {
        return false;
    }

    // Enable IRQ 5
    sb16_enable_irq();

    // Enable speaker
    sb16_speaker_on();

    // Initialize mixer settings
    sb16_set_master_volume(200); // Set high volume
    sb16_set_pcm_volume(200);    // Set high PCM volume

    // Initialize context
    g_sb16_context.master_volume = 200;
    g_sb16_context.pcm_volume = 200;
    g_sb16_context.device.sample_rate = SB16_SAMPLE_RATE_22050;
    g_sb16_context.device.format = SB16_FORMAT_MONO_8;

    // Initialize buffers
    for (int i = 0; i < SB16_MAX_BUFFERS; ++i)
    {
        sb16_create_buffer(i, SB16_BUFFER_SIZE);
    }

    g_sb16_context.device.initialized = true;
    return true;
}

void sb16_shutdown(void)
{
    // Stop playback
    sb16_stop_playback();

    // Disable speaker
    sb16_speaker_off();

    // Disable IRQ 5
    sb16_disable_irq();

    // Destroy buffers
    for (int i = 0; i < SB16_MAX_BUFFERS; ++i)
    {
        sb16_destroy_buffer(i);
    }

    g_sb16_context.device.initialized = false;
}

void sb16_audio_control(uint8_t command)
{
    switch (command)
    {
    case VIRTUAL_AUDIO_VOLUME_UP:
        sb16_set_master_volume(g_sb16_context.master_volume + 10);
        break;
    case VIRTUAL_AUDIO_VOLUME_DOWN:
        sb16_set_master_volume(g_sb16_context.master_volume - 10);
        break;
    case VIRTUAL_AUDIO_MUTE_TOGGLE:
        // Toggle mute here
        break;
    case VIRTUAL_AUDIO_PLAY:
        // Playback control
        break;
    case VIRTUAL_AUDIO_PAUSE:
        sb16_stop_playback();
        break;
    case VIRTUAL_AUDIO_STOP:
        sb16_stop_playback();
        break;
    }
}
void sb16_audio_process_buffer(struct process *process)
{
    if (!process || !g_sb16_context.device.initialized)
    {
        return;
    }

    int head_index = process->audio.head % VIOS_AUDIO_BUFFER_SIZE;
    int tail_index = process->audio.tail % VIOS_AUDIO_BUFFER_SIZE;

    // Process audio data in the buffer
    while (head_index != tail_index)
    {
        char c = process->audio.buffer[head_index];
        if (c != 0)
        {
            // Play the audio data
            sb16_start_playback((uint8_t *)&c, 1, false);
        }
        head_index = (head_index + 1) % VIOS_AUDIO_BUFFER_SIZE;
    }

    // Update the head position
    process->audio.head = head_index;
}

bool sb16_reset(void)
{
    // Write to reset port
    outb(SB16_RESET, 1);
    timer_sleep_ms(3); // Wait longer for reset
    outb(SB16_RESET, 0);

    // Wait for DSP ready signal (0xAA)
    for (int i = 0; i < 1000; i++)
    {
        if ((inb(SB16_READ_STATUS) & 0x80) != 0)
        {
            uint8_t data = inb(SB16_READ_DATA);
            if (data == 0xAA)
            {
                return true; // DSP reset successful
            }
        }
        sleep_us(100);
    }

    return false; // Reset failed
}

bool sb16_detect(void)
{
    // Query DSP version
    uint16_t version = sb16_get_version();
    g_sb16_context.device.version_major = (version >> 8) & 0xFF;
    g_sb16_context.device.version_minor = version & 0xFF;

    // Accept any version that responds (for emulation compatibility)
    return (g_sb16_context.device.version_major > 0);
}

uint16_t sb16_get_version(void)
{
    if (!sb16_write_dsp(SB16_CMD_GET_VERSION))
        return 0;

    // Wait for data to be available
    for (int i = 0; i < 100; i++)
    {
        if ((inb(SB16_READ_STATUS) & 0x80) != 0)
        {
            uint8_t major = inb(SB16_READ_DATA);
            // Wait for second byte
            for (int j = 0; j < 100; j++)
            {
                if ((inb(SB16_READ_STATUS) & 0x80) != 0)
                {
                    uint8_t minor = inb(SB16_READ_DATA);
                    return (major << 8) | minor;
                }
                sleep_us(10);
            }
            break;
        }
        sleep_us(10);
    }

    return 0; // Failed to get version
}

void sb16_speaker_on(void)
{
    sb16_write_dsp(SB16_CMD_SPEAKER_ON);
    g_sb16_context.device.speaker_on = true;
}

void sb16_speaker_off(void)
{
    sb16_write_dsp(SB16_CMD_SPEAKER_OFF);
    g_sb16_context.device.speaker_on = false;
}

bool sb16_set_sample_rate(uint32_t rate)
{
    g_sb16_context.device.sample_rate = rate;
    sb16_write_dsp(SB16_CMD_SET_OUTPUT_RATE);
    sb16_write_dsp(rate >> 8);
    sb16_write_dsp(rate & 0xFF);
    return true;
}

void sb16_play_beep(uint32_t frequency)
{
    // Stop any existing beep
    sb16_stop_beep();

    // Try direct DSP programming for a simple beep
    if (!g_sb16_context.device.initialized)
    {
        return;
    }

    // Enable speaker
    sb16_speaker_on();

    // Set high volume
    sb16_set_master_volume(255);
    sb16_set_pcm_volume(255);

    // Generate a simple square wave buffer
    beep_buffer_size = 8192; // Larger buffer
    beep_buffer = _sb16_generate_square_wave(frequency, SB16_SAMPLE_RATE_11025, beep_buffer_size);

    if (beep_buffer)
    {
        beep_playing = true;
        // Try simpler playback
        sb16_simple_playback(beep_buffer, beep_buffer_size);
    }
}

// Simple direct playback without complex DMA
void sb16_simple_playback(uint8_t *data, uint32_t size)
{
    if (!data || size == 0)
        return;

    // Set sample rate using time constant for older SB compatibility
    uint8_t time_constant = 256 - (1000000 / 22050);
    sb16_write_dsp(0x40); // Set time constant
    sb16_write_dsp(time_constant);

    // Use direct DAC output mode
    sb16_write_dsp(0x14);                     // 8-bit single-cycle DMA output
    sb16_write_dsp((size - 1) & 0xFF);        // Low byte of length
    sb16_write_dsp(((size - 1) >> 8) & 0xFF); // High byte of length
}

void sb16_stop_beep(void)
{
    beep_playing = false;
    sb16_stop_playback();

    // Clean up beep buffer
    if (beep_buffer)
    {
        kfree(beep_buffer);
        beep_buffer = NULL;
        beep_buffer_size = 0;
    }
}

// Internal function implementations
bool sb16_wait_for_ready(void)
{
    for (int i = 0; i < 100; ++i)
    {
        if ((inb(SB16_WRITE_STATUS) & 0x80) == 0)
            return true;
        sleep_us(10);
    }
    return false;
}

bool sb16_write_dsp(uint8_t data)
{
    if (!sb16_wait_for_ready())
        return false;
    outb(SB16_WRITE_DATA, data);
    return true;
}

uint8_t sb16_read_dsp(void)
{
    return inb(SB16_READ_DATA);
}

void sb16_write_mixer(uint8_t reg, uint8_t value)
{
    outb(SB16_MIXER_ADDR, reg);
    outb(SB16_MIXER_DATA, value);
}

uint8_t sb16_read_mixer(uint8_t reg)
{
    outb(SB16_MIXER_ADDR, reg);
    return inb(SB16_MIXER_DATA);
}

bool sb16_create_buffer(uint32_t buffer_id, uint32_t size)
{
    if (buffer_id >= SB16_MAX_BUFFERS)
        return false;

    sb16_buffer_t *buffer = &g_sb16_context.buffers[buffer_id];
    buffer->data = (uint8_t *)kmalloc(size);
    if (!buffer->data)
        return false;

    buffer->size = size;
    buffer->position = 0;
    buffer->in_use = false;
    return true;
}

void sb16_destroy_buffer(uint32_t buffer_id)
{
    if (buffer_id >= SB16_MAX_BUFFERS)
        return;

    sb16_buffer_t *buffer = &g_sb16_context.buffers[buffer_id];
    if (buffer->data)
    {
        kfree(buffer->data);
        buffer->data = NULL;
    }
    buffer->size = 0;
    buffer->position = 0;
    buffer->in_use = false;
}

bool sb16_start_playback(uint8_t *data, uint32_t size, bool loop)
{
    if (!g_sb16_context.device.initialized)
        return false;

    // Set up DMA for 8-bit mono playback
    _sb16_setup_dma(data, size, false);

    // Set sample rate
    sb16_set_sample_rate(SB16_SAMPLE_RATE_22050);

    // Program the DSP for 8-bit auto-init DMA (for looping)
    if (loop)
    {
        sb16_write_dsp(0x1C); // 8-bit auto-init DMA
    }
    else
    {
        sb16_write_dsp(0x14); // 8-bit single-cycle DMA
    }
    sb16_write_dsp((size - 1) & 0xFF);        // Low byte of length
    sb16_write_dsp(((size - 1) >> 8) & 0xFF); // High byte of length

    g_sb16_context.device.playing = true;
    g_sb16_context.loop_playback = loop;

    return true;
}

void sb16_stop_playback(void)
{
    if (!g_sb16_context.device.playing)
        return;

    // Stop DMA
    sb16_write_dsp(SB16_CMD_PAUSE_DMA);

    // Mask DMA channel
    outb(DMA_MASK_REG, 0x04 | SB16_DMA_CHANNEL_8);

    g_sb16_context.device.playing = false;
    beep_playing = false;
}

bool sb16_is_playing(void)
{
    return g_sb16_context.device.playing;
}

void _sb16_setup_dma(uint8_t *buffer, uint32_t size, bool is_16bit)
{
    uint8_t channel = is_16bit ? SB16_DMA_CHANNEL_16 : SB16_DMA_CHANNEL_8;
    uint32_t physical_addr = (uint32_t)buffer;

    // For channel 1 (8-bit SoundBlaster)
    if (channel == 1)
    {
        // Mask the DMA channel
        outb(DMA_MASK_REG, 0x04 | channel);

        // Clear the flip-flop
        outb(DMA_CLEAR_FF, 0x00);

        // Set mode - single mode, increment, read, channel 1
        outb(DMA_MODE_REG, 0x48 | channel); // 0x49 for channel 1

        // Set address (channel 1 uses ports 0x02/0x03)
        outb(DMA_ADDR_1, physical_addr & 0xFF);
        outb(DMA_ADDR_1, (physical_addr >> 8) & 0xFF);

        // Set count
        outb(DMA_COUNT_1, (size - 1) & 0xFF);
        outb(DMA_COUNT_1, ((size - 1) >> 8) & 0xFF);

        // Set page (channel 1 uses page register 0x83)
        outb(DMA_PAGE_1, (physical_addr >> 16) & 0xFF);

        // Unmask the DMA channel
        outb(DMA_MASK_REG, channel);
    }
}

uint8_t *_sb16_generate_square_wave(uint32_t frequency, uint32_t sample_rate, uint32_t duration_samples)
{
    uint8_t *buffer = (uint8_t *)kmalloc(duration_samples);
    if (!buffer)
        return NULL;

    uint32_t samples_per_cycle = sample_rate / frequency;
    uint32_t half_cycle = samples_per_cycle / 2;

    for (uint32_t i = 0; i < duration_samples; i++)
    {
        uint32_t cycle_pos = i % samples_per_cycle;
        buffer[i] = (cycle_pos < half_cycle) ? 200 : 55; // High/Low values
    }

    return buffer;
}

uint8_t *_sb16_generate_sine_wave(uint32_t frequency, uint32_t sample_rate, uint32_t duration_samples)
{
    uint8_t *buffer = (uint8_t *)kmalloc(duration_samples);
    if (!buffer)
        return NULL;

    for (uint32_t i = 0; i < duration_samples; i++)
    {
        double angle = 2.0 * 3.14159 * frequency * i / sample_rate;
        double sine_val = fpu_sin(angle);
        buffer[i] = (uint8_t)((sine_val + 1.0) * 127.5); // Convert to 0-255 range
    }

    return buffer;
}

void sb16_set_master_volume(uint8_t volume)
{
    sb16_write_mixer(0x22, volume);
    g_sb16_context.master_volume = volume;
}

void sb16_set_pcm_volume(uint8_t volume)
{
    sb16_write_mixer(0x04, volume);
    g_sb16_context.pcm_volume = volume;
}

void sb16_interrupt_handler(void)
{
    // Acknowledge the interrupt from Sound Blaster
    inb(SB16_INT_ACK_16);

    // For auto-init mode, the DMA continues automatically
    // We just need to acknowledge the interrupt
    // The hardware will keep looping the buffer
}
