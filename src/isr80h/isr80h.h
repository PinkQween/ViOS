#ifndef ISR80H_H
#define ISR80H_H

struct interrupt_frame;

enum SystemCommands
{
    // Process Management (0-2)
    SYSTEM_COMMAND0_EXIT,
    SYSTEM_COMMAND1_PROCESS_LOAD_START,
    SYSTEM_COMMAND2_INVOKE_SYSTEM_COMMAND,

    // I/O Operations (3-6)
    SYSTEM_COMMAND3_GETKEY,
    SYSTEM_COMMAND4_PUTCHAR_SERIAL,
    SYSTEM_COMMAND5_PRINT_SERIAL,
    BLANK_COMMAND6,

    // Memory Management (7-8)
    SYSTEM_COMMAND7_MALLOC,
    SYSTEM_COMMAND8_FREE,

    // File Operations (9)
    SYSTEM_COMMAND9_READ,

    // System Utilities (10-11)
    SYSTEM_COMMAND10_SLEEP,
    SYSTEM_COMMAND11_GET_PROGRAM_ARGUMENTS,
};

void isr80h_register_commands();

// Process Management
void *isr80h_command0_exit(struct interrupt_frame *frame);
void *isr80h_command1_process_load_start(struct interrupt_frame *frame);
void *isr80h_command2_invoke_system_command(struct interrupt_frame *frame);

// I/O Operations
void *isr80h_command3_getkey(struct interrupt_frame *frame);
void *isr80h_command4_putchar_serial(struct interrupt_frame *frame);
void *isr80h_command5_print_serial(struct interrupt_frame *frame);

// Memory Management
void *isr80h_command7_malloc(struct interrupt_frame *frame);
void *isr80h_command8_free(struct interrupt_frame *frame);

// File Operations
void *isr80h_command9_read(struct interrupt_frame *frame);

// System Utilities
void *isr80h_command10_sleep(struct interrupt_frame *frame);
void *isr80h_command11_get_program_arguments(struct interrupt_frame *frame);

#endif