#ifndef VIOS_H
#define VIOS_H

#include <stddef.h>
#include <stdbool.h>

// Define types manually since stdint.h is not available
typedef unsigned int uint32_t;

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

// VIX Graphics API
typedef struct {
    int width;
    int height;
    int bpp;
    int refresh_rate;
} vix_screen_info_t;

// Color utility macros
#define VIX_RGB(r, g, b) (((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b))
#define VIX_COLOR_BLACK VIX_RGB(0, 0, 0)
#define VIX_COLOR_WHITE VIX_RGB(255, 255, 255)
#define VIX_COLOR_RED VIX_RGB(255, 0, 0)
#define VIX_COLOR_GREEN VIX_RGB(0, 255, 0)
#define VIX_COLOR_BLUE VIX_RGB(0, 0, 255)
#define VIX_COLOR_YELLOW VIX_RGB(255, 255, 0)
#define VIX_COLOR_CYAN VIX_RGB(0, 255, 255)
#define VIX_COLOR_MAGENTA VIX_RGB(255, 0, 255)

// VIX Graphics API functions
void vix_draw_pixel(int x, int y, uint32_t color);
void vix_draw_rect(int x, int y, int width, int height, uint32_t color);
void vix_fill_rect(int x, int y, int width, int height, uint32_t color);
void vix_clear_screen(uint32_t color);
void vix_present_frame(void);
void vix_get_screen_info(vix_screen_info_t *info);
void vix_draw_line(int x1, int y1, int x2, int y2, uint32_t color);
void vix_draw_circle(int x, int y, int radius, uint32_t color);
void vix_fill_circle(int x, int y, int radius, uint32_t color);

#endif
