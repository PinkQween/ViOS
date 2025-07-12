#include "renderer.h"
#include "graphics.h"
#include "memory/memory.h"
#include "string/string.h"
#include "mouse/mouse.h"

static uint16_t get_x_resolution()
{
    VBEInfoBlock *VBE = (VBEInfoBlock *)VBEInfoAddress;
    return VBE->x_resolution;
}

static uint16_t get_y_resolution()
{
    VBEInfoBlock *VBE = (VBEInfoBlock *)VBEInfoAddress;
    return VBE->y_resolution;
}

void renderer_init_frame_state(FrameState *state)
{
    memset(state, 0, sizeof(FrameState));
    state->prev_mouse_x = state->prev_mouse_y = -1;
    state->prev_red_rect_x = state->prev_green_rect_size = -1;
    strcpy(state->frame_info, "Frame: ...");
    strcpy(state->actual_fps_info, "Actual FPS: ...");
    strcpy(state->resolution_info, "Resolution: ...");
}

void renderer_update_frame_info(FrameState *state)
{
    if (state->frame_counter++ % 30 != 0)
        return;

    uint32_t frame_count = graphics_get_frame_count();
    uint32_t current_fps = graphics_get_fps();
    uint32_t max_fps = graphics_get_max_fps();

    strcpy(state->frame_info, "Frame: ");

    char num_str[16];
    int_to_str((int)frame_count, num_str);
    strcat(state->frame_info, num_str);

    strcat(state->frame_info, " | FPS: ");
    int_to_str((int)current_fps, num_str);
    strcat(state->frame_info, num_str);

    strcat(state->frame_info, " | Max: ");
    if (max_fps == 0)
        strcat(state->frame_info, "UNLIMITED");
    else
    {
        int_to_str((int)max_fps, num_str);
        strcat(state->frame_info, num_str);
    }
}

void renderer_update_actual_fps(FrameState *state)
{
    uint32_t now = _graphics_get_time_ms();
    if (now - state->last_time >= 1000)
    {
        state->actual_fps = state->frame_counter;
        state->frame_counter = 0;
        state->last_time = now;
    }

    strcpy(state->actual_fps_info, "Actual FPS: ");
    char num_str[16];
    int_to_str((int)state->actual_fps, num_str);
    strcat(state->actual_fps_info, num_str);
}

void renderer_draw_animated_rects(FrameState *state)
{
    if (state->animation_counter % 2 != 0)
        return;

    int rect_x = 50 + ((state->animation_counter / 2) % 200);
    int size = 20 + ((state->animation_counter / 2) % 20);
    int rect_y = 80;

    // Clear previous red rectangle
    if (state->prev_red_rect_x != -1 && state->prev_red_rect_x != rect_x)
        DrawRect(state->prev_red_rect_x, rect_y, 50, 30, 11, 25, 69);

    // Draw current red rectangle
    DrawRect(rect_x, rect_y, 50, 30, 255, 100, 100);
    state->prev_red_rect_x = rect_x;

    // Clear previous green rectangle (fixed position)
    if (state->prev_green_rect_size != -1 && state->prev_green_rect_size != size)
        DrawRect(400, 150, state->prev_green_rect_size, state->prev_green_rect_size, 11, 25, 69);

    // Draw current green rectangle
    DrawRect(400, 150, size, size, 100, 255, 100);
    state->prev_green_rect_size = size;
}

void renderer_update_mouse(FrameState *state, struct mouse *mouse)
{
    int x = mouse->x, y = mouse->y;

    if (state->prev_mouse_x != -1 && state->prev_mouse_y != -1 && (x != state->prev_mouse_x || y != state->prev_mouse_y))
        DrawRect(state->prev_mouse_x, state->prev_mouse_y, 12, 12, 11, 25, 69);

    DrawMouse(x, y, 255, 255, 255);

    state->prev_mouse_x = x;
    state->prev_mouse_y = y;
}

void renderer_draw_info_overlay(FrameState *state)
{
    DrawRect(10, 10, 300, 16, 11, 25, 69);
    DrawRect(10, 28, 200, 16, 11, 25, 69);
    DrawRect(10, 46, 200, 16, 11, 25, 69);

    DrawAtariString(state->frame_info, 10, 10, 255, 255, 255, 1);
    DrawAtariString(state->actual_fps_info, 10, 28, 255, 255, 0, 1);
    DrawAtariString(state->resolution_info, 10, 46, 0, 255, 255, 1);

    DrawAtariString("ViOS Enhanced Graphics System Running", 10, 64, 200, 200, 200, 1);
}

void renderer_get_resolution_string(char *buffer, int buffer_size)
{
    char num_str[16];

    strcpy(buffer, "Resolution: ");
    int_to_str((int)get_x_resolution(), num_str);
    strcat(buffer, num_str);
    strcat(buffer, "x");
    int_to_str((int)get_y_resolution(), num_str);
    strcat(buffer, num_str);
}
