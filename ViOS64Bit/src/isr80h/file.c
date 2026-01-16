#include "file.h"
#include "task/task.h"
#include "task/process.h"
#include "idt/idt.h"
#include <stddef.h>
#include <stdint.h>
#include "kernel.h"

void* isr80h_command14_fstat(struct interrupt_frame* frame)
{
    long fd = (long) task_get_stack_item(task_current(), 0);
    struct file_stat* virt_file_stat_addr = (struct file_stat*) task_get_stack_item(task_current(), 1);
    return (void*)(long) process_fstat(task_current()->process, fd, virt_file_stat_addr);
}

void* isr80h_command13_fseek(struct interrupt_frame* frame)
{ 
    long fd = (long) task_get_stack_item(task_current(), 0);
    long offset = (long) task_get_stack_item(task_current(), 1);
    long whence = (long) task_get_stack_item(task_current(), 2);

    return (void*) (long) process_fseek(task_current()->process, fd, offset, whence);
}

void* isr80h_command12_fread(struct interrupt_frame* frame)
{
    int res = 0;
    void* buffer_virt_addr = task_get_stack_item(task_current(), 0);
    size_t size = (size_t) task_get_stack_item(task_current(), 1);
    size_t count = (size_t) task_get_stack_item(task_current(), 2);

    long fd = (long) task_get_stack_item(task_current(), 3);
    res = process_fread(task_current()->process, buffer_virt_addr, size, count, fd);
    return (void*) (int64_t) res;
}

void* isr80h_command11_fclose(struct interrupt_frame* frame)
{
    int64_t fd = 0;
    fd = (int64_t) task_get_stack_item(task_current(), 0);

    // We have the file number lets close it
    process_fclose(task_current()->process, fd);

    // return null
    return NULL;
}

void* isr80h_command10_fopen(struct interrupt_frame* frame)
{
    int fd = 0;
    void* filename_virt_addr = NULL;
    void* mode_virt_addr = NULL;
    kernel_debug_log("[fopen] start\n");
    filename_virt_addr = task_get_stack_item(task_current(), 0);
    kernel_debug_log("[fopen] got filename virt\n");
    filename_virt_addr = task_virtual_address_to_physical(task_current(), filename_virt_addr);
    kernel_debug_log("[fopen] translated filename\n");
    if (!filename_virt_addr)
    {
        kernel_debug_log("[fopen] filename translation failed\n");
        fd = -1;
        goto out;
    }

    mode_virt_addr = task_get_stack_item(task_current(), 1);
    kernel_debug_log("[fopen] got mode virt\n");
    mode_virt_addr = task_virtual_address_to_physical(task_current(), mode_virt_addr);
    kernel_debug_log("[fopen] translated mode\n");
    if (!mode_virt_addr)
    {
        kernel_debug_log("[fopen] mode translation failed\n");
        fd = -1;
        goto out;
    }

    kernel_debug_log("[fopen] calling process_fopen\n");
    fd = process_fopen(task_current()->process, filename_virt_addr, mode_virt_addr);
    kernel_debug_log("[fopen] process_fopen returned\n");
    if (fd <= 0)
    {
        goto out;
    }

out:
    kernel_debug_log("[fopen] done\n");
    return (void*) (int64_t) fd;
}

void* isr80h_command16_fwrite(struct interrupt_frame* frame)
{
    int res = 0;
    kernel_debug_log("[fwrite] start\n");
    const void* buffer_virt_addr = task_get_stack_item(task_current(), 0);
    size_t size = (size_t) task_get_stack_item(task_current(), 1);
    size_t count = (size_t) task_get_stack_item(task_current(), 2);

    long fd = (long) task_get_stack_item(task_current(), 3);
    kernel_debug_log("[fwrite] calling process_fwrite\n");
    res = process_fwrite(task_current()->process, buffer_virt_addr, size, count, fd);
    kernel_debug_log("[fwrite] done\n");
    return (void*) (int64_t) res;
}