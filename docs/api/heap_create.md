heap_create
===========

**Prototype:**

```c
int heap_create(struct heap* heap, void* ptr, void* end, struct heap_table* table);
```

**Type:** `Internal Kernel API`

Description
-----------

Creates and initializes a new heap data structure for dynamic memory allocation. This function sets up the heap management system by validating parameters, initializing the heap structure, and setting up the block allocation table. The heap uses a block-based allocation strategy with a table to track allocation status.

Parameters
----------

*   `struct heap* heap` — Pointer to the heap structure to initialize
*   `void* ptr` — Starting address of the heap memory region (must be block-aligned)
*   `void* end` — Ending address of the heap memory region (must be block-aligned)
*   `struct heap_table* table` — Pointer to the pre-allocated heap table structure

Returns
-------

Returns 0 on success, or a negative error code on failure:
- `-EINVARG` if alignment requirements are not met or table validation fails

Notes
-----

- Both `ptr` and `end` must be aligned to `VIOS_HEAP_BLOCK_SIZE` boundaries
- The heap table must be pre-allocated and sized appropriately for the heap region
- The table entries are initialized to `HEAP_BLOCK_TABLE_ENTRY_FREE`
- This function is called during kernel initialization to set up the kernel heap
- The heap uses a block-based allocation strategy where each block is `VIOS_HEAP_BLOCK_SIZE` bytes
- The heap table tracks allocation status using bit flags for each block
