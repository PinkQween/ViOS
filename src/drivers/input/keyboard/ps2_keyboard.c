#include "ps2_keyboard.h"
#include "keyboard.h"
#include "drivers/io/io.h"
#include "kernel.h"
#include "idt/idt.h"
#include "task/task.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PS2_KEYBOARD_CAPS_LOCK 0x3A
#define PS2_KEYBOARD_SHIFT_KEYS {0x2A, 0x36}

static const uint8_t shift_keys[] = PS2_KEYBOARD_SHIFT_KEYS;
static const size_t shift_keys_count = sizeof(shift_keys) / sizeof(shift_keys[0]);

static bool shift_down = false;

static uint8_t keyboard_scan_set_one[] = {
    0x00, 0x1B, '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0', '-', '=',
    0x08, '\t', 'Q', 'W', 'E', 'R', 'T',
    'Y', 'U', 'I', 'O', 'P', '[', ']',
    0x0D, 0x00, 'A', 'S', 'D', 'F', 'G',
    'H', 'J', 'K', 'L', ';', '\'', '`',
    0x00, '\\', 'Z', 'X', 'C', 'V', 'B',
    'N', 'M', ',', '.', '/', 0x00, '*',
    0x00, 0x20, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, '7', '8', '9', '-', '4', '5',
    '6', '+', '1', '2', '3', '0', '.'};

int PS2_keyboard_init();
uint8_t PS2_keyboard_scancode_to_char(uint8_t scancode);
void PS2_keyboard_handle_interrupt();

struct keyboard PS2_keyboard = {
    .name = {"PS2"},
    .init = PS2_keyboard_init};

void set_keyboard_leds(bool caps, bool num, bool scroll)
{
    uint8_t led_cmd = 0xED;
    uint8_t led_state = (scroll ? 1 : 0) | (num ? 2 : 0) | (caps ? 4 : 0);
    int timeout = 1000; // Reasonable timeout value

    // Send the command
    outb(0x60, led_cmd);
    // Wait for ACK (0xFA)
    uint8_t ack;
    while (timeout-- > 0)
    {
        insb(0x60, &ack, 1);
        if (ack == 0xFA)
            break;
    }
    if (timeout <= 0)
        return; // Timeout occurred

    // Send LED state
    outb(0x60, led_state);
    timeout = 1000;
    while (timeout-- > 0)
    {
        insb(0x60, &ack, 1);
        if (ack == 0xFA)
            break;
    }
}

int PS2_keyboard_init()
{
    idt_register_interrupt_callback(ISR_KEYBOARD_INTERRUPT, PS2_keyboard_handle_interrupt);
    keyboard_set_caps_lock(&PS2_keyboard, KEYBOARD_CAPS_LOCK_OFF);
    outb(PS2_PORT, PS2_COMMAND_ENABLE_FIRST_PORT);
    set_keyboard_leds(false, false, false);
    keyboard_set_caps_lock(&PS2_keyboard, KEYBOARD_CAPS_LOCK_OFF);

    // Enable IRQ 1 (keyboard) and IRQ 2 (cascade to slave PIC) on the master PIC
    uint8_t mask;
    insb(0x21, &mask, 1);
    mask &= ~(1 << 1); // Clear bit 1 for IRQ 1 (keyboard)
    mask &= ~(1 << 2); // Clear bit 2 for IRQ 2 (cascade to slave PIC)
    outb(0x21, mask);
    return 0;
}

uint8_t PS2_keyboard_scancode_to_char(uint8_t scancode)
{
    size_t max = sizeof(keyboard_scan_set_one) / sizeof(uint8_t);
    if (scancode >= max)
        return 0;

    char c = keyboard_scan_set_one[scancode];

    // Apply lowercase if caps lock is off and shift is not held
    bool caps = keyboard_get_caps_lock(&PS2_keyboard) == KEYBOARD_CAPS_LOCK_ON;
    if ((c >= 'A' && c <= 'Z') && (caps != shift_down))
    {
        c += 32; // Convert to lowercase
    }
    return c;
}

void PS2_keyboard_handle_interrupt()
{
    uint8_t scancode;
    insb(KEYBOARD_INPUT_PORT, &scancode, 1);
    uint8_t discard;
    insb(KEYBOARD_INPUT_PORT, &discard, 1); // discard extra byte if needed

    // Handle release
    if (scancode & PS2_KEYBOARD_KEY_RELEASED)
    {
        scancode &= ~PS2_KEYBOARD_KEY_RELEASED;
        // Check if the released key was a shift key
        for (size_t i = 0; i < shift_keys_count; ++i)
        {
            if (shift_keys[i] == scancode)
            {
                shift_down = false;
            }
        }
        return;
    }

    if (scancode == PS2_KEYBOARD_CAPS_LOCK)
    {
        KEYBOARD_CAPS_LOCK_STATE state = keyboard_get_caps_lock(&PS2_keyboard);
        keyboard_set_caps_lock(&PS2_keyboard, state == KEYBOARD_CAPS_LOCK_ON ? KEYBOARD_CAPS_LOCK_OFF : KEYBOARD_CAPS_LOCK_ON);
    }

    for (size_t i = 0; i < shift_keys_count; ++i)
    {
        if (shift_keys[i] == scancode)
        {
            shift_down = true;
        }
    }

    uint8_t c = PS2_keyboard_scancode_to_char(scancode);
    if (c != 0)
    {
        keyboard_push(c);
    }
}

struct keyboard *PS2_init()
{
    return &PS2_keyboard;
}