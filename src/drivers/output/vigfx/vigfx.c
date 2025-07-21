#include "vigfx.h"
#include "vesa.h"

static struct graphics_device *active_gpu = 0;

void graphics_init()
{
    active_gpu = vesa_init();
}

void gpu_draw(int x, int y, int r, int g, int b)
{
    if (active_gpu && active_gpu->draw_pixel)
        active_gpu->draw_pixel(x, y, r, g, b);
}

void gpu_clear_screen(int r, int g, int b)
{
    if (active_gpu && active_gpu->clear)
        active_gpu->clear(r, g, b);
}

void gpu_flush_screen()
{
    if (active_gpu && active_gpu->flush)
        active_gpu->flush();
}

int gpu_screen_width(void)
{
    return active_gpu ? active_gpu->width : 0;
}

int gpu_screen_height(void)
{
    return active_gpu ? active_gpu->height : 0;
}
