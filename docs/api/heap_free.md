heap_free
=========

**Prototype:**

```c
void heap_free(struct heap* heap, void* ptr);
```

**Type:** `Internal Kernel API`

Description
-----------

Frees a previously allocated memory block and returns it to the heap for reuse. This function handles the deallocation process by marking the associated heap blocks as free and following the chain of blocks if the allocation spans multiple blocks.

Parameters
----------

*   `struct heap* heap` — Pointer to the heap containing the memory block
*   `void* ptr` — Pointer to the memory block to free (must have been returned by `heap_malloc`)

Returns
-------

None (void function).

Notes
-----

- The pointer must have been previously returned by `heap_malloc` on the same heap
- The function automatically follows the `HEAP_BLOCK_HAS_NEXT` chain to free all blocks
- All blocks in the allocation are marked as `HEAP_BLOCK_TABLE_ENTRY_FREE`
- Block flags are cleared during the freeing process
- This function is used internally by `kfree`
- Freeing an invalid pointer may cause undefined behavior
- The freed memory becomes available for subsequent allocations
