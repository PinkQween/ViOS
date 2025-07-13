#include "isr80h.h"
#include "idt/idt.h"
#include "heap.h"
#include "kernel.h"
#include "process.h"
#include "file.h"
#include "keyboard.h"
#include "serial.h"
#include "waits.h"

/**
 * Registers all supported system commands and their handlers for interrupt 0x80h.
 *
 * This function binds each system command identifier to its corresponding handler function,
 * enabling the kernel to dispatch system calls and graphics operations through the interrupt 0x80h interface.
 */
void isr80h_register_commands()
{
    // Process Management
    isr80h_register_command(SYSTEM_COMMAND0_EXIT, isr80h_command0_exit);
    isr80h_register_command(SYSTEM_COMMAND1_PROCESS_LOAD_START, isr80h_command1_process_load_start);
    isr80h_register_command(SYSTEM_COMMAND2_INVOKE_SYSTEM_COMMAND, isr80h_command2_invoke_system_command);

    // I/O Operations
    isr80h_register_command(SYSTEM_COMMAND3_GETKEY, isr80h_command3_getkey);
    isr80h_register_command(SYSTEM_COMMAND4_PUTCHAR_SERIAL, isr80h_command4_putchar_serial);
    isr80h_register_command(SYSTEM_COMMAND5_PRINT_SERIAL, isr80h_command5_print_serial);

    // Memory Management
    isr80h_register_command(SYSTEM_COMMAND7_MALLOC, isr80h_command7_malloc);
    isr80h_register_command(SYSTEM_COMMAND8_FREE, isr80h_command8_free);

    // File Operations
    isr80h_register_command(SYSTEM_COMMAND9_READ, isr80h_command9_read);

    // System Utilities
    isr80h_register_command(SYSTEM_COMMAND10_SLEEP, isr80h_command10_sleep);
    isr80h_register_command(SYSTEM_COMMAND11_GET_PROGRAM_ARGUMENTS, isr80h_command11_get_program_arguments);
}
