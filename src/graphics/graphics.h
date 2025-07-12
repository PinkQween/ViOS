#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "fonts/characters_AtariST8x16SystemFont.h"
#include <stdint.h>
#include <stdbool.h>

// Forward declaration for interrupt frame
struct interrupt_frame;

// VBE Information Block Structure
typedef struct VBEInfoBlockStruct
{
    unsigned short mode_attribute;
    unsigned char win_a_attribute;
    unsigned char win_b_attribute;
    unsigned short win_granuality;
    unsigned short win_size;
    unsigned short win_a_segment;
    unsigned short win_b_segment;
    unsigned int win_func_ptr;
    unsigned short bytes_per_scan_line;
    unsigned short x_resolution;
    unsigned short y_resolution;
    unsigned char char_x_size;
    unsigned char char_y_size;
    unsigned char number_of_planes;
    unsigned char bits_per_pixel;
    unsigned char number_of_banks;
    unsigned char memory_model;
    unsigned char bank_size;
    unsigned char number_of_image_pages;
    unsigned char b_reserved;
    unsigned char red_mask_size;
    unsigned char red_field_position;
    unsigned char green_mask_size;
    unsigned char green_field_position;
    unsigned char blue_mask_size;
    unsigned char blue_field_position;
    unsigned char reserved_mask_size;
    unsigned char reserved_field_position;
    unsigned char direct_color_info;
    unsigned int screen_ptr;
} VBEInfoBlock;

// Memory addresses
#define VBEInfoAddress 0x900
#define FRAMEBUFFER_PRIMARY 0x00A00000
#define FRAMEBUFFER_SECONDARY 0x00B00000

// Graphics constants
#define GRAPHICS_MAX_WIDTH 1920
#define GRAPHICS_MAX_HEIGHT 1080
#define GRAPHICS_BPP 16
#define GRAPHICS_BYTES_PER_PIXEL 2

// Color format (RGB565)
typedef uint16_t Color;
#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF
#define COLOR_RED 0xF800
#define COLOR_GREEN 0x07E0
#define COLOR_BLUE 0x001F

// Rectangle structure
typedef struct
{
    int x, y;
    int width, height;
} Rectangle;

// Point structure
typedef struct
{
    int x, y;
} Point;

// Surface structure for advanced rendering
typedef struct GraphicsSurface
{
    uint16_t *pixels;
    int width;
    int height;
    int pitch;           // Bytes per scanline
    Rectangle clip_rect; // Clipping rectangle
    bool is_locked;      // Surface lock state
    uint32_t format;     // Pixel format
} GraphicsSurface;

// Display mode structure
typedef struct
{
    int width;
    int height;
    int bpp;
    int refresh_rate;
    bool interlaced;
} DisplayMode;

// Graphics context for rendering state
typedef struct
{
    // Display information
    DisplayMode current_mode;
    VBEInfoBlock *vbe_info;

    // Double buffering
    GraphicsSurface *front_buffer;
    GraphicsSurface *back_buffer;
    bool double_buffering_enabled;
    bool buffer_swap_pending;

    // VSync and timing
    bool vsync_enabled;
    uint32_t last_vsync_time;
    uint32_t frame_count;
    uint32_t dropped_frames;

    // FPS Control
    uint32_t max_fps;              // Maximum FPS limit (0 = unlimited)
    bool fps_limit_enabled;        // Whether FPS limiting is active
    uint32_t target_frame_time_ms; // Target frame time in milliseconds

    // Clipping and transformation
    Rectangle global_clip;
    Point origin_offset;

    // Performance counters
    uint32_t frames_per_second;
    uint32_t last_fps_update;
    uint32_t render_time_us;

    // State flags
    bool initialized;
    bool in_frame;
    bool needs_full_refresh;

    // Hardware capabilities
    bool hardware_acceleration;
    bool hardware_cursor;
    bool linear_framebuffer;

} GraphicsContext;

// Core Graphics API
bool graphics_initialize(void);
void graphics_shutdown(void);
bool graphics_set_mode(int width, int height, int bpp);
DisplayMode graphics_get_current_mode(void);
GraphicsContext *graphics_get_context(void);

// Frame management
void graphics_begin_frame(void);
void graphics_end_frame(void);
void graphics_present(void);
void graphics_wait_vsync(void);
void graphics_swap_buffers(void);

// Surface management
GraphicsSurface *graphics_create_surface(int width, int height);
void graphics_destroy_surface(GraphicsSurface *surface);
bool graphics_lock_surface(GraphicsSurface *surface);
void graphics_unlock_surface(GraphicsSurface *surface);
void graphics_clear_surface(GraphicsSurface *surface, Color color);

// Drawing primitives
void graphics_set_pixel(GraphicsSurface *surface, int x, int y, Color color);
Color graphics_get_pixel(GraphicsSurface *surface, int x, int y);
void graphics_draw_line(GraphicsSurface *surface, Point start, Point end, Color color);
void graphics_draw_rect(GraphicsSurface *surface, Rectangle rect, Color color);
void graphics_fill_rect(GraphicsSurface *surface, Rectangle rect, Color color);
void graphics_draw_circle(GraphicsSurface *surface, Point center, int radius, Color color);
void graphics_fill_circle(GraphicsSurface *surface, Point center, int radius, Color color);

// Blitting and composition
void graphics_blit_surface(GraphicsSurface *dest, GraphicsSurface *src, Rectangle *src_rect, Point dest_point);
void graphics_blit_scaled(GraphicsSurface *dest, GraphicsSurface *src, Rectangle *src_rect, Rectangle *dest_rect);
void graphics_blit_alpha(GraphicsSurface *dest, GraphicsSurface *src, Rectangle *src_rect, Point dest_point, uint8_t alpha);

// Clipping
void graphics_set_clip_rect(GraphicsSurface *surface, Rectangle *rect);
void graphics_get_clip_rect(GraphicsSurface *surface, Rectangle *rect);
bool graphics_point_in_clip(GraphicsSurface *surface, Point point);

// Color utilities
Color graphics_rgb_to_color(uint8_t r, uint8_t g, uint8_t b);
void graphics_color_to_rgb(Color color, uint8_t *r, uint8_t *g, uint8_t *b);
Color graphics_blend_colors(Color src, Color dst, uint8_t alpha);

// Text rendering
void graphics_draw_text(GraphicsSurface *surface, const char *text, Point position, Color color);
void graphics_draw_text_scaled(GraphicsSurface *surface, const char *text, Point position, Color color, int scale);
int graphics_text_width(const char *text, int scale);
int graphics_text_height(int scale);

// FPS Control
void graphics_set_max_fps(uint32_t max_fps);
uint32_t graphics_get_max_fps(void);
void graphics_enable_fps_limit(bool enabled);
bool graphics_is_fps_limit_enabled(void);
void graphics_set_unlimited_fps(void);

// Performance and debugging
uint32_t graphics_get_fps(void);
uint32_t graphics_get_frame_count(void);
void graphics_get_stats(uint32_t *frames, uint32_t *dropped, uint32_t *render_time);
void graphics_reset_stats(void);

// Legacy compatibility functions
void Draw(int x, int y, int r, int g, int b);
void ClearScreen(int r, int g, int b);
void DrawRect(int x, int y, int width, int height, int r, int g, int b);
void DrawRoundedRect(int x, int y, int width, int height, int radius, int r, int g, int b);
void FillRoundedRect(int x, int y, int width, int height, int radius, int r, int g, int b);
void DrawMouse(int x, int y, int r, int g, int b);
void Flush(void);
void DrawAtariString(char *str, int x, int y, int r, int g, int b, int scale);
void DrawAtariChar(char c, int x, int y, int r, int g, int b, int scale);

// Internal utility functions (not for external use)
bool _graphics_init_framebuffer(void);
void _graphics_init_surfaces(void);
uint32_t _graphics_get_time_ms(void);
bool _graphics_is_point_visible(GraphicsSurface *surface, Point point);
void _graphics_update_fps_counter(void);
void graphics_pit_interrupt_handler(struct interrupt_frame *frame);

#endif // GRAPHICS_H
