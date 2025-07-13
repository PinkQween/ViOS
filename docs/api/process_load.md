process_load
============

**Prototype:**

```c
int process_load(const char* filename, struct process** process);
```

**Type:** `Internal Kernel API`

Description
-----------

Loads a program from the filesystem into memory and creates a new process structure. This function handles loading executable files (ELF or binary format), allocating memory for the process, and initializing the process control block. The process is loaded but not started.

Parameters
----------

*   `const char* filename` — Path to the executable file to load
*   `struct process** process` — Pointer to store the created process structure

Returns
-------

Returns 0 on success, or a negative error code on failure.

Notes
-----

- This function is for kernel use only
- Supports both ELF and binary executable formats
- The process is loaded but not started - use `process_switch` to begin execution
- Memory is allocated for the process code, data, and stack
- The process structure is initialized with default values
- File format is detected automatically based on file headers
- Failure can occur due to file not found, invalid format, or insufficient memory
