#ifndef VIOS_SYSCALL_H
#define VIOS_SYSCALL_H

#include <stdint.h>

// Forward declarations for structures defined in ViOS.h
struct process_arguments;

#ifdef __cplusplus
extern "C"
{
#endif

    // System command definitions
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

    // Process Management
    void vios_sys_exit(int code);
    int vios_sys_process_load_start(const char *filename);
    int vios_sys_invoke_system_command(int command, void *args);

    // I/O Operations
    int vios_sys_getkey(void);
    void vios_sys_putchar(char c);
    void vios_sys_serial_print(const char *str);

    // Memory Management
    void *vios_sys_malloc(uint32_t size);
    void vios_sys_free(void *ptr);

    // File Operations
    int vios_sys_read(const char *filename, void *buffer, uint32_t size);

    // System Utilities
    void vios_sys_sleep(uint32_t milliseconds);
    void vios_sys_get_program_arguments(struct process_arguments *arguments);

#ifdef __cplusplus
}
#endif

#endif
