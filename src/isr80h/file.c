#include "file.h"
#include "fs/file.h"
#include "status.h"
#include "kernel.h"
#include "task/task.h"
#include "memory/heap/kheap.h"
#include "config.h"

#define MAX_PATH_LEN 256

static int copy_path_from_task_stack(struct task *task, int index, char *kernel_buf, int max_len)
{
    void *user_ptr = task_get_stack_item(task, index);
    if (!user_ptr)
    {
        return -EINVARG;
    }

    task_page_task(task);

    for (int i = 0; i < max_len - 1; i++)
    {
        char c = *((char *)user_ptr + i);
        kernel_buf[i] = c;
        if (c == '\0')
            break;
    }

    kernel_page();

    if (kernel_buf[max_len - 1] != '\0' && kernel_buf[max_len - 2] != '\0')
    {
        return -EINVARG;
    }

    return 0;
}

void *isr80h_command_read(struct interrupt_frame *frame)
{
    char path[MAX_PATH_LEN] = {0};

    int res = copy_path_from_task_stack(task_current(), 0, path, MAX_PATH_LEN);
    if (res < 0)
    {
        return ERROR(res);
    }

    int fd = fopen(path, "r");
    if (fd < 0)
    {
        return ERROR(-EIO);
    }

    struct file_stat stat;
    if (fstat(fd, &stat) != 0)
    {
        fclose(fd);
        return ERROR(-EIO);
    }

    uint32_t filesize = stat.filesize;

    char *buffer = kmalloc(filesize + 1);
    if (!buffer)
    {
        fclose(fd);
        return ERROR(-ENOMEM);
    }

    int read_items = fread(buffer, 1, filesize, fd);
    if (read_items != (int)filesize)
    {
        kfree(buffer);
        fclose(fd);
        return ERROR(-EIO);
    }

    buffer[read_items] = '\0';
    fclose(fd);

    return buffer;
}