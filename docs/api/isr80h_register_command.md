isr80h_register_command
=======================

**Prototype:**

```c
void isr80h_register_command(int command_id, ISR80H_COMMAND command);
```

**Type:** `Internal Kernel API`

Description
-----------

Registers a system call handler function for a specific system call number. This function maps system call numbers to their corresponding handler functions in the kernel's system call table. When a user process invokes a system call via `int 0x80`, the kernel uses this table to dispatch to the appropriate handler.

Parameters
----------

*   `int command_id` — System call number (from `enum SystemCommands`)
*   `ISR80H_COMMAND command` — Function pointer to the system call handler

Returns
-------

None (void function).

Notes
-----

- The `command_id` must be a valid system call number from `enum SystemCommands`
- The `command` parameter is a function pointer with signature `void* (*)(struct interrupt_frame*)`
- This function is called during kernel initialization to set up the system call table
- Each system call number can only have one registered handler
- The handler function receives the interrupt frame containing CPU state and parameters
- System call parameters are accessed via the task's stack using `task_get_stack_item`
- Handler functions should return a value that gets placed in the EAX register
- Used internally by `isr80h_register_commands` to set up all system calls
