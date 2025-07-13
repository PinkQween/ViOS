task_get_stack_item
===================

**Prototype:**

```c
void* task_get_stack_item(struct task* task, int index);
```

**Type:** `Internal Kernel API`

Description
-----------

Retrieves a parameter from a task's stack at the specified index. This function is used by system call handlers to access parameters passed by user-space programs. It safely switches to the task's memory space, reads the stack item, and returns to kernel space, handling the virtual memory complexities automatically.

Parameters
----------

*   `struct task* task` — Pointer to the task whose stack to read from
*   `int index` — Zero-based index of the stack item to retrieve

Returns
-------

Returns the value at the specified stack index as a void pointer.

Notes
-----

- Used extensively by system call handlers to access user-provided parameters
- Index 0 corresponds to the first parameter, index 1 to the second, etc.
- Automatically handles memory space switching to access the user stack
- The function switches to the task's page directory temporarily
- Returns to kernel page directory after reading the stack item
- Essential for implementing system calls that take multiple parameters
- The returned value is typically cast to the appropriate type by the caller
- Stack items are accessed relative to the task's ESP register value
