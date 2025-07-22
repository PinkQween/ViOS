#include "task.h"
#include "debug/simple_serial.h"
#include "drivers/io/timing/timer.h"

// The current task that is running
struct task *current_task = 0;

// Task linked list
struct task *task_tail = 0;
struct task *task_head = 0;

int task_init(struct task *task, struct process *process);

struct task *task_current()
{
    return current_task;
}

struct task *task_new(struct process *process)
{
    int res = 0;
    struct task *task = kzalloc(sizeof(struct task));
    if (!task)
    {
        res = -ENOMEM;
        goto out;
    }

    res = task_init(task, process);
    if (res != VIOS_ALL_OK)
    {
        goto out;
    }

    if (task_head == 0)
    {
        task_head = task;
        task_tail = task;
        current_task = task;
        goto out;
    }

    task_tail->next = task;
    task->prev = task_tail;
    task_tail = task;

out:
    if (ISERR(res))
    {
        task_free(task);
        return ERROR(res);
    }

    return task;
}

struct task *task_get_next()
{
    struct task *start = current_task;
    struct task *next = current_task->next ? current_task->next : task_head;
    while (next != start)
    {
        if (!next->sleeping)
        {
            return next;
        }
        next = next->next ? next->next : task_head;
    }
    // If only one task is not sleeping, return it
    if (!start->sleeping)
        return start;
    // All tasks are sleeping
    return NULL;
}

static void task_list_remove(struct task *task)
{
    if (task->prev)
    {
        task->prev->next = task->next;
    }

    if (task == task_head)
    {
        task_head = task->next;
    }

    if (task == task_tail)
    {
        task_tail = task->prev;
    }

    if (task == current_task)
    {
        current_task = task_get_next();
    }
}

int task_free(struct task *task)
{
    paging_free_4gb(task->page_directory);
    task_list_remove(task);

    // Finally free the task data
    kfree(task);
    return 0;
}

void task_next()
{
    struct task *next_task = task_get_next();
    if (!next_task)
    {
        panic("No more tasks!\n");
    }

    task_switch(next_task);
    task_return(&next_task->registers);
}

int task_switch(struct task *task)
{
    current_task = task;
    paging_switch(task->page_directory);
    return 0;
}

void task_save_state(struct task *task, struct interrupt_frame *frame)
{
    task->registers.ip = frame->ip;
    task->registers.cs = frame->cs;
    task->registers.flags = frame->flags;
    task->registers.esp = frame->esp;
    task->registers.ss = frame->ss;
    task->registers.eax = frame->eax;
    task->registers.ebp = frame->ebp;
    task->registers.ebx = frame->ebx;
    task->registers.ecx = frame->ecx;
    task->registers.edi = frame->edi;
    task->registers.edx = frame->edx;
    task->registers.esi = frame->esi;
}
int copy_string_from_task(struct task *task, void *virtual, void *phys, int max)
{
    if (max >= PAGING_PAGE_SIZE)
    {
        return -EINVARG;
    }

    int res = 0;
    char *tmp = kzalloc(max);
    if (!tmp)
    {
        res = -ENOMEM;
        goto out;
    }

    uint32_t *task_directory = task->page_directory->directory_entry;
    uint32_t old_entry = paging_get(task_directory, tmp);
    paging_map(task->page_directory, tmp, tmp, PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    paging_switch(task->page_directory);
    strncpy(tmp, virtual, max);
    kernel_page();

    res = paging_set(task_directory, tmp, old_entry);
    if (res < 0)
    {
        res = -EIO;
        goto out_free;
    }

    strncpy(phys, tmp, max);

out_free:
    kfree(tmp);
out:
    return res;
}
void task_current_save_state(struct interrupt_frame *frame)
{
    if (!task_current())
    {
        panic("No current task to save\n");
    }

    struct task *task = task_current();
    task_save_state(task, frame);
}

int task_page()
{
    user_registers();
    task_switch(current_task);
    return 0;
}

int task_page_task(struct task *task)
{
    user_registers();
    paging_switch(task->page_directory);
    return 0;
}

void task_run_first_ever_task()
{
    if (!current_task)
    {
        panic("task_run_first_ever_task(): No current task exists!\n");
    }

    simple_serial_puts("DEBUG: task_run_first_ever_task: About to switch to task_head\n");
    simple_serial_puts("DEBUG: task_run_first_ever_task: task_head->registers.ip = ");
    print_hex32(task_head->registers.ip);
    simple_serial_puts("\n");
    simple_serial_puts("DEBUG: task_run_first_ever_task: task_head->registers.cs = ");
    print_hex32(task_head->registers.cs);
    simple_serial_puts("\n");
    simple_serial_puts("DEBUG: task_run_first_ever_task: task_head->registers.ss = ");
    print_hex32(task_head->registers.ss);
    simple_serial_puts("\n");
    simple_serial_puts("DEBUG: task_run_first_ever_task: task_head->registers.esp = ");
    print_hex32(task_head->registers.esp);
    simple_serial_puts("\n");

    task_switch(task_head);
    simple_serial_puts("DEBUG: task_run_first_ever_task: About to call task_return\n");
    task_return(&task_head->registers);
    simple_serial_puts("DEBUG: task_run_first_ever_task: task_return returned (this shouldn't happen)\n");
}

int task_init(struct task *task, struct process *process)
{
    memset(task, 0, sizeof(struct task));
    // Map the entire 4GB address space to its self
    task->page_directory = paging_new_4gb(PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    if (!task->page_directory)
    {
        return -EIO;
    }

    task->registers.ip = VIOS_PROGRAM_VIRTUAL_ADDRESS;
    if (process->filetype == PROCESS_FILETYPE_ELF)
    {
        task->registers.ip = elf_header(process->elf_file)->e_entry;
    }

    task->registers.ss = USER_DATA_SEGMENT;
    task->registers.cs = USER_CODE_SEGMENT;
    task->registers.esp = VIOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START;

    simple_serial_puts("DEBUG: task_init: Setting registers\n");
    simple_serial_puts("DEBUG: task_init: ip = ");
    print_hex32(task->registers.ip);
    simple_serial_puts("\n");
    simple_serial_puts("DEBUG: task_init: cs = ");
    print_hex32(task->registers.cs);
    simple_serial_puts("\n");
    simple_serial_puts("DEBUG: task_init: ss = ");
    print_hex32(task->registers.ss);
    simple_serial_puts("\n");
    simple_serial_puts("DEBUG: task_init: esp = ");
    print_hex32(task->registers.esp);
    simple_serial_puts("\n");

    task->process = process;

    return 0;
}

void *task_get_stack_item(struct task *task, int index)
{
    void *result = 0;

    uint32_t *sp_ptr = (uint32_t *)task->registers.esp;

    // Switch to the given tasks page
    task_page_task(task);

    result = (void *)sp_ptr[index];

    // Switch back to the kernel page
    kernel_page();

    return result;
}

void *task_virtual_address_to_physical(struct task *task, void *virtual_address)
{
    return paging_get_physical_address(task->page_directory->directory_entry, virtual_address);
}

void task_scheduler_tick()
{
    // If no tasks exist yet, just return
    if (!task_head)
    {
        return;
    }
    
    // Wake up any sleeping tasks whose wakeup_tick has passed
    struct task *t = task_head;
    unsigned long now = timer_get_ticks();
    do
    {
        if (t->sleeping && t->wakeup_tick <= now)
        {
            t->sleeping = 0;
        }
        t = t->next;
    } while (t && t != task_head);
    
    // Only switch if there is a runnable task
    struct task *next = task_get_next();
    if (next && next != current_task)
    {
        task_next();
    }
}
