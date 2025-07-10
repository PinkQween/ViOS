#ifndef VIOS_H
#define VIOS_H

#include <stddef.h>
#include <stdbool.h>

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

void vios_exit();
void vios_print(const char *str, int x, int y, int r, int g, int b, int scale);
int vios_getkey();
void vios_putchar(char c, int x, int y, int r, int g, int b, int scale);
void *vios_malloc(size_t size);
void vios_free(void *ptr);
int vios_getkeyblock();
void vios_terminal_readline(char *out, int max, bool output_while_typing);
void vios_process_load_start(const char *filename);
struct command_argument *vios_parse_command(const char *command, int max);
void vios_process_get_arguments(struct process_arguments *arguments);
int vios_system(struct command_argument *arguments);
int vios_system_run(const char *command);
void vios_sleep(int seconds);
char *vios_read(const char *filename);

#endif