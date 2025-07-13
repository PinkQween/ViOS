task_current
============

**Prototype:**

```c
struct task* task_current(void);
```

**Type:** `Internal Kernel API`

Description
-----------

Returns a pointer to the currently executing task. This function provides access to the task control block of the task that is currently running on the CPU. It is used throughout the kernel to access current task information such as process data, memory mappings, and register state.

Parameters
----------

None.

Returns
-------

Returns a pointer to the current task structure, or NULL if no task is currently running.

Notes
-----

- Returns the global `current_task` pointer
- May return NULL during early kernel initialization before any tasks are created
- This function is used extensively by system call handlers to access current task context
- The returned task pointer is valid until the next context switch
- Used by system calls to access the calling process's memory space and data structures
- The current task changes when the scheduler performs a context switch
- Thread-safe as it simply returns a global pointer value
- Critical for implementing per-process system call behavior
