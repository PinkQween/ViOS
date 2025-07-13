copy_string_from_task
=====================

**Prototype:**

```c
int copy_string_from_task(struct task* task, void* virtual, void* phys, int max);
```

**Type:** `Internal Kernel API`

Description
-----------

Safely copies a string from user space to kernel space. This function handles the complex task of copying data between different memory spaces by temporarily mapping the user space address into kernel space, performing the copy operation, and then restoring the original memory mapping. This is essential for system calls that need to access user-provided string data.

Parameters
----------

*   `struct task* task` — Pointer to the task whose memory space contains the string
*   `void* virtual` — Virtual address in the task's memory space where the string is located
*   `void* phys` — Physical address in kernel space where the string should be copied
*   `int max` — Maximum number of bytes to copy (must be less than `PAGING_PAGE_SIZE`)

Returns
-------

Returns 0 on success, or a negative error code on failure:
- `-EINVARG` if max is greater than or equal to `PAGING_PAGE_SIZE`
- `-ENOMEM` if temporary buffer allocation fails
- `-EIO` if page mapping operations fail

Notes
-----

- Used extensively by system call handlers to access user-provided string parameters
- Implements safe cross-memory-space copying with proper page mapping
- The maximum copy size is limited to less than one page (4KB)
- Temporarily switches to the task's page directory during the copy operation
- Restores the original kernel page directory after completion
- Essential for system calls like `sys_print` that take string parameters
- Handles the complexity of virtual memory address translation
