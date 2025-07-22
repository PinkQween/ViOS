#ifndef VIGFX_H
#define VIGFX_H

#include <stdint.h>

struct graphics_device
{
    void (*draw_pixel)(int x, int y, int r, int g, int b);
    void (*clear)(int r, int g, int b);
    void (*flush)(void);
    uint32_t (*get_pixel)(int x, int y);

    int width;
    int height;
};

void graphics_init(void);

void gpu_draw(int x, int y, int r, int g, int b);
void gpu_clear_screen(int r, int g, int b);
void gpu_flush_screen(void);
uint32_t gpu_get_pixel(int x, int y);

// Access to current screen size
int gpu_screen_width(void);
int gpu_screen_height(void);

#endif
