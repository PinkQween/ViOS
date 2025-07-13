process_switch
==============

**Prototype:**

```c
int process_switch(struct process* process);
```

**Type:** `Internal Kernel API`

Description
-----------

Switches execution to the specified process, making it the currently running process. This function performs a context switch, saving the current processor state and loading the state of the target process. The process must have been previously loaded using `process_load`.

Parameters
----------

*   `struct process* process` — Pointer to the process to switch to

Returns
-------

Returns 0 on success, or a negative error code on failure.

Notes
-----

- This function is for kernel use only
- The process must have been loaded using `process_load`
- Performs a full context switch including CPU registers and memory mapping
- The current process is suspended and the target process becomes active
- This function is typically called by the scheduler
- May not return immediately if the process runs for an extended period
- Failure can occur if the process structure is invalid or corrupted
