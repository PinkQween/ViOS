#ifndef VIX_GRAPHICS_H
#define VIX_GRAPHICS_H

#include <stdint.h>

struct interrupt_frame;

// VIX Graphics API structures
struct vix_screen_info {
    int width;
    int height;
    int bpp;
    int refresh_rate;
};

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

// VIX Graphics API system call handlers
void *isr80h_command11_vix_draw_pixel(struct interrupt_frame *frame);
void *isr80h_command12_vix_draw_rect(struct interrupt_frame *frame);
void *isr80h_command13_vix_fill_rect(struct interrupt_frame *frame);
void *isr80h_command14_vix_clear_screen(struct interrupt_frame *frame);
void *isr80h_command15_vix_present_frame(struct interrupt_frame *frame);
void *isr80h_command16_vix_get_screen_info(struct interrupt_frame *frame);
void *isr80h_command17_vix_draw_line(struct interrupt_frame *frame);
void *isr80h_command18_vix_draw_circle(struct interrupt_frame *frame);
void *isr80h_command19_vix_fill_circle(struct interrupt_frame *frame);

#endif // VIX_GRAPHICS_H
