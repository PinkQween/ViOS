#include "isr80h.h"

void isr80h_register_commands()
{
    simple_serial_puts("DEBUG: Entering isr80h_register_commands\n");
    // Process Management
    isr80h_register_command(SYSTEM_COMMAND_EXIT, isr80h_command0_exit);
    simple_serial_puts("DEBUG: Registering command SYSTEM_COMMAND_EXIT\n");
    isr80h_register_command(SYSTEM_COMMAND_PROCESS_LOAD_START, isr80h_command1_process_load_start);
    simple_serial_puts("DEBUG: Registering command SYSTEM_COMMAND_PROCESS_LOAD_START\n");
    isr80h_register_command(SYSTEM_COMMAND_INVOKE_SYSTEM_COMMAND, isr80h_command2_invoke_system_command);
    simple_serial_puts("DEBUG: Registering command SYSTEM_COMMAND_INVOKE_SYSTEM_COMMAND\n");

    // I/O Operations
    isr80h_register_command(SYSTEM_COMMAND_GETKEY, isr80h_command3_getkey);
    simple_serial_puts("DEBUG: Registering command SYSTEM_COMMAND_GETKEY\n");
    isr80h_register_command(SYSTEM_COMMAND_PUTCHAR_SERIAL, isr80h_command4_putchar_serial);
    simple_serial_puts("DEBUG: Registering command SYSTEM_COMMAND_PUTCHAR_SERIAL\n");
    isr80h_register_command(SYSTEM_COMMAND_PRINT_SERIAL, isr80h_command5_print_serial);
    simple_serial_puts("DEBUG: Registering command SYSTEM_COMMAND_PRINT_SERIAL\n");

    // Memory Management
    isr80h_register_command(SYSTEM_COMMAND_MALLOC, isr80h_command7_malloc);
    simple_serial_puts("DEBUG: Registering command SYSTEM_COMMAND_MALLOC\n");
    isr80h_register_command(SYSTEM_COMMAND_FREE, isr80h_command8_free);
    simple_serial_puts("DEBUG: Registering command SYSTEM_COMMAND_FREE\n");

    // File Operations
    isr80h_register_command(SYSTEM_COMMAND_READ, isr80h_command9_read);
    simple_serial_puts("DEBUG: Registering command SYSTEM_COMMAND_READ\n");

    // System Utilities
    isr80h_register_command(SYSTEM_COMMAND_SLEEP, isr80h_command10_sleep);
    simple_serial_puts("DEBUG: Registering command SYSTEM_COMMAND_SLEEP\n");
    isr80h_register_command(SYSTEM_COMMAND_GET_PROGRAM_ARGUMENTS, isr80h_command11_get_program_arguments);
    simple_serial_puts("DEBUG: Registering command SYSTEM_COMMAND_GET_PROGRAM_ARGUMENTS\n");
    isr80h_register_command(SYSTEM_COMMAND_USLEEP, isr80h_command12_usleep);
    simple_serial_puts("DEBUG: Registering command SYSTEM_COMMAND_USLEEP\n");
    isr80h_register_command(SYSTEM_COMMAND_NANOSLEEP, isr80h_command13_nanosleep);
    simple_serial_puts("DEBUG: Registering command SYSTEM_COMMAND_NANOSLEEP\n");

    // Audio
    isr80h_register_command(SYSTEM_COMMAND_AUDIO, isr80h_command_audio);
    simple_serial_puts("DEBUG: Registering command SYSTEM_COMMAND_AUDIO\n");

    simple_serial_puts("DEBUG: ISR80H commands registered\n");
}