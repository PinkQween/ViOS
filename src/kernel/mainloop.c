#include "mainloop.h"
#include "drivers/input/mouse/mouse.h"
#include "status.h"
#include "task/process.h"
#include "task/task.h"
#include "panic/panic.h"
#include "string/string.h"
#include "debug/simple_serial.h"
#include "drivers/output/vigfx/vigfx.h"
#include "utils/utils.h"
#include "memory/heap/kheap.h"

void kernel_run_main_loop()
{
    simple_serial_puts("DEBUG: ===== ENTERING MAIN LOOP =====\n");
    simple_serial_puts("DEBUG: Entering main loop\n");

    // struct process *process = 0;
    // int res = process_load_switch("0:/sbin/idle", &process);
    // if (res != VIOS_ALL_OK)
    // {
    //     simple_serial_puts("DEBUG: Failed to load idle process, continuing without it\n");
    // }
    // else
    // {
    //     simple_serial_puts("DEBUG: Successfully loaded idle process\n");
    // }

    // res = process_load_switch("0:/bin/beep", &process); // TODO: update to ("0:/sbin/reloivd", &process);
    // if (res != VIOS_ALL_OK)
    // {
    //     simple_serial_puts("DEBUG: Failed to load beep process, continuing without it\n");
    // }
    // else
    // {
    //     simple_serial_puts("DEBUG: Successfully loaded beep process\n");
    // }

    // task_run_first_ever_task();

    // Load background image once at startup

    global_mouse->set_speed(2);

    DRAW_CALLBACK bg_draw_func = NULL;
    void *bg_draw_ctx = NULL;

    int result = getFuncToDrawScaledRGBFill("0:/sys/assets/logo.rgb", 0, 0, gpu_screen_width(), gpu_screen_height(), &bg_draw_func, &bg_draw_ctx);

    if (result != 0)
    {
        simple_serial_puts("Failed to load background.\n");
    }

    // Draw background once initially
    bg_draw_func(bg_draw_ctx);
    
    // Track previous mouse position to detect movement
    int prev_mouse_x = -1;
    int prev_mouse_y = -1;
    
    // Buffer to store background pixels under cursor (11x11 max area)
    struct {
        uint8_t r, g, b;
    } cursor_bg_buffer[11][11];
    int buffer_valid = 0;
    
    while (1)
    {
        int needs_redraw = 0;
        
        // Check if mouse position has changed
        if (global_mouse)
        {
            if (prev_mouse_x != global_mouse->x || prev_mouse_y != global_mouse->y)
            {
                needs_redraw = 1;
                
                // Restore old cursor area using saved background pixels
                if (prev_mouse_x >= 0 && prev_mouse_y >= 0 && buffer_valid)
                {
                    for (int y = 0; y <= 10 && prev_mouse_y + y < gpu_screen_height(); y++)
                    {
                        for (int x = 0; x <= 10 - y && prev_mouse_x + x < gpu_screen_width(); x++)
                        {
                            gpu_draw(prev_mouse_x + x, prev_mouse_y + y, 
                                    cursor_bg_buffer[y][x].r,
                                    cursor_bg_buffer[y][x].g,
                                    cursor_bg_buffer[y][x].b);
                        }
                    }
                }
                
                // Save background pixels under new cursor position
                for (int y = 0; y <= 10 && global_mouse->y + y < gpu_screen_height(); y++)
                {
                    for (int x = 0; x <= 10 - y && global_mouse->x + x < gpu_screen_width(); x++)
                    {
                        // Get the current pixel color at this position
                        uint32_t pixel = gpu_get_pixel(global_mouse->x + x, global_mouse->y + y);
                        cursor_bg_buffer[y][x].r = (pixel >> 16) & 0xFF;
                        cursor_bg_buffer[y][x].g = (pixel >> 8) & 0xFF;
                        cursor_bg_buffer[y][x].b = pixel & 0xFF;
                    }
                }
                buffer_valid = 1;
                
                // Update mouse position tracking
                prev_mouse_x = global_mouse->x;
                prev_mouse_y = global_mouse->y;
                
                // Draw new mouse cursor
                for (int y = 0; y <= 10 && global_mouse->y + y < gpu_screen_height(); y++)
                {
                    for (int x = 0; x <= 10 - y && global_mouse->x + x < gpu_screen_width(); x++)
                    {
                        gpu_draw(global_mouse->x + x, global_mouse->y + y, 255, 255, 255);
                    }
                }
            }
        }
        
        // Only flush if something changed
        if (needs_redraw)
        {
            gpu_flush_screen();
        }
    }
}
