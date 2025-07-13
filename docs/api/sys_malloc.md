sys_malloc
==========

**Prototype:**

```c
void* sys_malloc(size_t size);
```

**Type:** `System Call`

Description
-----------

Allocates a block of memory of the specified size for the current process. This system call provides dynamic memory allocation in user space, with the kernel managing the allocation within the process's memory space. The allocated memory is not initialized and may contain arbitrary data.

Parameters
----------

*   `size_t size` — The number of bytes to allocate

Returns
-------

Returns a pointer to the allocated memory block on success, or NULL if allocation fails.

Notes
-----

- System call number: `SYSTEM_COMMAND4_MALLOC` (4)
- Allocations larger than 1MB are logged as potentially suspicious
- The memory is allocated from the process's heap space
- The returned pointer is valid until freed with `sys_free`
- Memory is not initialized - contains arbitrary data
- Allocation failures can occur due to insufficient memory or heap fragmentation
