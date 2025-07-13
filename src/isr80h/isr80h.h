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
    SYSTEM_COMMAND4_PUTCHAR,
    SYSTEM_COMMAND5_PRINT_SERIAL,
    SYSTEM_COMMAND6_PRINT_CHAR_SERIAL,

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
void *isr80h_command1_process_load_start(struct interrupt_frame *frame);

#endif