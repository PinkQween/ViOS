#ifndef VIOS_H
#define VIOS_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct command_argument {
  char argument[512];
  struct command_argument *next;
};

struct process_arguments {
  int argc;
  char **argv;
};

// System calls
void vios_print(const char *message);
int vios_getkey();
void *vios_malloc(size_t size);
void vios_free(void *ptr);
void *vios_realloc(void *old_ptr, size_t new_size);
void vios_putchar(char c);
int vios_getkeyblock();
void vios_terminal_readline(char *out, int max, bool output_while_typing);
void vios_process_load_start(const char *filename);
struct command_argument *vios_parse_command(const char *command, int max);
void vios_process_get_arguments(struct process_arguments *arguments);
int vios_system(struct command_argument *arguments);
int vios_system_run(const char *command);
void vios_exit();

// File operations
int vios_fopen(const char* filename, const char* mode);
void vios_fclose(size_t fd);
long vios_fread(void* buffer, size_t size, size_t count, long fd);
long vios_fwrite(const void* ptr, size_t size, size_t nmemb, long fd);
long vios_fseek(long fd, long offset, long whence);
long vios_fstat(long fd, void* file_stat_out);

#endif