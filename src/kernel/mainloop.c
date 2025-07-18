#include "mainloop.h"
#include "drivers/input/mouse/mouse.h"
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
    // int res = process_load_switch("0:/sbin/reloivd", &process);
    int res = process_load_switch("0:/bin/beep", &process);

    if (res != VIOS_ALL_OK)
    {
        panic("Failed to load reloivd.\n");
    }

    task_run_first_ever_task();

    while (1)
    {
    }
}
