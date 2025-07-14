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
    simple_serial_puts("DEBUG: ===== ENTERING MAIN LOOP =====\n");
    simple_serial_puts("DEBUG: Entering main loop\n");

    struct process *process = 0;
    int res = process_load_switch("0:/cpp_test.elf", &process);
    if (res != VIOS_ALL_OK)
    {
        panic("Failed to load blank.elf\n");
    }

    struct command_argument argument;
    strcpy(argument.argument, "Testing!");
    argument.next = 0x00;

    process_inject_arguments(process, &argument);

    res = process_load_switch("0:/blank_asm.elf", &process);
    if (res != VIOS_ALL_OK)
    {
        panic("Failed to load blank_asm.elf\n");
    }

    task_run_first_ever_task();

    // Fallback: if no program loaded or task execution failed, run graphics loop
    simple_serial_puts("DEBUG: Running graphics fallback loop\n");

    FrameState frame_state;
    renderer_init_frame_state(&frame_state);
    graphics_set_unlimited_fps();
    extern uint32_t _graphics_get_time_ms(void);
    frame_state.last_time = _graphics_get_time_ms();

    while (1)
    {
        graphics_begin_frame();

        GraphicsContext *ctx = graphics_get_context();
        if (ctx && ctx->needs_full_refresh)
        {
            ClearScreen(11, 25, 69);
            frame_state.prev_mouse_x = frame_state.prev_mouse_y = -1;
            ctx->needs_full_refresh = false;
        }

        renderer_update_frame_info(&frame_state);
        renderer_update_actual_fps(&frame_state);
        renderer_get_resolution_string(frame_state.resolution_info, sizeof(frame_state.resolution_info));
        renderer_draw_info_overlay(&frame_state);
        renderer_draw_animated_rects(&frame_state);
        renderer_update_mouse(&frame_state, mouse);

        frame_state.animation_counter++;
        graphics_end_frame();
        graphics_present();
    }
}
