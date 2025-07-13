kfree
=====

**Prototype:**

```c
void kfree(void* ptr);
```

**Type:** `Internal Kernel API`

Description
-----------

Frees a previously allocated block of kernel memory. This function deallocates memory that was previously allocated using `kmalloc` or `kzalloc`, making it available for future kernel allocations. The pointer becomes invalid after this call.

Parameters
----------

*   `void* ptr` — Pointer to the memory block to free (must have been returned by `kmalloc` or `kzalloc`)

Returns
-------

None (void function).

Notes
-----

- This function is for kernel use only and should not be called from user space
- The pointer must have been allocated by `kmalloc` or `kzalloc`
- Passing a NULL pointer may result in undefined behavior
- After calling this function, the pointer should not be used again
- Double-freeing the same pointer may cause heap corruption
- This function may be called from interrupt context depending on heap implementation
