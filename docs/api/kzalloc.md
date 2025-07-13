kzalloc
=======

**Prototype:**

```c
void* kzalloc(size_t size);
```

**Type:** `Internal Kernel API`

Description
-----------

Allocates a block of memory from the kernel heap and initializes it to zero. This function combines the functionality of `kmalloc` and `memset`, providing a convenient way to allocate zero-initialized memory for kernel data structures. The memory is allocated from the dedicated kernel heap.

Parameters
----------

*   `size_t size` — The number of bytes to allocate

Returns
-------

Returns a pointer to the allocated and zero-initialized memory block on success, or NULL if allocation fails.

Notes
-----

- This function is for kernel use only and should not be called from user space
- Memory is allocated from the kernel heap and initialized to zero
- Internally calls `kmalloc` followed by `memset` to zero the memory
- Allocation failures can occur if the kernel heap is exhausted
- The returned pointer is valid until freed with `kfree`
- Preferred over `kmalloc` when zero-initialized memory is required
