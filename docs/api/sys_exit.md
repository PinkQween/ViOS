sys_exit
========

**Prototype:**

```c
void sys_exit(int exit_code);
```

**Type:** `System Call`

Description
-----------

Terminates the current process with the specified exit code. This system call causes the current process to exit immediately and returns control to the kernel. The exit code can be used by parent processes to determine the termination status.

Parameters
----------

*   `int exit_code` — The exit status code to return to the parent process

Returns
-------

This system call does not return as it terminates the current process.

Notes
-----

- System call number: `SYSTEM_COMMAND0_EXIT` (0)
- This is a non-returning system call
- The exit code is typically 0 for successful termination, non-zero for errors
- After calling this system call, the process resources are cleaned up by the kernel
