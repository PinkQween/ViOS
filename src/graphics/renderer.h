#ifndef GRAPHICS_RENDERER_H
#define GRAPHICS_RENDERER_H

#include <stdint.h>
#include <stdbool.h>

struct mouse;

/**
 * Frame information display structure
 */
typedef struct {
    char frame_info[64];
    char actual_fps_info[64];
    char resolution_info[64];
    
    uint32_t last_time;
    uint32_t frame_counter;
    uint32_t actual_fps;
    
    int animation_counter;
    int prev_mouse_x, prev_mouse_y;
    int prev_red_rect_x, prev_green_rect_size;
} FrameState;

/**
 * Initialize the frame state
 * @param state Pointer to frame state structure to initialize
 */
void renderer_init_frame_state(FrameState *state);

/**
 * Update frame information strings
 * @param state Pointer to frame state structure
 */
void renderer_update_frame_info(FrameState *state);

/**
 * Calculate and update actual FPS
 * @param state Pointer to frame state structure
 */
void renderer_update_actual_fps(FrameState *state);

/**
 * Draw animated rectangles
 * @param state Pointer to frame state structure
 */
void renderer_draw_animated_rects(FrameState *state);

/**
 * Update and draw mouse cursor
 * @param state Pointer to frame state structure
 * @param mouse Pointer to mouse interface
 */
void renderer_update_mouse(FrameState *state, struct mouse *mouse);

/**
 * Render frame information overlay
 * @param state Pointer to frame state structure
 */
void renderer_draw_info_overlay(FrameState *state);

/**
 * Get current screen resolution as formatted string
 * @param buffer Buffer to store resolution string
 * @param buffer_size Size of the buffer
 */
void renderer_get_resolution_string(char *buffer, int buffer_size);

#endif // GRAPHICS_RENDERER_H
