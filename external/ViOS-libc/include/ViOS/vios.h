#ifndef VIOS_H
#define VIOS_H

#include "syscall.h"

struct command_argument
{
    char argument[512];
    struct command_argument *next;
};

struct process_arguments
{
    int argc;
    char **argv;
};

// ViOS-specific constants and definitions
#define VIOS_VERSION "0.0.1"

// Error codes
#define VIOS_SUCCESS 0
#define VIOS_ERROR -1
#define VIOS_ERROR_INVALID_PARAMETER -2
#define VIOS_ERROR_NOT_FOUND -3
#define VIOS_ERROR_PERMISSION_DENIED -4
#define VIOS_ERROR_OUT_OF_MEMORY -5

// File operation flags
#define VIOS_O_RDONLY 0x0000
#define VIOS_O_WRONLY 0x0001
#define VIOS_O_RDWR 0x0002
#define VIOS_O_CREAT 0x0100
#define VIOS_O_TRUNC 0x0200
#define VIOS_O_APPEND 0x0400

// Process states
#define VIOS_PROCESS_STATE_READY 0
#define VIOS_PROCESS_STATE_RUNNING 1
#define VIOS_PROCESS_STATE_BLOCKED 2
#define VIOS_PROCESS_STATE_TERMINATED 3

// System limits
#define VIOS_MAX_PATH_LENGTH 256
#define VIOS_MAX_PROCESSES 64
#define VIOS_MAX_OPEN_FILES 128

#ifdef __cplusplus
extern "C"
{
#endif

    // ViOS-specific utility functions
    const char *vios_get_version(void);
    int vios_get_error_code(void);
    void vios_clear_error(void);

    // C startup function
    void c_start(void);

#ifdef __cplusplus
}
#endif

#endif