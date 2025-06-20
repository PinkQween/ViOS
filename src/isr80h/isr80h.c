#include "isr80h.h"
#include "io.h"
#include "idt/idt.h"
#include "heap.h"
#include "kernel.h"
#include "process.h"
#include "file.h"

void isr80h_register_commands()
{
    isr80h_register_command(SYSTEM_COMMAND0_EXIT, isr80h_command0_exit);
    isr80h_register_command(SYSTEM_COMMAND1_PRINT, isr80h_command1_print);
    isr80h_register_command(SYSTEM_COMMAND2_GETKEY, isr80h_command2_getkey);
    isr80h_register_command(SYSTEM_COMMAND3_PUTCHAR, isr80h_command3_putchar);
    isr80h_register_command(SYSTEM_COMMAND4_MALLOC, isr80h_command4_malloc);
    isr80h_register_command(SYSTEM_COMMAND5_FREE, isr80h_command5_free);
    isr80h_register_command(SYSTEM_COMMAND6_PROCESS_LOAD_START, isr80h_command6_process_load_start);
    isr80h_register_command(SYSTEM_COMMAND7_INVOKE_SYSTEM_COMMAND, isr80h_command7_invoke_system_command);
    isr80h_register_command(SYSTEM_COMMAND8_GET_PROGRAM_ARGUMENTS, isr80h_command8_get_program_arguments);
    isr80h_register_command(SYSTEM_COMMAND9_SLEEP, isr80h_command9_sleep);
    isr80h_register_command(SYSTEM_COMMAND10_READ, isr80h_command10_read);
}