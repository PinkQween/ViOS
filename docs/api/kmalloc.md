kmalloc
=======

**Prototype:**

```c
void* kmalloc(size_t size);
```

**Type:** `Internal Kernel API`

Description
-----------

Allocates a block of memory from the kernel heap. This function is used internally by the kernel to allocate memory for kernel data structures, buffers, and other kernel-space objects. The memory is allocated from the dedicated kernel heap and is not accessible to user processes.

Parameters
----------

*   `size_t size` — The number of bytes to allocate

Returns
-------

Returns a pointer to the allocated memory block on success, or NULL if allocation fails.

Notes
-----

- This function is for kernel use only and should not be called from user space
- Memory is allocated from the kernel heap, separate from process heaps
- The returned memory is not initialized and may contain arbitrary data
- Allocation failures can occur if the kernel heap is exhausted
- The returned pointer is valid until freed with `kfree`
- This function may be called from interrupt context depending on heap implementation
