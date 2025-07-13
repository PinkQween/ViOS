task_new
========

**Prototype:**

```c
struct task* task_new(struct process* process);
```

**Type:** `Internal Kernel API`

Description
-----------

Creates a new task structure for the specified process. This function allocates and initializes a task control block, sets up the task's virtual memory space, initializes CPU registers, and adds the task to the kernel's task linked list. Each task represents an execution context that can be scheduled by the kernel.

Parameters
----------

*   `struct process* process` — Pointer to the process that owns this task

Returns
-------

Returns a pointer to the newly created task on success, or an error pointer on failure.

Notes
-----

- Allocates memory for the task structure using `kzalloc`
- Creates a new 4GB page directory for the task's virtual memory space
- Initializes CPU registers with appropriate values for user-mode execution
- Sets up the task's initial instruction pointer and stack pointer
- Adds the task to the global task linked list for scheduling
- The task becomes the current task if it's the first task created
- ELF processes have their entry point set from the ELF header
- Binary processes start execution at `VIOS_PROGRAM_VIRTUAL_ADDRESS`
- The task's stack is mapped to `VIOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START`
- Returns an error pointer if memory allocation or initialization fails
