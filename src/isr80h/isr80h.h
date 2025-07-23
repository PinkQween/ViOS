#ifndef ISR80H_H
#define ISR80H_H

#include "idt/idt.h"
#include "heap.h"
#include "kernel.h"
#include "process.h"
#include "file.h"
#include "keyboard.h"
#include "serial.h"
#include "timers.h"
#include "audio.h"
#include "graphics.h"

struct interrupt_frame;

enum SystemCommands
{
    // Process Management (0-2)
    SYSTEM_COMMAND_EXIT = 0,
    SYSTEM_COMMAND_PROCESS_LOAD_START,
    SYSTEM_COMMAND_INVOKE_SYSTEM_COMMAND,

    // I/O Operations (3-5)
    SYSTEM_COMMAND_GETKEY,
    SYSTEM_COMMAND_PUTCHAR_SERIAL,
    SYSTEM_COMMAND_PRINT_SERIAL,

    // Memory Management (7-8)
    SYSTEM_COMMAND_MALLOC = 7,
    SYSTEM_COMMAND_FREE,

    // File Operations (9-10)
    SYSTEM_COMMAND_READ,
    SYSTEM_COMMAND_WRITE,

    // System Utilities (11-12)
    SYSTEM_COMMAND_SLEEP,
    SYSTEM_COMMAND_GET_PROGRAM_ARGUMENTS,

    // Time/Delay Functions (13-14)
    SYSTEM_COMMAND_USLEEP,
    SYSTEM_COMMAND_NANOSLEEP,

    // Reserved: Signal Handling (20-29)
    SYSTEM_COMMAND_SIGNAL_RESERVED0 = 20,
    SYSTEM_COMMAND_SIGNAL_RESERVED1,
    SYSTEM_COMMAND_SIGNAL_RESERVED2,
    SYSTEM_COMMAND_SIGNAL_RESERVED3,
    SYSTEM_COMMAND_SIGNAL_RESERVED4,
    SYSTEM_COMMAND_SIGNAL_RESERVED5,
    SYSTEM_COMMAND_SIGNAL_RESERVED6,
    SYSTEM_COMMAND_SIGNAL_RESERVED7,
    SYSTEM_COMMAND_SIGNAL_RESERVED8,
    SYSTEM_COMMAND_SIGNAL_RESERVED9,

    // Reserved: Networking (30-39)
    SYSTEM_COMMAND_NETWORK_RESERVED0 = 30,
    SYSTEM_COMMAND_NETWORK_RESERVED1,
    SYSTEM_COMMAND_NETWORK_RESERVED2,
    SYSTEM_COMMAND_NETWORK_RESERVED3,
    SYSTEM_COMMAND_NETWORK_RESERVED4,
    SYSTEM_COMMAND_NETWORK_RESERVED5,
    SYSTEM_COMMAND_NETWORK_RESERVED6,
    SYSTEM_COMMAND_NETWORK_RESERVED7,
    SYSTEM_COMMAND_NETWORK_RESERVED8,
    SYSTEM_COMMAND_NETWORK_RESERVED9,

    // Graphics Commands (40-49)
    SYSTEM_COMMAND_FLUSH_SCREEN = 40,
    SYSTEM_COMMAND_DRAW_PIXEL,
    SYSTEM_COMMAND_GRAPHICS_RESERVED2,
    SYSTEM_COMMAND_GRAPHICS_RESERVED3,
    SYSTEM_COMMAND_GRAPHICS_RESERVED4,
    SYSTEM_COMMAND_GRAPHICS_RESERVED5,
    SYSTEM_COMMAND_GRAPHICS_RESERVED6,
    SYSTEM_COMMAND_GRAPHICS_RESERVED7,
    SYSTEM_COMMAND_GRAPHICS_RESERVED8,
    SYSTEM_COMMAND_GRAPHICS_RESERVED9,

    // Audio (50)
    SYSTEM_COMMAND_AUDIO = 50,
};

void isr80h_register_commands();

// Process Management
void *isr80h_command_exit(struct interrupt_frame *frame);
void *isr80h_command_process_load_start(struct interrupt_frame *frame);
void *isr80h_command_invoke_system_command(struct interrupt_frame *frame);

// I/O Operations
void *isr80h_command_getkey(struct interrupt_frame *frame);
void *isr80h_command_putchar_serial(struct interrupt_frame *frame);
void *isr80h_command_print_serial(struct interrupt_frame *frame);

// Memory Management
void *isr80h_command_malloc(struct interrupt_frame *frame);
void *isr80h_command_free(struct interrupt_frame *frame);

// File Operations
void *isr80h_command_read(struct interrupt_frame *frame);

// System Utilities
void *isr80h_command_sleep(struct interrupt_frame *frame);
void *isr80h_command_get_program_arguments(struct interrupt_frame *frame);
// Time/Delay Functions
void *isr80h_command_usleep(struct interrupt_frame *frame);
void *isr80h_command_nanosleep(struct interrupt_frame *frame);

// Graphics Commands
void *isr80h_command_flush(struct interrupt_frame *frame);
void *isr80h_command_draw_pixel(struct interrupt_frame *frame);

// Audio
void *isr80h_command_audio(struct interrupt_frame *frame);

#endif