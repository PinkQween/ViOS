#include "vix_graphics.h"
#include "isr80h.h"
#include "task/task.h"
#include "task/process.h"
#include "graphics/graphics.h"
#include "kernel.h"
#include "idt/idt.h"
#include <stdint.h>

/**
 * Draws a single pixel at the specified coordinates with the given RGB color using the VIX Graphics API.
 *
 * Extracts the x and y coordinates and a packed 24-bit RGB color from the interrupt frame registers,
 * converts the color to the internal format, and sets the pixel on the current graphics context's back buffer if available.
 */

void *isr80h_command11_vix_draw_pixel(struct interrupt_frame *frame)
{
    // Parameters: EBX = x, ECX = y, EDX = color (RGB packed)
    int x = (int)frame->ebx;
    int y = (int)frame->ecx;
    uint32_t rgb = (uint32_t)frame->edx;
    
    // Extract RGB components
    uint8_t r = (rgb >> 16) & 0xFF;
    uint8_t g = (rgb >> 8) & 0xFF;
    uint8_t b = rgb & 0xFF;
    
    // Convert to graphics color and draw
    Color color = graphics_rgb_to_color(r, g, b);
    GraphicsContext *ctx = graphics_get_context();
    if (ctx && ctx->back_buffer) {
        graphics_set_pixel(ctx->back_buffer, x, y, color);
    }
    
    return 0;
}

/**
 * Draws the outline of a rectangle at the specified coordinates with the given dimensions and RGB color.
 *
 * Extracts the rectangle's position, size, and color from the interrupt frame registers and draws the rectangle outline on the current graphics context's back buffer if available.
 */
void *isr80h_command12_vix_draw_rect(struct interrupt_frame *frame)
{
    // Parameters: EBX = x, ECX = y, EDX = width, ESI = height, EDI = color
    int x = (int)frame->ebx;
    int y = (int)frame->ecx;
    int width = (int)frame->edx;
    int height = (int)frame->esi;
    uint32_t rgb = (uint32_t)frame->edi;
    
    // Extract RGB components
    uint8_t r = (rgb >> 16) & 0xFF;
    uint8_t g = (rgb >> 8) & 0xFF;
    uint8_t b = rgb & 0xFF;
    
    // Convert to graphics color and draw
    Color color = graphics_rgb_to_color(r, g, b);
    GraphicsContext *ctx = graphics_get_context();
    if (ctx && ctx->back_buffer) {
        Rectangle rect = {x, y, width, height};
        graphics_draw_rect(ctx->back_buffer, rect, color);
    }
    
    return 0;
}

/**
 * Fills a rectangle at the specified coordinates with the given dimensions and RGB color on the current graphics context's back buffer.
 *
 * The rectangle's position, size, and color are extracted from the interrupt frame registers.
 * Does nothing if no valid graphics context or back buffer is available.
 * @returns Always returns 0.
 */
void *isr80h_command13_vix_fill_rect(struct interrupt_frame *frame)
{
    // Parameters: EBX = x, ECX = y, EDX = width, ESI = height, EDI = color
    int x = (int)frame->ebx;
    int y = (int)frame->ecx;
    int width = (int)frame->edx;
    int height = (int)frame->esi;
    uint32_t rgb = (uint32_t)frame->edi;
    
    // Extract RGB components
    uint8_t r = (rgb >> 16) & 0xFF;
    uint8_t g = (rgb >> 8) & 0xFF;
    uint8_t b = rgb & 0xFF;
    
    // Convert to graphics color and fill
    Color color = graphics_rgb_to_color(r, g, b);
    GraphicsContext *ctx = graphics_get_context();
    if (ctx && ctx->back_buffer) {
        Rectangle rect = {x, y, width, height};
        graphics_fill_rect(ctx->back_buffer, rect, color);
    }
    
    return 0;
}

/**
 * Clears the entire screen to the specified RGB color.
 *
 * The color is provided as a packed 32-bit RGB value in the EBX register of the interrupt frame.
 * The operation is performed on the current graphics context's back buffer if available.
 */
void *isr80h_command14_vix_clear_screen(struct interrupt_frame *frame)
{
    // Parameters: EBX = color (RGB packed)
    uint32_t rgb = (uint32_t)frame->ebx;
    
    // Extract RGB components
    uint8_t r = (rgb >> 16) & 0xFF;
    uint8_t g = (rgb >> 8) & 0xFF;
    uint8_t b = rgb & 0xFF;
    
    // Convert to graphics color and clear
    Color color = graphics_rgb_to_color(r, g, b);
    GraphicsContext *ctx = graphics_get_context();
    if (ctx && ctx->back_buffer) {
        graphics_clear_surface(ctx->back_buffer, color);
    }
    
    return 0;
}

/**
 * Presents the current graphics frame buffer to the display.
 *
 * This function triggers the graphics subsystem to display the contents of the back buffer, making all recent drawing operations visible on screen.
 * @returns Always returns 0.
 */
void *isr80h_command15_vix_present_frame(struct interrupt_frame *frame)
{
    // No parameters - just present the current frame
    graphics_present();
    return 0;
}

/**
 * Copies the current screen information to a user-provided structure in memory.
 *
 * Extracts a pointer to a `vix_screen_info` structure from the interrupt frame and populates it with the current graphics mode's width, height, bits per pixel, and refresh rate.
 * @returns Always returns 0.
 */
void *isr80h_command16_vix_get_screen_info(struct interrupt_frame *frame)
{
    // Parameters: EBX = pointer to screen_info structure
    struct vix_screen_info *info = (struct vix_screen_info *)frame->ebx;
    
    // Copy screen info to user space
    struct task *current = task_current();
    if (current) {
        GraphicsContext *ctx = graphics_get_context();
        if (ctx) {
            struct vix_screen_info kernel_info = {
                .width = ctx->current_mode.width,
                .height = ctx->current_mode.height,
                .bpp = ctx->current_mode.bpp,
                .refresh_rate = ctx->current_mode.refresh_rate
            };
            
            // Copy to user space (simplified - in real implementation would need proper user space copying)
            *info = kernel_info;
        }
    }
    
    return 0;
}

/**
 * Draws a line between two points with a specified RGB color using the VIX Graphics API.
 *
 * Extracts the start and end coordinates and the packed RGB color from the interrupt frame registers, converts the color, and draws the line on the current graphics context's back buffer if available.
 */
void *isr80h_command17_vix_draw_line(struct interrupt_frame *frame)
{
    // Parameters: EBX = x1, ECX = y1, EDX = x2, ESI = y2, EDI = color
    int x1 = (int)frame->ebx;
    int y1 = (int)frame->ecx;
    int x2 = (int)frame->edx;
    int y2 = (int)frame->esi;
    uint32_t rgb = (uint32_t)frame->edi;
    
    // Extract RGB components
    uint8_t r = (rgb >> 16) & 0xFF;
    uint8_t g = (rgb >> 8) & 0xFF;
    uint8_t b = rgb & 0xFF;
    
    // Convert to graphics color and draw
    Color color = graphics_rgb_to_color(r, g, b);
    GraphicsContext *ctx = graphics_get_context();
    if (ctx && ctx->back_buffer) {
        Point start = {x1, y1};
        Point end = {x2, y2};
        graphics_draw_line(ctx->back_buffer, start, end, color);
    }
    
    return 0;
}

/**
 * Draws the outline of a circle at the specified coordinates with the given radius and RGB color.
 *
 * Extracts the center coordinates, radius, and packed RGB color from the interrupt frame registers,
 * converts the color, and draws the circle outline on the current graphics context's back buffer if available.
 */
void *isr80h_command18_vix_draw_circle(struct interrupt_frame *frame)
{
    // Parameters: EBX = x, ECX = y, EDX = radius, ESI = color
    int x = (int)frame->ebx;
    int y = (int)frame->ecx;
    int radius = (int)frame->edx;
    uint32_t rgb = (uint32_t)frame->esi;
    
    // Extract RGB components
    uint8_t r = (rgb >> 16) & 0xFF;
    uint8_t g = (rgb >> 8) & 0xFF;
    uint8_t b = rgb & 0xFF;
    
    // Convert to graphics color and draw
    Color color = graphics_rgb_to_color(r, g, b);
    GraphicsContext *ctx = graphics_get_context();
    if (ctx && ctx->back_buffer) {
        Point center = {x, y};
        graphics_draw_circle(ctx->back_buffer, center, radius, color);
    }
    
    return 0;
}

/**
 * Fills a circle at the specified coordinates with the given radius and RGB color on the current graphics back buffer.
 *
 * The circle is centered at (x, y) with the specified radius, and the fill color is determined by the packed RGB value provided in the interrupt frame.
 */
void *isr80h_command19_vix_fill_circle(struct interrupt_frame *frame)
{
    // Parameters: EBX = x, ECX = y, EDX = radius, ESI = color
    int x = (int)frame->ebx;
    int y = (int)frame->ecx;
    int radius = (int)frame->edx;
    uint32_t rgb = (uint32_t)frame->esi;
    
    // Extract RGB components
    uint8_t r = (rgb >> 16) & 0xFF;
    uint8_t g = (rgb >> 8) & 0xFF;
    uint8_t b = rgb & 0xFF;
    
    // Convert to graphics color and fill
    Color color = graphics_rgb_to_color(r, g, b);
    GraphicsContext *ctx = graphics_get_context();
    if (ctx && ctx->back_buffer) {
        Point center = {x, y};
        graphics_fill_circle(ctx->back_buffer, center, radius, color);
    }
    
    return 0;
}
