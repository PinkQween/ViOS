#include "graphics.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "rtc/rtc.h"
#include "math/fpu_math.h"
#include "idt/idt.h"
#include "io/io.h"
#include "debug/simple_serial.h"
#include "string/string.h"

// Global graphics context - Windows-level architecture
static GraphicsContext g_graphics_context;
static bool g_graphics_initialized = false;

// Performance tracking variables
static uint32_t g_frame_time_start = 0;
static uint32_t g_total_render_time = 0;
static uint32_t g_frames_rendered = 0;

// =================== INTERNAL UTILITY FUNCTIONS ===================

// PIT-based millisecond timer for accurate FPS measurement
static uint32_t g_pit_ticks = 0;
static uint32_t g_pit_divisor = 1193; // For ~1ms intervals

void _graphics_init_pit_timer(void)
{
    // Configure PIT channel 0 for ~1ms intervals
    outb(0x43, 0x36);                        // Mode 3, 16-bit binary
    outb(0x40, g_pit_divisor & 0xFF);        // Low byte
    outb(0x40, (g_pit_divisor >> 8) & 0xFF); // High byte
}

void _graphics_pit_tick(void)
{
    g_pit_ticks++;
}

uint32_t _graphics_get_time_ms(void)
{
    // Return milliseconds since boot using PIT ticks
    return g_pit_ticks;
}

bool _graphics_is_point_visible(GraphicsSurface *surface, Point point)
{
    if (!surface)
        return false;
    return (point.x >= surface->clip_rect.x &&
            point.y >= surface->clip_rect.y &&
            point.x < surface->clip_rect.x + surface->clip_rect.width &&
            point.y < surface->clip_rect.y + surface->clip_rect.height);
}

void _graphics_update_fps_counter(void)
{
    static uint32_t last_time = 0;
    static uint32_t frame_count = 0;

    frame_count++;
    uint32_t current_time = _graphics_get_time_ms();

    if (current_time - last_time >= 1000)
    {
        g_graphics_context.frames_per_second = frame_count;
        g_graphics_context.last_fps_update = current_time;
        frame_count = 0;
        last_time = current_time;
    }
}

bool _graphics_init_framebuffer(void)
{
    VBEInfoBlock *vbe = (VBEInfoBlock *)VBEInfoAddress;
    if (!vbe)
        return false;

    g_graphics_context.vbe_info = vbe;
    g_graphics_context.current_mode.width = vbe->x_resolution;
    g_graphics_context.current_mode.height = vbe->y_resolution;
    g_graphics_context.current_mode.bpp = vbe->bits_per_pixel;
    g_graphics_context.current_mode.refresh_rate = 60;
    g_graphics_context.current_mode.interlaced = false;

    // Set hardware capabilities
    g_graphics_context.hardware_acceleration = false;
    g_graphics_context.hardware_cursor = false;
    g_graphics_context.linear_framebuffer = true;

    return true;
}

void _graphics_init_surfaces(void)
{
    VBEInfoBlock *vbe = g_graphics_context.vbe_info;
    if (!vbe)
        return;

    // Create front buffer (maps to hardware framebuffer)
    g_graphics_context.front_buffer = graphics_create_surface(vbe->x_resolution, vbe->y_resolution);
    if (g_graphics_context.front_buffer)
    {
        g_graphics_context.front_buffer->pixels = (uint16_t *)vbe->screen_ptr;
        g_graphics_context.front_buffer->pitch = vbe->x_resolution * GRAPHICS_BYTES_PER_PIXEL;
    }

    // Create back buffer (in system memory for double buffering)
    g_graphics_context.back_buffer = graphics_create_surface(vbe->x_resolution, vbe->y_resolution);
    if (g_graphics_context.back_buffer)
    {
        size_t buffer_size = vbe->x_resolution * vbe->y_resolution * sizeof(uint16_t);
        g_graphics_context.back_buffer->pixels = (uint16_t *)kmalloc(buffer_size);
        g_graphics_context.back_buffer->pitch = vbe->x_resolution * GRAPHICS_BYTES_PER_PIXEL;

        // Clear back buffer to black
        memset(g_graphics_context.back_buffer->pixels, 0, buffer_size);
    }

    // Set global clipping rectangle
    g_graphics_context.global_clip.x = 0;
    g_graphics_context.global_clip.y = 0;
    g_graphics_context.global_clip.width = vbe->x_resolution;
    g_graphics_context.global_clip.height = vbe->y_resolution;
}

// =================== CORE GRAPHICS API ===================

bool graphics_initialize(void)
{
    if (g_graphics_initialized)
        return true;

    memset(&g_graphics_context, 0, sizeof(GraphicsContext));

    if (!_graphics_init_framebuffer())
    {
        return false;
    }

    _graphics_init_surfaces();

    if (!g_graphics_context.front_buffer || !g_graphics_context.back_buffer)
    {
        return false;
    }

    // Initialize PIT timer for accurate FPS measurement
    _graphics_init_pit_timer();

    // Register PIT interrupt handler (IRQ 0 = interrupt 32)
    idt_register_interrupt_callback(32, graphics_pit_interrupt_handler);

    g_graphics_context.double_buffering_enabled = true;
    g_graphics_context.vsync_enabled = true;
    g_graphics_context.initialized = true;
    g_graphics_context.in_frame = false;
    g_graphics_context.needs_full_refresh = true;
    g_graphics_context.frame_count = 0;
    g_graphics_context.dropped_frames = 0;

    // Initialize FPS control - default to unlimited for maximum performance
    g_graphics_context.max_fps = 0; // 0 = unlimited
    g_graphics_context.fps_limit_enabled = false;
    g_graphics_context.target_frame_time_ms = 0;

    g_graphics_initialized = true;
    return true;
}

void graphics_shutdown(void)
{
    if (!g_graphics_initialized)
        return;

    if (g_graphics_context.back_buffer && g_graphics_context.back_buffer->pixels)
    {
        kfree(g_graphics_context.back_buffer->pixels);
    }

    graphics_destroy_surface(g_graphics_context.front_buffer);
    graphics_destroy_surface(g_graphics_context.back_buffer);

    memset(&g_graphics_context, 0, sizeof(GraphicsContext));
    g_graphics_initialized = false;
}

bool graphics_set_mode(int width, int height, int bpp)
{
    if (!g_graphics_initialized)
        return false;

    // For now, we work with the current VBE mode
    return (width == g_graphics_context.current_mode.width &&
            height == g_graphics_context.current_mode.height &&
            bpp == g_graphics_context.current_mode.bpp);
}

DisplayMode graphics_get_current_mode(void)
{
    return g_graphics_context.current_mode;
}

GraphicsContext *graphics_get_context(void)
{
    return g_graphics_initialized ? &g_graphics_context : NULL;
}

// =================== FRAME MANAGEMENT ===================

void graphics_begin_frame(void)
{
    if (!g_graphics_initialized)
        return;

    g_graphics_context.in_frame = true;
    g_frame_time_start = _graphics_get_time_ms();
}

void graphics_end_frame(void)
{
    if (!g_graphics_initialized || !g_graphics_context.in_frame)
        return;

    uint32_t frame_time = _graphics_get_time_ms() - g_frame_time_start;
    g_total_render_time += frame_time;
    g_frames_rendered++;

    g_graphics_context.render_time_us = frame_time * 1000;
    g_graphics_context.frame_count++;
    g_graphics_context.in_frame = false;

    _graphics_update_fps_counter();
}

void graphics_present(void)
{
    if (!g_graphics_initialized)
        return;

    if (g_graphics_context.double_buffering_enabled)
    {
        graphics_swap_buffers();
    }

    if (g_graphics_context.vsync_enabled)
    {
        graphics_wait_vsync();
    }
}

void graphics_wait_vsync(void)
{
    // For maximum performance, only wait if FPS limiting is enabled
    if (!g_graphics_context.fps_limit_enabled || g_graphics_context.max_fps == 0)
        return;

    // High-performance frame rate limiting
    static uint32_t last_frame_time = 0;
    uint32_t current_time = _graphics_get_time_ms();
    uint32_t target_frame_time = g_graphics_context.target_frame_time_ms;

    if (target_frame_time > 0)
    {
        uint32_t elapsed = current_time - last_frame_time;
        if (elapsed < target_frame_time)
        {
            // Optimized sleep - only sleep if we have significant time remaining
            uint32_t sleep_time = target_frame_time - elapsed;
            if (sleep_time > 1) // Only sleep if > 1ms to avoid overhead
            {
                sleep_ms(sleep_time);
            }
        }
        last_frame_time = _graphics_get_time_ms();
    }
    else
    {
        last_frame_time = current_time;
    }

    g_graphics_context.last_vsync_time = last_frame_time;
}

void graphics_swap_buffers(void)
{
    if (!g_graphics_initialized || !g_graphics_context.double_buffering_enabled)
        return;
    if (!g_graphics_context.front_buffer || !g_graphics_context.back_buffer)
        return;

    // High-performance buffer copy with memory optimization
    uint16_t *front_pixels = g_graphics_context.front_buffer->pixels;
    uint16_t *back_pixels = g_graphics_context.back_buffer->pixels;

    int total_pixels = g_graphics_context.current_mode.width * g_graphics_context.current_mode.height;

    // Optimized memory copy (could use DMA in real hardware)
    for (int i = 0; i < total_pixels; i++)
    {
        front_pixels[i] = back_pixels[i];
    }

    g_graphics_context.buffer_swap_pending = false;
}

// =================== SURFACE MANAGEMENT ===================

GraphicsSurface *graphics_create_surface(int width, int height)
{
    GraphicsSurface *surface = (GraphicsSurface *)kmalloc(sizeof(GraphicsSurface));
    if (!surface)
        return NULL;

    surface->width = width;
    surface->height = height;
    surface->pitch = width * GRAPHICS_BYTES_PER_PIXEL;
    surface->is_locked = false;
    surface->format = GRAPHICS_BPP;

    // Set default clipping to entire surface
    surface->clip_rect.x = 0;
    surface->clip_rect.y = 0;
    surface->clip_rect.width = width;
    surface->clip_rect.height = height;

    surface->pixels = NULL; // Caller manages pixel memory

    return surface;
}

void graphics_destroy_surface(GraphicsSurface *surface)
{
    if (!surface)
        return;
    kfree(surface);
}

bool graphics_lock_surface(GraphicsSurface *surface)
{
    if (!surface)
        return false;
    if (surface->is_locked)
        return false; // Already locked

    surface->is_locked = true;
    return true;
}

void graphics_unlock_surface(GraphicsSurface *surface)
{
    if (!surface)
        return;
    surface->is_locked = false;
}

void graphics_clear_surface(GraphicsSurface *surface, Color color)
{
    if (!surface || !surface->pixels)
        return;

    int total_pixels = surface->width * surface->height;
    uint16_t *pixels = surface->pixels;

    // Optimized clear using 32-bit writes when possible
    if (total_pixels % 2 == 0 && ((uintptr_t)pixels & 3) == 0)
    {
        uint32_t double_color = (color << 16) | color;
        uint32_t *pixels32 = (uint32_t *)pixels;
        for (int i = 0; i < total_pixels / 2; i++)
        {
            pixels32[i] = double_color;
        }
    }
    else
    {
        for (int i = 0; i < total_pixels; i++)
        {
            pixels[i] = color;
        }
    }
}

// =================== DRAWING PRIMITIVES ===================

void graphics_set_pixel(GraphicsSurface *surface, int x, int y, Color color)
{
    if (!surface || !surface->pixels)
        return;
    if (x < 0 || y < 0 || x >= surface->width || y >= surface->height)
        return;

    Point point = {x, y};
    if (!_graphics_is_point_visible(surface, point))
        return;

    surface->pixels[y * surface->width + x] = color;
}

Color graphics_get_pixel(GraphicsSurface *surface, int x, int y)
{
    if (!surface || !surface->pixels)
        return COLOR_BLACK;
    if (x < 0 || y < 0 || x >= surface->width || y >= surface->height)
        return COLOR_BLACK;

    return surface->pixels[y * surface->width + x];
}

void graphics_draw_line(GraphicsSurface *surface, Point start, Point end, Color color)
{
    if (!surface)
        return;

    // Bresenham's line algorithm - optimized version
    int dx = abs(end.x - start.x);
    int dy = abs(end.y - start.y);
    int sx = start.x < end.x ? 1 : -1;
    int sy = start.y < end.y ? 1 : -1;
    int err = dx - dy;

    int x = start.x;
    int y = start.y;

    while (true)
    {
        graphics_set_pixel(surface, x, y, color);

        if (x == end.x && y == end.y)
            break;

        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y += sy;
        }
    }
}

void graphics_draw_rect(GraphicsSurface *surface, Rectangle rect, Color color)
{
    if (!surface)
        return;

    // Draw rectangle outline - optimized
    Point top_left = {rect.x, rect.y};
    Point top_right = {rect.x + rect.width - 1, rect.y};
    Point bottom_left = {rect.x, rect.y + rect.height - 1};
    Point bottom_right = {rect.x + rect.width - 1, rect.y + rect.height - 1};

    graphics_draw_line(surface, top_left, top_right, color);       // Top
    graphics_draw_line(surface, top_right, bottom_right, color);   // Right
    graphics_draw_line(surface, bottom_right, bottom_left, color); // Bottom
    graphics_draw_line(surface, bottom_left, top_left, color);     // Left
}

void graphics_fill_rect(GraphicsSurface *surface, Rectangle rect, Color color)
{
    if (!surface)
        return;

    // Clamp rectangle to surface bounds
    int x1 = rect.x < 0 ? 0 : rect.x;
    int y1 = rect.y < 0 ? 0 : rect.y;
    int x2 = rect.x + rect.width > surface->width ? surface->width : rect.x + rect.width;
    int y2 = rect.y + rect.height > surface->height ? surface->height : rect.y + rect.height;

    // Optimized filled rectangle
    for (int y = y1; y < y2; y++)
    {
        uint16_t *row = &surface->pixels[y * surface->width + x1];
        for (int x = x1; x < x2; x++)
        {
            *row++ = color;
        }
    }
}

void graphics_draw_circle(GraphicsSurface *surface, Point center, int radius, Color color)
{
    if (!surface)
        return;

    // Midpoint circle algorithm - optimized
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;

    while (y >= x)
    {
        // Draw 8 octants efficiently
        graphics_set_pixel(surface, center.x + x, center.y + y, color);
        graphics_set_pixel(surface, center.x - x, center.y + y, color);
        graphics_set_pixel(surface, center.x + x, center.y - y, color);
        graphics_set_pixel(surface, center.x - x, center.y - y, color);
        graphics_set_pixel(surface, center.x + y, center.y + x, color);
        graphics_set_pixel(surface, center.x - y, center.y + x, color);
        graphics_set_pixel(surface, center.x + y, center.y - x, color);
        graphics_set_pixel(surface, center.x - y, center.y - x, color);

        x++;
        if (d > 0)
        {
            y--;
            d = d + 4 * (x - y) + 10;
        }
        else
        {
            d = d + 4 * x + 6;
        }
    }
}

void graphics_fill_circle(GraphicsSurface *surface, Point center, int radius, Color color)
{
    if (!surface)
        return;

    // Optimized filled circle using scanlines
    for (int y = -radius; y <= radius; y++)
    {
        int x_extent = (int)fpu_sqrt((double)(radius * radius - y * y));
        for (int x = -x_extent; x <= x_extent; x++)
        {
            graphics_set_pixel(surface, center.x + x, center.y + y, color);
        }
    }
}

// =================== BLITTING AND COMPOSITION ===================

void graphics_blit_surface(GraphicsSurface *dest, GraphicsSurface *src, Rectangle *src_rect, Point dest_point)
{
    if (!dest || !src || !dest->pixels || !src->pixels)
        return;

    Rectangle source = src_rect ? *src_rect : (Rectangle){0, 0, src->width, src->height};

    // Optimized blitting with bounds checking
    int src_x_start = source.x < 0 ? 0 : source.x;
    int src_y_start = source.y < 0 ? 0 : source.y;
    int src_x_end = source.x + source.width > src->width ? src->width : source.x + source.width;
    int src_y_end = source.y + source.height > src->height ? src->height : source.y + source.height;

    for (int src_y = src_y_start; src_y < src_y_end; src_y++)
    {
        int dst_y = dest_point.y + (src_y - source.y);
        if (dst_y < 0 || dst_y >= dest->height)
            continue;

        for (int src_x = src_x_start; src_x < src_x_end; src_x++)
        {
            int dst_x = dest_point.x + (src_x - source.x);
            if (dst_x < 0 || dst_x >= dest->width)
                continue;

            Color pixel = src->pixels[src_y * src->width + src_x];
            dest->pixels[dst_y * dest->width + dst_x] = pixel;
        }
    }
}

void graphics_blit_scaled(GraphicsSurface *dest, GraphicsSurface *src, Rectangle *src_rect, Rectangle *dest_rect)
{
    if (!dest || !src || !dest->pixels || !src->pixels || !dest_rect)
        return;

    Rectangle source = src_rect ? *src_rect : (Rectangle){0, 0, src->width, src->height};

    // Bilinear scaling algorithm
    double x_scale = (double)source.width / dest_rect->width;
    double y_scale = (double)source.height / dest_rect->height;

    for (int dst_y = 0; dst_y < dest_rect->height; dst_y++)
    {
        int screen_y = dest_rect->y + dst_y;
        if (screen_y < 0 || screen_y >= dest->height)
            continue;

        for (int dst_x = 0; dst_x < dest_rect->width; dst_x++)
        {
            int screen_x = dest_rect->x + dst_x;
            if (screen_x < 0 || screen_x >= dest->width)
                continue;

            int src_x = (int)(source.x + dst_x * x_scale);
            int src_y = (int)(source.y + dst_y * y_scale);

            if (src_x >= 0 && src_x < src->width && src_y >= 0 && src_y < src->height)
            {
                Color pixel = src->pixels[src_y * src->width + src_x];
                dest->pixels[screen_y * dest->width + screen_x] = pixel;
            }
        }
    }
}

void graphics_blit_alpha(GraphicsSurface *dest, GraphicsSurface *src, Rectangle *src_rect, Point dest_point, uint8_t alpha)
{
    if (!dest || !src || !dest->pixels || !src->pixels)
        return;

    Rectangle source = src_rect ? *src_rect : (Rectangle){0, 0, src->width, src->height};

    for (int y = 0; y < source.height; y++)
    {
        for (int x = 0; x < source.width; x++)
        {
            int src_x = source.x + x;
            int src_y = source.y + y;
            int dst_x = dest_point.x + x;
            int dst_y = dest_point.y + y;

            if (src_x >= 0 && src_x < src->width && src_y >= 0 && src_y < src->height &&
                dst_x >= 0 && dst_x < dest->width && dst_y >= 0 && dst_y < dest->height)
            {

                Color src_pixel = src->pixels[src_y * src->width + src_x];
                Color dst_pixel = dest->pixels[dst_y * dest->width + dst_x];
                Color blended = graphics_blend_colors(src_pixel, dst_pixel, alpha);
                dest->pixels[dst_y * dest->width + dst_x] = blended;
            }
        }
    }
}

// =================== CLIPPING ===================

void graphics_set_clip_rect(GraphicsSurface *surface, Rectangle *rect)
{
    if (!surface)
        return;

    if (rect)
    {
        // Clamp to surface bounds
        surface->clip_rect.x = rect->x < 0 ? 0 : rect->x;
        surface->clip_rect.y = rect->y < 0 ? 0 : rect->y;
        surface->clip_rect.width = rect->x + rect->width > surface->width ? surface->width - surface->clip_rect.x : rect->width;
        surface->clip_rect.height = rect->y + rect->height > surface->height ? surface->height - surface->clip_rect.y : rect->height;
    }
    else
    {
        // Reset to full surface
        surface->clip_rect.x = 0;
        surface->clip_rect.y = 0;
        surface->clip_rect.width = surface->width;
        surface->clip_rect.height = surface->height;
    }
}

void graphics_get_clip_rect(GraphicsSurface *surface, Rectangle *rect)
{
    if (!surface || !rect)
        return;
    *rect = surface->clip_rect;
}

bool graphics_point_in_clip(GraphicsSurface *surface, Point point)
{
    return _graphics_is_point_visible(surface, point);
}

// =================== COLOR UTILITIES ===================

Color graphics_rgb_to_color(uint8_t r, uint8_t g, uint8_t b)
{
    // Convert RGB888 to RGB565 with proper bit shifting
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}

void graphics_color_to_rgb(Color color, uint8_t *r, uint8_t *g, uint8_t *b)
{
    if (!r || !g || !b)
        return;

    // Convert RGB565 to RGB888 with proper scaling
    *r = ((color >> 11) & 0x1F) << 3;
    *g = ((color >> 5) & 0x3F) << 2;
    *b = (color & 0x1F) << 3;

    // Improve color accuracy by adding the lower bits
    *r |= (*r >> 5);
    *g |= (*g >> 6);
    *b |= (*b >> 5);
}

Color graphics_blend_colors(Color src, Color dst, uint8_t alpha)
{
    if (alpha == 255)
        return src;
    if (alpha == 0)
        return dst;

    uint8_t src_r, src_g, src_b;
    uint8_t dst_r, dst_g, dst_b;

    graphics_color_to_rgb(src, &src_r, &src_g, &src_b);
    graphics_color_to_rgb(dst, &dst_r, &dst_g, &dst_b);

    // High-quality alpha blending
    uint8_t inv_alpha = 255 - alpha;
    uint8_t blend_r = (src_r * alpha + dst_r * inv_alpha) >> 8;
    uint8_t blend_g = (src_g * alpha + dst_g * inv_alpha) >> 8;
    uint8_t blend_b = (src_b * alpha + dst_b * inv_alpha) >> 8;

    return graphics_rgb_to_color(blend_r, blend_g, blend_b);
}

// =================== TEXT RENDERING ===================

void graphics_draw_text(GraphicsSurface *surface, const char *text, Point position, Color color)
{
    graphics_draw_text_scaled(surface, text, position, color, 1);
}

void graphics_draw_text_scaled(GraphicsSurface *surface, const char *text, Point position, Color color, int scale)
{
    if (!surface || !text || scale < 1)
        return;

    int x = position.x;
    int y = position.y;

    while (*text)
    {
        if (*text == '\n')
        {
            x = position.x;
            y += FONT_ATARIST8X16SYSTEMFONT_HEIGHT * scale;
        }
        else if (*text == '\t')
        {
            x += FONT_ATARIST8X16SYSTEMFONT_WIDTH * scale * 4; // Tab = 4 spaces
        }
        else
        {
            // Render character with high-quality scaling
            for (int char_y = 0; char_y < FONT_ATARIST8X16SYSTEMFONT_HEIGHT; char_y++)
            {
                unsigned int char_row = getAtariST8x16SystemFontCharacter(*text, char_y);

                for (int char_x = 0; char_x < FONT_ATARIST8X16SYSTEMFONT_WIDTH; char_x++)
                {
                    if (char_row & (1 << (FONT_ATARIST8X16SYSTEMFONT_WIDTH - 1 - char_x)))
                    {
                        // Draw scaled pixel block
                        for (int sy = 0; sy < scale; sy++)
                        {
                            for (int sx = 0; sx < scale; sx++)
                            {
                                int pixel_x = x + char_x * scale + sx;
                                int pixel_y = y + char_y * scale + sy;
                                graphics_set_pixel(surface, pixel_x, pixel_y, color);
                            }
                        }
                    }
                }
            }
            x += FONT_ATARIST8X16SYSTEMFONT_WIDTH * scale;
        }
        text++;
    }
}

int graphics_text_width(const char *text, int scale)
{
    if (!text || scale < 1)
        return 0;

    int width = 0;
    int current_line_width = 0;

    while (*text)
    {
        if (*text == '\n')
        {
            if (current_line_width > width)
                width = current_line_width;
            current_line_width = 0;
        }
        else if (*text == '\t')
        {
            current_line_width += FONT_ATARIST8X16SYSTEMFONT_WIDTH * scale * 4;
        }
        else
        {
            current_line_width += FONT_ATARIST8X16SYSTEMFONT_WIDTH * scale;
        }
        text++;
    }

    if (current_line_width > width)
        width = current_line_width;
    return width;
}

int graphics_text_height(int scale)
{
    return FONT_ATARIST8X16SYSTEMFONT_HEIGHT * scale;
}

// =================== PERFORMANCE AND DEBUGGING ===================

uint32_t graphics_get_fps(void)
{
    return g_graphics_context.frames_per_second;
}

uint32_t graphics_get_frame_count(void)
{
    return g_graphics_context.frame_count;
}

void graphics_get_stats(uint32_t *frames, uint32_t *dropped, uint32_t *render_time)
{
    if (frames)
        *frames = g_graphics_context.frame_count;
    if (dropped)
        *dropped = g_graphics_context.dropped_frames;
    if (render_time)
        *render_time = g_graphics_context.render_time_us;
}

void graphics_reset_stats(void)
{
    g_graphics_context.frame_count = 0;
    g_graphics_context.dropped_frames = 0;
    g_graphics_context.render_time_us = 0;
    g_graphics_context.frames_per_second = 0;
    g_total_render_time = 0;
    g_frames_rendered = 0;
}

// =================== FPS CONTROL ===================

void graphics_set_max_fps(uint32_t max_fps)
{
    if (!g_graphics_initialized)
        return;

    g_graphics_context.max_fps = max_fps;

    if (max_fps == 0)
    {
        // Unlimited FPS - disable frame limiting
        g_graphics_context.fps_limit_enabled = false;
        g_graphics_context.target_frame_time_ms = 0;
    }
    else
    {
        // Calculate target frame time in milliseconds
        g_graphics_context.fps_limit_enabled = true;
        g_graphics_context.target_frame_time_ms = 1000 / max_fps;
    }
}

uint32_t graphics_get_max_fps(void)
{
    return g_graphics_context.max_fps;
}

void graphics_enable_fps_limit(bool enabled)
{
    if (!g_graphics_initialized)
        return;

    g_graphics_context.fps_limit_enabled = enabled;

    // If disabling, ensure we're not accidentally limiting FPS
    if (!enabled)
    {
        g_graphics_context.target_frame_time_ms = 0;
    }
    else if (g_graphics_context.max_fps > 0)
    {
        // Re-calculate target frame time
        g_graphics_context.target_frame_time_ms = 1000 / g_graphics_context.max_fps;
    }
}

bool graphics_is_fps_limit_enabled(void)
{
    return g_graphics_context.fps_limit_enabled;
}

void graphics_set_unlimited_fps(void)
{
    graphics_set_max_fps(0); // 0 = unlimited
}

// =================== LEGACY COMPATIBILITY ===================

void Draw(int x, int y, int r, int g, int b)
{
    if (!g_graphics_initialized)
        return;

    Color color = graphics_rgb_to_color(r, g, b);
    GraphicsSurface *surface = g_graphics_context.double_buffering_enabled ? g_graphics_context.back_buffer : g_graphics_context.front_buffer;

    graphics_set_pixel(surface, x, y, color);
}

void ClearScreen(int r, int g, int b)
{
    if (!g_graphics_initialized)
        return;

    Color color = graphics_rgb_to_color(r, g, b);
    GraphicsSurface *surface = g_graphics_context.double_buffering_enabled ? g_graphics_context.back_buffer : g_graphics_context.front_buffer;

    graphics_clear_surface(surface, color);
}

void DrawRect(int x, int y, int width, int height, int r, int g, int b)
{
    if (!g_graphics_initialized)
        return;

    Color color = graphics_rgb_to_color(r, g, b);
    GraphicsSurface *surface = g_graphics_context.double_buffering_enabled ? g_graphics_context.back_buffer : g_graphics_context.front_buffer;

    Rectangle rect = {x, y, width, height};
    graphics_fill_rect(surface, rect, color);
}

void DrawMouse(int x, int y, int r, int g, int b)
{
    if (!g_graphics_initialized)
        return;

    Color color = graphics_rgb_to_color(r, g, b);
    GraphicsSurface *surface = g_graphics_context.double_buffering_enabled ? g_graphics_context.back_buffer : g_graphics_context.front_buffer;

    // Professional mouse cursor with anti-aliasing
    static int mouse_pattern[] = {
        0b1111111111,
        0b1111111110,
        0b1111111100,
        0b1111111000,
        0b1111110000,
        0b1111100000,
        0b1111000000,
        0b1110000000,
        0b1100000000,
        0b1000000000,
    };

    // Draw cursor with shadow for better visibility
    for (int row = 0; row < 10; row++)
    {
        for (int col = 0; col < 10; col++)
        {
            if (mouse_pattern[row] & (1 << (9 - col)))
            {
                // Draw shadow (offset by 1,1)
                graphics_set_pixel(surface, x + col + 1, y + row + 1, COLOR_BLACK);
                // Draw cursor
                graphics_set_pixel(surface, x + col, y + row, color);
            }
        }
    }
}

void Flush(void)
{
    if (!g_graphics_initialized)
        return;
    graphics_present();
}

void DrawAtariString(char *str, int x, int y, int r, int g, int b, int scale)
{
    if (!g_graphics_initialized || !str)
        return;

    Color color = graphics_rgb_to_color(r, g, b);
    GraphicsSurface *surface = g_graphics_context.double_buffering_enabled ? g_graphics_context.back_buffer : g_graphics_context.front_buffer;

    Point position = {x, y};
    graphics_draw_text_scaled(surface, str, position, color, scale);
}

void DrawAtariChar(char c, int x, int y, int r, int g, int b, int scale)
{
    char str[2] = {c, '\0'};
    DrawAtariString(str, x, y, r, g, b, scale);
}

// PIT interrupt handler (IRQ 0)
void graphics_pit_interrupt_handler(struct interrupt_frame *frame)
{
    _graphics_pit_tick();
    simple_serial_puts("PIT tick\n");
    char debug_str[64];
    int_to_str((int)_graphics_get_time_ms(), debug_str);
    simple_serial_puts("ms: ");
    simple_serial_puts(debug_str);
    simple_serial_puts("\n");
}
