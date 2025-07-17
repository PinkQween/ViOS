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

// List of user programs to load
#define NUM_PROGRAMS 4
    const char *program_paths[NUM_PROGRAMS] = {
        "0:/etc/default/user/programs/asm_test/asm_test.elf",
        "0:/etc/default/user/programs/cpp_prnt/cpp_prnt.elf",
        "0:/etc/default/user/programs/cpp_test/cpp_test.elf",
        "0:/etc/default/user/programs/c_print/c_print.elf"};
    struct process *processes[NUM_PROGRAMS] = {0};

    for (int i = 0; i < NUM_PROGRAMS; ++i)
    {
        int res = process_load_switch(program_paths[i], &processes[i]);
        if (res != VIOS_ALL_OK)
        {
            panic("Failed to load user program\n");
        }
        struct command_argument argument;
        strcpy(argument.argument, "Testing!");
        argument.next = 0x00;
        process_inject_arguments(processes[i], &argument);
    }

    // Debug: Print all tasks in the task list
    simple_serial_puts("DEBUG: Task list after loading user programs:\n");
    extern struct task *task_head;
    struct task *t = task_head;
    int task_count = 0;
    while (t)
    {
        simple_serial_puts("  Task at: ");
        print_hex32((uint32_t)t);
        simple_serial_puts(", process id: ");
        if (t->process)
        {
            print_hex32((uint32_t)t->process->id);
        }
        else
        {
            simple_serial_puts("(null)");
        }
        simple_serial_puts("\n");
        t = t->next;
        task_count++;
        if (task_count > 10)
            break; // Prevent infinite loop if list is corrupted
    }

    // Ensure timer IRQ is unmasked and interrupts are enabled for preemptive multitasking
    extern void kernel_unmask_timer_irq(void);
    extern void enable_interrupts(void);
    kernel_unmask_timer_irq();
    enable_interrupts();

    task_run_first_ever_task();

    // Fallback: if no program loaded or task execution failed, run graphics loop

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
