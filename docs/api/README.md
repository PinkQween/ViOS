# ViOS Kernel API Documentation

This directory contains documentation for all kernel-level internal APIs and system calls in the ViOS operating system.

## System Calls

System calls provide the interface between user-space programs and the kernel. They are invoked using the `int 0x80` instruction with the system call number in the EAX register.

### Core System Calls
- [sys_exit](./sys_exit.md) - Terminate the current process
- [sys_print](./sys_print.md) - Print text to the screen
- [sys_getkey](./sys_getkey.md) - Get keyboard input
- [sys_malloc](./sys_malloc.md) - Allocate memory
- [sys_free](./sys_free.md) - Free allocated memory
- [sys_sleep](./sys_sleep.md) - Sleep for specified seconds

### VIX Graphics System Calls
- [vix_draw_pixel](./vix_draw_pixel.md) - Draw a single pixel
- [vix_clear_screen](./vix_clear_screen.md) - Clear the screen
- [vix_present_frame](./vix_present_frame.md) - Present the framebuffer

## Internal Kernel APIs

These functions are used internally by the kernel and should not be called from user-space programs.

### Memory Management
- [kmalloc](./kmalloc.md) - Allocate kernel memory
- [kzalloc](./kzalloc.md) - Allocate zero-initialized kernel memory
- [kfree](./kfree.md) - Free kernel memory
- [kheap_init](./kheap_init.md) - Initialize the kernel heap system
- [heap_create](./heap_create.md) - Create and initialize a heap structure
- [heap_malloc](./heap_malloc.md) - Allocate memory from a specific heap
- [heap_free](./heap_free.md) - Free memory from a specific heap

### Virtual Memory & Paging
- [paging_new_4gb](./paging_new_4gb.md) - Create a new 4GB page directory
- [paging_switch](./paging_switch.md) - Switch to a different page directory

### Task Management
- [task_new](./task_new.md) - Create a new task
- [task_switch](./task_switch.md) - Switch to a different task
- [task_current](./task_current.md) - Get the current running task

### Process Management
- [process_load](./process_load.md) - Load a program into memory
- [process_switch](./process_switch.md) - Switch to a different process

### Interrupt Management
- [idt_init](./idt_init.md) - Initialize the Interrupt Descriptor Table
- [enable_interrupts](./enable_interrupts.md) - Enable hardware interrupts
- [disable_interrupts](./disable_interrupts.md) - Disable hardware interrupts
- [isr80h_register_command](./isr80h_register_command.md) - Register a system call handler

### System Call Utilities
- [copy_string_from_task](./copy_string_from_task.md) - Safely copy strings from user space
- [task_get_stack_item](./task_get_stack_item.md) - Get system call parameters from user stack

## System Call Numbers

| System Call | Number | Description |
|-------------|--------|-------------|
| sys_exit | 0 | Terminate process |
| sys_print | 1 | Print text to screen |
| sys_getkey | 2 | Get keyboard input |
| sys_putchar | 3 | Print single character |
| sys_malloc | 4 | Allocate memory |
| sys_free | 5 | Free memory |
| sys_process_load_start | 6 | Load and start process |
| sys_invoke_system_command | 7 | Invoke system command |
| sys_get_program_arguments | 8 | Get program arguments |
| sys_sleep | 9 | Sleep for seconds |
| sys_read | 10 | Read from file |
| vix_draw_pixel | 11 | Draw pixel |
| vix_draw_rect | 12 | Draw rectangle |
| vix_fill_rect | 13 | Fill rectangle |
| vix_clear_screen | 14 | Clear screen |
| vix_present_frame | 15 | Present framebuffer |
| vix_get_screen_info | 16 | Get screen information |
| vix_draw_line | 17 | Draw line |
| vix_draw_circle | 18 | Draw circle |
| vix_fill_circle | 19 | Fill circle |

## Color Macros

The VIX graphics system provides these color utility macros:

```c
#define VIX_RGB(r, g, b) (((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b))
#define VIX_COLOR_BLACK VIX_RGB(0, 0, 0)
#define VIX_COLOR_WHITE VIX_RGB(255, 255, 255)
#define VIX_COLOR_RED VIX_RGB(255, 0, 0)
#define VIX_COLOR_GREEN VIX_RGB(0, 255, 0)
#define VIX_COLOR_BLUE VIX_RGB(0, 0, 255)
#define VIX_COLOR_YELLOW VIX_RGB(255, 255, 0)
#define VIX_COLOR_CYAN VIX_RGB(0, 255, 255)
#define VIX_COLOR_MAGENTA VIX_RGB(255, 0, 255)
```

## Notes

- All system calls use the `int 0x80` software interrupt mechanism
- System call parameters are passed via the stack
- Error codes are typically negative values
- User-space programs should use the stdlib wrapper functions rather than invoking system calls directly
- The VIX graphics API uses double-buffering - call `vix_present_frame` to make drawing visible
