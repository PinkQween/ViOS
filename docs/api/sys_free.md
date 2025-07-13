sys_free
========

**Prototype:**

```c
void sys_free(void* ptr);
```

**Type:** `System Call`

Description
-----------

Frees a previously allocated block of memory. This system call deallocates memory that was previously allocated using `sys_malloc`, making it available for future allocations. The pointer becomes invalid after this call.

Parameters
----------

*   `void* ptr` — Pointer to the memory block to free (must have been returned by `sys_malloc`)

Returns
-------

Returns 0 on success.

Notes
-----

- System call number: `SYSTEM_COMMAND5_FREE` (5)
- The pointer is validated before freeing to prevent crashes
- Passing a NULL pointer is safe and does nothing
- Attempting to free an invalid pointer may result in undefined behavior
- After calling this function, the pointer should not be used again
- Double-freeing the same pointer may cause heap corruption
