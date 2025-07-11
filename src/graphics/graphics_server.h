#ifndef GRAPHICS_SERVER_H
#define GRAPHICS_SERVER_H

#include <stdint.h>
#include <stdbool.h>
#include "graphics/surface.h"
#include "ipc/ipc.h"

// Graphics command types
typedef enum {
    GRAPHICS_CMD_CLEAR,
    GRAPHICS_CMD_DRAW_PIXEL,
    GRAPHICS_CMD_DRAW_RECT,
    GRAPHICS_CMD_DRAW_CIRCLE,
    GRAPHICS_CMD_DRAW_TEXT,
    GRAPHICS_CMD_BLIT_SURFACE,
    GRAPHICS_CMD_UPDATE_REGION
} graphics_command_type_t;

// Graphics command structures
struct graphics_command {
    graphics_command_type_t type;
    union {
        struct {
            uint32_t color;
        } clear;
        
        struct {
            int x, y;
            uint32_t color;
        } pixel;
        
        struct {
            int x, y;
            int width, height;
            uint32_t color;
        } rect;
        
        struct {
            int x, y;
            int radius;
            uint32_t color;
        } circle;
        
        struct {
            int x, y;
            char text[256];
            uint32_t color;
        } text;
        
        struct {
            struct surface *source;
            int src_x, src_y;
            int dst_x, dst_y;
            int width, height;
        } blit;
        
        struct {
            int x, y;
            int width, height;
        } region;
    };
};

// Graphics server functions
int graphics_server_init(void);
void graphics_server_run(void);
void graphics_server_shutdown(void);

// Command processing
void graphics_process_commands(void);
int graphics_server_queue_command(struct graphics_command *cmd);
void graphics_update_screen(void);

// Drawing functions
void graphics_clear_screen(uint32_t color);
void graphics_draw_pixel(int x, int y, uint32_t color);
void graphics_draw_rectangle(int x, int y, int width, int height, uint32_t color);
void graphics_draw_circle(int x, int y, int radius, uint32_t color);
void graphics_draw_text(int x, int y, const char *text, uint32_t color);

// IPC handling
int graphics_server_handle_request(struct ipc_message *msg);

// Graphics server state
struct graphics_server_state {
    bool initialized;
    struct graphics_context *ctx;
    struct surface *back_buffer;
    struct surface *front_buffer;
    bool dirty;
    uint32_t frame_count;
    uint32_t last_update_time;
};

#endif // GRAPHICS_SERVER_H
