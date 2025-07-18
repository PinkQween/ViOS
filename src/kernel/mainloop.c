#include "mainloop.h"
#include "vigfx/vigfx.h"
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
    int res = process_load_switch("0:/sbin/reloivd", &process);

    if (res != VIOS_ALL_OK)
    {
        panic("Failed to load reloivd.\n");
    }

    task_run_first_ever_task();

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

    // Fallback: if no program loaded or task execution failed, run ViGFX loop
    simple_serial_puts("DEBUG: Starting VirGFX rendering loop\n");
    
    struct vigfx_context *ctx = vigfx_create_context(NULL);
    if (!ctx) {
        simple_serial_puts("DEBUG: Failed to create VirGFX context\n");
        return;
    }
    
    while (1)
    {
        // TODO: Submit real GPU commands for raytracing, etc.
        vigfx_present(ctx, NULL);
    }
}
