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

void kernel_run_main_loop(struct mouse *mouse)
{
    simple_serial_puts("DEBUG: ===== ENTERING MAIN LOOP =====\n");
    simple_serial_puts("DEBUG: Entering main loop\n");

    struct process *process = 0;
    int res = process_load_switch("0:/sbin/idle", &process);
    if (res != VIOS_ALL_OK)
    {
        panic("Failed to load idle.\n");
    }

    res = process_load_switch("0:/bin/beep", &process); // TODO: update to ("0:/sbin/reloivd", &process);

    if (res != VIOS_ALL_OK)
    {
        panic("Failed to load reloivd.\n");
    }

    // task_run_first_ever_task();

    while (1)
    {
        draw_scaled_rgb_fill("0:/sys/assets/logo.rgb", 0, 0, gpu_screen_width(), gpu_screen_height());

        // Draw mouse cursor
        if (mouse)
        {
            int mouse_x = mouse->x;
            int mouse_y = mouse->y;

            simple_serial_puts("DEBUG: Mouse position: ");
            print_hex32(mouse_x);
            print_hex32(mouse_y);
            simple_serial_putc('\n');

            // Draw a simple square as the mouse cursor
            for (int y = 0; y <= 10; y++)
            {
                for (int x = 0; x <= 10 - y; x++)
                {
                    gpu_draw(mouse_x + x, mouse_y + y, 255, 255, 255);
                }
            }
        }

        gpu_flush_screen();
    }
}
