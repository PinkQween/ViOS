task_switch
===========

**Prototype:**

```c
int task_switch(struct task* task);
```

**Type:** `Internal Kernel API`

Description
-----------

Switches the CPU context to execute the specified task. This function performs a task context switch by updating the current task pointer and switching to the task's page directory. This is a low-level operation used by the kernel scheduler to implement multitasking.

Parameters
----------

*   `struct task* task` — Pointer to the task to switch to

Returns
-------

Returns 0 on success.

Notes
-----

- Updates the global `current_task` pointer to the new task
- Switches to the task's page directory using `paging_switch`
- This function does not save/restore CPU registers - that is handled separately
- Used internally by the scheduler during context switches
- The task must have been previously created with `task_new`
- After this call, memory accesses use the new task's virtual memory space
- This function is typically called with interrupts disabled
- The actual CPU register switching is handled by `task_return` assembly code
