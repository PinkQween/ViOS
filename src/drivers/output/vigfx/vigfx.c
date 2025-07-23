#include <stdint.h>
#include "vigfx.h"
#include "vesa.h"
#include "debug/simple_serial.h"
#include "utils/utils.h"

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

uint32_t gpu_get_pixel(int x, int y) {
    if (active_gpu && active_gpu->get_pixel) {
        return active_gpu->get_pixel(x, y);
    }
    return 0;
}

void gpu_draw_alpha(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    // Clamp alpha to 0-255
    if (a == 255)
    {
        gpu_draw(x, y, r, g, b);
        return;
    }
    if (a == 0)
        return;

    uint32_t bg_pixel = gpu_get_pixel(x, y);
    uint8_t bg_r = (bg_pixel >> 16) & 0xFF;
    uint8_t bg_g = (bg_pixel >> 8) & 0xFF;
    uint8_t bg_b = bg_pixel & 0xFF;

    // Alpha blending: out = fg * alpha + bg * (1 - alpha)
    uint8_t out_r = (r * a + bg_r * (255 - a)) / 255;
    uint8_t out_g = (g * a + bg_g * (255 - a)) / 255;
    uint8_t out_b = (b * a + bg_b * (255 - a)) / 255;

    gpu_draw(x, y, out_r, out_g, out_b);
}