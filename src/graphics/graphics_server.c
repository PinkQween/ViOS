#include "graphics_server.h"
#include "graphics/graphics.h"
#include "common/debug.h"
#include "task/scheduler.h"
#include "memory/memory.h"
#include "ipc/ipc.h"

// Graphics server state
static struct graphics_server_state {
    bool initialized;
    struct graphics_context *ctx;
    struct surface *back_buffer;
    struct surface *front_buffer;
    bool dirty;
    uint32_t frame_count;
    uint32_t last_update_time;
} graphics_state;

// Graphics commands queue
#define MAX_GRAPHICS_COMMANDS 256
static struct graphics_command command_queue[MAX_GRAPHICS_COMMANDS];
static int command_queue_head = 0;
static int command_queue_tail = 0;

int graphics_server_init(void) {
    debug_print("Initializing graphics server...\n");
    
    // Initialize graphics system
    if (graphics_init() < 0) {
        debug_print("Failed to initialize graphics system\n");
        return -1;
    }
    
    // Get graphics context
    graphics_state.ctx = graphics_get_context();
    if (!graphics_state.ctx) {
        debug_print("Failed to get graphics context\n");
        return -1;
    }
    
    // Create back buffer
    graphics_state.back_buffer = surface_create(
        graphics_state.ctx->screen_width,
        graphics_state.ctx->screen_height,
        SURFACE_FORMAT_RGB32
    );
    if (!graphics_state.back_buffer) {
        debug_print("Failed to create back buffer\n");
        return -1;
    }
    
    // Get front buffer reference
    graphics_state.front_buffer = graphics_state.ctx->screen_buffer;
    
    // Clear initial state
    graphics_clear_screen(0x000000);  // Black background
    graphics_state.dirty = true;
    graphics_state.frame_count = 0;
    graphics_state.last_update_time = 0;
    graphics_state.initialized = true;
    
    debug_print("Graphics server initialized successfully\n");
    return 0;
}

void graphics_server_run(void) {
    debug_print("Starting graphics server main loop\n");
    
    while (graphics_state.initialized) {
        // Process graphics commands
        graphics_process_commands();
        
        // Update screen if dirty
        if (graphics_state.dirty) {
            graphics_update_screen();
            graphics_state.dirty = false;
            graphics_state.frame_count++;
        }
        
        // Monitor service health
        // TODO: Add health monitoring
        
        // Yield to other tasks
        task_yield();
    }
}

void graphics_process_commands(void) {
    while (command_queue_head != command_queue_tail) {
        struct graphics_command *cmd = &command_queue[command_queue_head];
        
        switch (cmd->type) {
            case GRAPHICS_CMD_CLEAR:
                graphics_clear_screen(cmd->clear.color);
                break;
                
            case GRAPHICS_CMD_DRAW_PIXEL:
                graphics_draw_pixel(cmd->pixel.x, cmd->pixel.y, cmd->pixel.color);
                break;
                
            case GRAPHICS_CMD_DRAW_RECT:
                graphics_draw_rectangle(
                    cmd->rect.x, cmd->rect.y,
                    cmd->rect.width, cmd->rect.height,
                    cmd->rect.color
                );
                break;
                
            case GRAPHICS_CMD_DRAW_CIRCLE:
                graphics_draw_circle(
                    cmd->circle.x, cmd->circle.y,
                    cmd->circle.radius,
                    cmd->circle.color
                );
                break;
                
            case GRAPHICS_CMD_DRAW_TEXT:
                graphics_draw_text(
                    cmd->text.x, cmd->text.y,
                    cmd->text.text,
                    cmd->text.color
                );
                break;
                
            case GRAPHICS_CMD_BLIT_SURFACE:
                // TODO: Implement surface blitting
                break;
                
            case GRAPHICS_CMD_UPDATE_REGION:
                // Mark specific region as dirty
                graphics_state.dirty = true;
                break;
                
            default:
                debug_print("Unknown graphics command: %d\n", cmd->type);
                break;
        }
        
        // Move to next command
        command_queue_head = (command_queue_head + 1) % MAX_GRAPHICS_COMMANDS;
        graphics_state.dirty = true;
    }
}

void graphics_update_screen(void) {
    // Copy back buffer to front buffer
    if (graphics_state.back_buffer && graphics_state.front_buffer) {
        surface_blit(
            graphics_state.back_buffer,
            graphics_state.front_buffer,
            0, 0, 0, 0,
            graphics_state.ctx->screen_width,
            graphics_state.ctx->screen_height
        );
    }
    
    // Update display
    graphics_swap_buffers();
}

int graphics_server_queue_command(struct graphics_command *cmd) {
    int next_tail = (command_queue_tail + 1) % MAX_GRAPHICS_COMMANDS;
    
    // Check if queue is full
    if (next_tail == command_queue_head) {
        debug_print("Graphics command queue full\n");
        return -1;
    }
    
    // Add command to queue
    command_queue[command_queue_tail] = *cmd;
    command_queue_tail = next_tail;
    
    return 0;
}

void graphics_clear_screen(uint32_t color) {
    if (!graphics_state.back_buffer) return;
    
    surface_fill(graphics_state.back_buffer, 0, 0,
                 graphics_state.ctx->screen_width,
                 graphics_state.ctx->screen_height,
                 color);
}

void graphics_draw_pixel(int x, int y, uint32_t color) {
    if (!graphics_state.back_buffer) return;
    
    surface_set_pixel(graphics_state.back_buffer, x, y, color);
}

void graphics_draw_rectangle(int x, int y, int width, int height, uint32_t color) {
    if (!graphics_state.back_buffer) return;
    
    surface_fill(graphics_state.back_buffer, x, y, width, height, color);
}

void graphics_draw_circle(int x, int y, int radius, uint32_t color) {
    if (!graphics_state.back_buffer) return;
    
    // Simple circle drawing algorithm
    int cx = x;
    int cy = y;
    int r = radius;
    
    for (int dy = -r; dy <= r; dy++) {
        for (int dx = -r; dx <= r; dx++) {
            if (dx*dx + dy*dy <= r*r) {
                surface_set_pixel(graphics_state.back_buffer, cx + dx, cy + dy, color);
            }
        }
    }
}

void graphics_draw_text(int x, int y, const char *text, uint32_t color) {
    if (!graphics_state.back_buffer) return;
    
    // Simple text rendering - each character is 8x8 pixels
    int char_width = 8;
    int char_height = 8;
    
    for (int i = 0; text[i] != '\0'; i++) {
        char c = text[i];
        int char_x = x + i * char_width;
        
        // Simple font rendering - just draw rectangles for now
        // TODO: Implement proper font rendering
        if (c >= 'A' && c <= 'Z') {
            surface_fill(graphics_state.back_buffer, char_x, y, char_width, char_height, color);
        } else if (c >= 'a' && c <= 'z') {
            surface_fill(graphics_state.back_buffer, char_x, y + 2, char_width, char_height - 2, color);
        } else if (c >= '0' && c <= '9') {
            surface_fill(graphics_state.back_buffer, char_x + 1, y + 1, char_width - 2, char_height - 2, color);
        } else if (c == ' ') {
            // Skip spaces
        } else {
            // Unknown character - draw a dot
            surface_set_pixel(graphics_state.back_buffer, char_x + 4, y + 4, color);
        }
    }
}

// Graphics server API functions for IPC
int graphics_server_handle_request(struct ipc_message *msg) {
    if (!msg) return -1;
    
    struct graphics_command cmd;
    
    switch (msg->type) {
        case IPC_GRAPHICS_CLEAR:
            cmd.type = GRAPHICS_CMD_CLEAR;
            cmd.clear.color = msg->data.graphics_clear.color;
            return graphics_server_queue_command(&cmd);
            
        case IPC_GRAPHICS_DRAW_PIXEL:
            cmd.type = GRAPHICS_CMD_DRAW_PIXEL;
            cmd.pixel.x = msg->data.graphics_pixel.x;
            cmd.pixel.y = msg->data.graphics_pixel.y;
            cmd.pixel.color = msg->data.graphics_pixel.color;
            return graphics_server_queue_command(&cmd);
            
        case IPC_GRAPHICS_DRAW_RECT:
            cmd.type = GRAPHICS_CMD_DRAW_RECT;
            cmd.rect.x = msg->data.graphics_rect.x;
            cmd.rect.y = msg->data.graphics_rect.y;
            cmd.rect.width = msg->data.graphics_rect.width;
            cmd.rect.height = msg->data.graphics_rect.height;
            cmd.rect.color = msg->data.graphics_rect.color;
            return graphics_server_queue_command(&cmd);
            
        case IPC_GRAPHICS_DRAW_CIRCLE:
            cmd.type = GRAPHICS_CMD_DRAW_CIRCLE;
            cmd.circle.x = msg->data.graphics_circle.x;
            cmd.circle.y = msg->data.graphics_circle.y;
            cmd.circle.radius = msg->data.graphics_circle.radius;
            cmd.circle.color = msg->data.graphics_circle.color;
            return graphics_server_queue_command(&cmd);
            
        case IPC_GRAPHICS_DRAW_TEXT:
            cmd.type = GRAPHICS_CMD_DRAW_TEXT;
            cmd.text.x = msg->data.graphics_text.x;
            cmd.text.y = msg->data.graphics_text.y;
            strcpy(cmd.text.text, msg->data.graphics_text.text);
            cmd.text.color = msg->data.graphics_text.color;
            return graphics_server_queue_command(&cmd);
            
        default:
            debug_print("Unknown graphics IPC message type: %d\n", msg->type);
            return -1;
    }
}

void graphics_server_shutdown(void) {
    debug_print("Shutting down graphics server\n");
    
    graphics_state.initialized = false;
    
    if (graphics_state.back_buffer) {
        surface_destroy(graphics_state.back_buffer);
        graphics_state.back_buffer = NULL;
    }
    
    debug_print("Graphics server shutdown complete\n");
}
