#include "mouse.h"
#include "drivers/io/io.h"
#include "idt/idt.h"
#include "task/task.h"
#include "kernel.h"
#include "debug/simple_serial.h"
#include "drivers/output/vigfx/vigfx.h"
#include "string/string.h"

static struct mouse ps2_mouse;

// Forward declaration
void ps2_mouse_handle_packet(struct mouse *mouse);
void ps2_mouse_set_speed(float speed_multiplier);
void ps2_mouse_set_position(int x, int y);

// PS/2 constants
#define PS2_MOUSE_IRQ 12
#define PS2_COMMAND_PORT 0x64
#define PS2_DATA_PORT 0x60

// Mouse sensitivity settings (can be modified at runtime)
static int mouse_sensitivity_numerator = 1;
static int mouse_sensitivity_denominator = 2;  // This gives 0.5x speed (half speed)

// Mouse sample rate settings (samples per second)
// Common values: 10, 20, 40, 60, 80, 100, 200
// Higher values = more frequent interrupts = smoother movement
#define MOUSE_SAMPLE_RATE 200  // 200 Hz sample rate for smoothest movement

// Helper functions for PS/2 communication
static void ps2_wait_write()
{
    while (inb(PS2_COMMAND_PORT) & 0x02); // Wait until input buffer is empty
}

static void ps2_wait_read()
{
    while (!(inb(PS2_COMMAND_PORT) & 0x01)); // Wait until output buffer is full
}

static uint8_t packet[3];
static uint8_t packet_index = 0;

static int32_t mouse_x = 0;
static int32_t mouse_y = 0;
static int mouse_left = 0;
static int mouse_right = 0;
static int mouse_middle = 0;

void ps2_mouse_handle_interrupt(struct interrupt_frame *frame)
{
    simple_serial_puts("DEBUG: Mouse interrupt received!\n");
    uint8_t data = inb(PS2_DATA_PORT);

    // Byte 1 must always have bit 3 set
    if (packet_index == 0 && !(data & 0x08))
    {
        simple_serial_puts("DEBUG: Invalid first byte, bit 3 not set\n");
        return;
    }

    packet[packet_index++] = data;
    simple_serial_puts("DEBUG: Got packet byte, index now: ");
    simple_serial_putc('0' + packet_index);
    simple_serial_putc('\n');

    if (packet_index < 3)
        return;

    packet_index = 0;
    simple_serial_puts("DEBUG: Processing complete mouse packet\n");

    int dx = (int8_t)packet[1];
    int dy = (int8_t)packet[2];

    if (packet[0] & 0x10)
        dx |= 0xFFFFFF00;
    if (packet[0] & 0x20)
        dy |= 0xFFFFFF00;

    // Apply mouse sensitivity scaling
    dx = (dx * mouse_sensitivity_numerator) / mouse_sensitivity_denominator;
    dy = (dy * mouse_sensitivity_numerator) / mouse_sensitivity_denominator;

    mouse_x += dx;
    mouse_y -= dy; // Y is inverted

    // Boundary checking to keep mouse on screen (with safety checks)
    int screen_width = gpu_screen_width();
    int screen_height = gpu_screen_height();
    
    if (screen_width > 0 && screen_height > 0)
    {
        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x >= screen_width) mouse_x = screen_width - 1;
        if (mouse_y >= screen_height) mouse_y = screen_height - 1;
    }

    mouse_left = packet[0] & 0x01;
    mouse_right = packet[0] & 0x02;
    mouse_middle = packet[0] & 0x04;

    simple_serial_puts("DEBUG: Mouse position updated: x=");
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", mouse_x);
    simple_serial_puts(buf);
    simple_serial_puts(" y=");
    snprintf(buf, sizeof(buf), "%d", mouse_y);
    simple_serial_puts(buf);
    simple_serial_puts(" dx=");
    snprintf(buf, sizeof(buf), "%d", dx);
    simple_serial_puts(buf);
    simple_serial_puts(" dy=");
    snprintf(buf, sizeof(buf), "%d", dy);
    simple_serial_puts(buf);
    simple_serial_puts("\n");
    
    // Call handle_packet to update the struct
    ps2_mouse_handle_packet(&ps2_mouse);
    
    // Call generic mouse interrupt handler to notify the mouse system
    mouse_handle_interrupt();
}

int ps2_mouse_init_driver(struct mouse *mouse)
{
    simple_serial_puts("DEBUG: Starting PS/2 mouse initialization\n");
    
    // First, ensure master PIC allows slave PIC (IRQ 2)
    uint8_t master_mask = inb(0x21);
    simple_serial_puts("DEBUG: Current master PIC mask: ");
    char c1 = ((master_mask >> 0) & 0xF);
    char c2 = ((master_mask >> 4) & 0xF);
    simple_serial_putc(c1 < 10 ? '0' + c1 : 'A' + c1 - 10);
    simple_serial_putc(c2 < 10 ? '0' + c2 : 'A' + c2 - 10);
    simple_serial_puts("\n");
    
    master_mask &= ~(1 << 2); // Clear bit 2 for IRQ 2 (slave PIC)
    outb(0x21, master_mask);
    simple_serial_puts("DEBUG: Enabled IRQ 2 on master PIC for slave\n");
    
    // Enable auxiliary device (mouse)
    ps2_wait_write();
    outb(PS2_COMMAND_PORT, 0xA8);
    simple_serial_puts("DEBUG: Enabled auxiliary device\n");
    
    // Get current configuration byte
    ps2_wait_write();
    outb(PS2_COMMAND_PORT, 0x20);
    ps2_wait_read();
    uint8_t status = inb(PS2_DATA_PORT) | 2; // Enable mouse interrupts
    
    // Set new configuration byte
    ps2_wait_write();
    outb(PS2_COMMAND_PORT, 0x60);
    ps2_wait_write();
    outb(PS2_DATA_PORT, status);
    simple_serial_puts("DEBUG: Configured PS/2 controller\n");

    // Set mouse sample rate for smoother movement
    ps2_wait_write();
    outb(PS2_COMMAND_PORT, 0xD4); // Next byte goes to mouse
    ps2_wait_write();
    outb(PS2_DATA_PORT, 0xF3); // Set sample rate command
    ps2_wait_read();
    uint8_t sample_rate_ack = inb(PS2_DATA_PORT); // Read ACK
    simple_serial_puts("DEBUG: Sent F3 (set sample rate) command, ACK: 0x");
    char sr_ack_h = ((sample_rate_ack >> 4) & 0xF);
    char sr_ack_l = (sample_rate_ack & 0xF);
    simple_serial_putc(sr_ack_h < 10 ? '0' + sr_ack_h : 'A' + sr_ack_h - 10);
    simple_serial_putc(sr_ack_l < 10 ? '0' + sr_ack_l : 'A' + sr_ack_l - 10);
    simple_serial_putc('\n');

    // Send the sample rate value
    ps2_wait_write();
    outb(PS2_COMMAND_PORT, 0xD4); // Next byte goes to mouse
    ps2_wait_write();
    outb(PS2_DATA_PORT, MOUSE_SAMPLE_RATE); // Sample rate value
    ps2_wait_read();
    uint8_t rate_ack = inb(PS2_DATA_PORT); // Read ACK
    simple_serial_puts("DEBUG: Set sample rate to ");
    char rate_buf[16];
    snprintf(rate_buf, sizeof(rate_buf), "%d", MOUSE_SAMPLE_RATE);
    simple_serial_puts(rate_buf);
    simple_serial_puts(" Hz, ACK: 0x");
    char rate_ack_h = ((rate_ack >> 4) & 0xF);
    char rate_ack_l = (rate_ack & 0xF);
    simple_serial_putc(rate_ack_h < 10 ? '0' + rate_ack_h : 'A' + rate_ack_h - 10);
    simple_serial_putc(rate_ack_l < 10 ? '0' + rate_ack_l : 'A' + rate_ack_l - 10);
    simple_serial_putc('\n');

    // Send command to mouse: Enable streaming packets
    ps2_wait_write();
    outb(PS2_COMMAND_PORT, 0xD4); // Next byte goes to mouse
    ps2_wait_write();
    outb(PS2_DATA_PORT, 0xF4); // Enable streaming
    ps2_wait_read();
    uint8_t ack = inb(PS2_DATA_PORT); // Read ACK
    simple_serial_puts("DEBUG: Sent F4 command, ACK: 0x");
    char ack_h = ((ack >> 4) & 0xF);
    char ack_l = (ack & 0xF);
    simple_serial_putc(ack_h < 10 ? '0' + ack_h : 'A' + ack_h - 10);
    simple_serial_putc(ack_l < 10 ? '0' + ack_l : 'A' + ack_l - 10);
    simple_serial_putc('\n');

    // Register interrupt handler
    // IRQ 12 maps to interrupt 0x2C after PIC remapping (0x28 + 4)
    idt_register_interrupt_callback(0x2C, ps2_mouse_handle_interrupt);
    simple_serial_puts("DEBUG: Registered mouse interrupt handler at 0x2C\n");

    // Enable IRQ 12 (mouse) on the slave PIC
    // Read current mask, clear bit 4 (IRQ 12), write back
    uint8_t slave_mask = inb(0xA1);
    simple_serial_puts("DEBUG: Current slave PIC mask: 0x");
    char s1_h = ((slave_mask >> 4) & 0xF);
    char s1_l = (slave_mask & 0xF);
    simple_serial_putc(s1_h < 10 ? '0' + s1_h : 'A' + s1_h - 10);
    simple_serial_putc(s1_l < 10 ? '0' + s1_l : 'A' + s1_l - 10);
    simple_serial_puts("\n");
    
    slave_mask &= ~(1 << 4); // Clear bit 4 for IRQ 12
    outb(0xA1, slave_mask);
    simple_serial_puts("DEBUG: Enabled IRQ 12 on slave PIC\n");
    
    simple_serial_puts("DEBUG: Final slave PIC mask: 0x");
    slave_mask = inb(0xA1);
    char s2_h = ((slave_mask >> 4) & 0xF);
    char s2_l = (slave_mask & 0xF);
    simple_serial_putc(s2_h < 10 ? '0' + s2_h : 'A' + s2_h - 10);
    simple_serial_putc(s2_l < 10 ? '0' + s2_l : 'A' + s2_l - 10);
    simple_serial_puts("\n");

    // Enable IRQ 2 on master PIC (required for slave PIC interrupts)
    master_mask = inb(0x21);
    simple_serial_puts("DEBUG: Current master PIC mask: 0x");
    char m1_h = ((master_mask >> 4) & 0xF);
    char m1_l = (master_mask & 0xF);
    simple_serial_putc(m1_h < 10 ? '0' + m1_h : 'A' + m1_h - 10);
    simple_serial_putc(m1_l < 10 ? '0' + m1_l : 'A' + m1_l - 10);
    simple_serial_puts("\n");
    
    master_mask &= ~(1 << 2); // Clear bit 2 for IRQ 2 (slave PIC)
    outb(0x21, master_mask);
    simple_serial_puts("DEBUG: Enabled IRQ 2 on master PIC for slave interrupts\n");
    
    simple_serial_puts("DEBUG: Final master PIC mask: 0x");
    master_mask = inb(0x21);
    char m2_h = ((master_mask >> 4) & 0xF);
    char m2_l = (master_mask & 0xF);
    simple_serial_putc(m2_h < 10 ? '0' + m2_h : 'A' + m2_h - 10);
    simple_serial_putc(m2_l < 10 ? '0' + m2_l : 'A' + m2_l - 10);
    simple_serial_puts("\n");

    // Initialize mouse position to center of screen (with safety checks)
    int screen_width = gpu_screen_width();
    int screen_height = gpu_screen_height();
    
    if (screen_width > 0 && screen_height > 0)
    {
        mouse_x = screen_width / 2;
        mouse_y = screen_height / 2;
    }
    else
    {
        // Default position if screen not initialized yet
        mouse_x = 320;
        mouse_y = 240;
    }
    
    simple_serial_puts("DEBUG: PS/2 mouse initialization complete\n");
    return 0;
}

void ps2_mouse_handle_packet(struct mouse *mouse)
{
    mouse->x = mouse_x;
    mouse->y = mouse_y;
    mouse->left = mouse_left;
    mouse->right = mouse_right;
    mouse->middle = mouse_middle;
}

// Function to set mouse speed/sensitivity
// speed_multiplier: 1.0 = normal speed, 0.5 = half speed, 2.0 = double speed, etc.
void ps2_mouse_set_speed(float speed_multiplier)
{
    if (speed_multiplier <= 0.0f)
        return; // Invalid speed
        
    // Convert float to integer fraction for better precision
    if (speed_multiplier >= 1.0f)
    {
        mouse_sensitivity_numerator = (int)(speed_multiplier * 10);
        mouse_sensitivity_denominator = 10;
    }
    else
    {
        mouse_sensitivity_numerator = (int)(speed_multiplier * 100);
        mouse_sensitivity_denominator = 100;
    }
    
    // Simplify the fraction to avoid overflow
    int gcd_val = 1;
    int a = mouse_sensitivity_numerator;
    int b = mouse_sensitivity_denominator;
    while (b != 0)
    {
        int temp = b;
        b = a % b;
        a = temp;
    }
    gcd_val = a;
    
    mouse_sensitivity_numerator /= gcd_val;
    mouse_sensitivity_denominator /= gcd_val;
    
    simple_serial_puts("DEBUG: Mouse speed set to ");
    char buf[32];
    snprintf(buf, sizeof(buf), "%d/%d", mouse_sensitivity_numerator, mouse_sensitivity_denominator);
    simple_serial_puts(buf);
    simple_serial_puts("\n");
}

// Function to get current mouse position
void ps2_mouse_get_position(int *x, int *y)
{
    if (x) *x = mouse_x;
    if (y) *y = mouse_y;
}

// Function to set mouse position (for programmatic cursor movement)
void ps2_mouse_set_position(int x, int y)
{
    int screen_width = gpu_screen_width();
    int screen_height = gpu_screen_height();
    
    if (screen_width > 0 && screen_height > 0)
    {
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x >= screen_width) x = screen_width - 1;
        if (y >= screen_height) y = screen_height - 1;
    }
    
    mouse_x = x;
    mouse_y = y;
    
    // Update mouse struct and notify system
    ps2_mouse_handle_packet(&ps2_mouse);
    mouse_handle_interrupt();
}

struct mouse *ps2_mouse_init()
{
    ps2_mouse.init = ps2_mouse_init_driver;
    ps2_mouse.handle_packet = ps2_mouse_handle_packet;
    ps2_mouse.set_position = ps2_mouse_set_position;
    ps2_mouse.set_speed = ps2_mouse_set_speed;
    ps2_mouse.name[0] = 'P';
    ps2_mouse.name[1] = 'S';
    ps2_mouse.name[2] = '/';
    ps2_mouse.name[3] = '2';
    ps2_mouse.name[4] = '\0';

    return &ps2_mouse;
}