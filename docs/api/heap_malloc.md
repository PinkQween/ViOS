heap_malloc
===========

**Prototype:**

```c
void* heap_malloc(struct heap* heap, size_t size);
```

**Type:** `Internal Kernel API`

Description
-----------

Allocates a contiguous block of memory from the specified heap. This function implements the core heap allocation algorithm by finding a suitable sequence of free blocks, marking them as allocated, and returning a pointer to the allocated memory. The allocation size is automatically aligned to block boundaries.

Parameters
----------

*   `struct heap* heap` — Pointer to the heap to allocate from
*   `size_t size` — Number of bytes to allocate

Returns
-------

Returns a pointer to the allocated memory block on success, or NULL if allocation fails due to insufficient contiguous space.

Notes
-----

- The requested size is automatically aligned up to `VIOS_HEAP_BLOCK_SIZE` boundaries
- The function searches for contiguous free blocks to satisfy the allocation
- Allocated blocks are marked with `HEAP_BLOCK_TABLE_ENTRY_TAKEN` and appropriate flags
- The first block in an allocation is marked with `HEAP_BLOCK_IS_FIRST`
- Multi-block allocations use `HEAP_BLOCK_HAS_NEXT` to chain blocks together
- This function is used internally by `kmalloc` and `kzalloc`
- Returns NULL if no suitable contiguous block sequence is found
