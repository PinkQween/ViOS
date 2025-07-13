#include "mainloop.h"
#include "graphics/graphics.h"
#include "graphics/renderer.h"
#include "mouse/mouse.h"
#include "status.h"
#include "task/process.h"
#include "task/task.h"
#include "panic/panic.h"
#include "string/string.h"
#include "debug/simple_serial.h"

void kernel_run_main_loop(struct mouse *mouse)
{
    // struct process *process = 0;
    // int res = process_load_switch("0:/cpp_non-existant.elf", &process);
    // if (res != VIOS_ALL_OK)
    // {
    //     simple_serial_puts("Failed to load cpp_test.elf\n");
    // }

    // struct command_argument argument;
    // strcpy(argument.argument, "Testing!");
    // argument.next = 0x00;

    // process_inject_arguments(process, &argument);

    // task_run_first_ever_task();

    FrameState frame_state;
    renderer_init_frame_state(&frame_state);

    // Set unlimited FPS
    graphics_set_unlimited_fps();

    // Initialize timing
    extern uint32_t _graphics_get_time_ms(void);
    frame_state.last_time = _graphics_get_time_ms();

    while (1)
    {
        graphics_begin_frame();

        // Handle full screen refresh if needed
        GraphicsContext *ctx = graphics_get_context();
        if (ctx && ctx->needs_full_refresh)
        {
            ClearScreen(11, 25, 69);
            frame_state.prev_mouse_x = frame_state.prev_mouse_y = -1;
            ctx->needs_full_refresh = false;
        }

        // Update frame information
        renderer_update_frame_info(&frame_state);
        renderer_update_actual_fps(&frame_state);

        // Update resolution info
        renderer_get_resolution_string(frame_state.resolution_info,
                                       sizeof(frame_state.resolution_info));

        // Render all components
        renderer_draw_info_overlay(&frame_state);
        renderer_draw_animated_rects(&frame_state);
        renderer_update_mouse(&frame_state, mouse);

        frame_state.animation_counter++;

        graphics_end_frame();
        graphics_present();
    }
}
