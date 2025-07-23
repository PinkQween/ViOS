#include "isr80h.h"

void isr80h_register_commands()
{
    // Process Management
    isr80h_register_command(SYSTEM_COMMAND_EXIT, isr80h_command_exit);
    isr80h_register_command(SYSTEM_COMMAND_PROCESS_LOAD_START, isr80h_command_process_load_start);
    isr80h_register_command(SYSTEM_COMMAND_INVOKE_SYSTEM_COMMAND, isr80h_command_invoke_system_command);

    // I/O Operations
    isr80h_register_command(SYSTEM_COMMAND_GETKEY, isr80h_command_getkey);
    isr80h_register_command(SYSTEM_COMMAND_PUTCHAR_SERIAL, isr80h_command_putchar_serial);
    isr80h_register_command(SYSTEM_COMMAND_PRINT_SERIAL, isr80h_command_print_serial);

    // Memory Management
    isr80h_register_command(SYSTEM_COMMAND_MALLOC, isr80h_command_malloc);
    isr80h_register_command(SYSTEM_COMMAND_FREE, isr80h_command_free);

    // File Operations
    isr80h_register_command(SYSTEM_COMMAND_READ, isr80h_command_read);

    // System Utilities
    isr80h_register_command(SYSTEM_COMMAND_SLEEP, isr80h_command_sleep);
    isr80h_register_command(SYSTEM_COMMAND_GET_PROGRAM_ARGUMENTS, isr80h_command_get_program_arguments);
    isr80h_register_command(SYSTEM_COMMAND_USLEEP, isr80h_command_usleep);
    isr80h_register_command(SYSTEM_COMMAND_NANOSLEEP, isr80h_command_nanosleep);

    // Graphics
    isr80h_register_command(SYSTEM_COMMAND_FLUSH_SCREEN, isr80h_command_flush);
    isr80h_register_command(SYSTEM_COMMAND_DRAW_PIXEL, isr80h_command_draw_pixel);

    // Audio
    isr80h_register_command(SYSTEM_COMMAND_AUDIO, isr80h_command_audio);
}