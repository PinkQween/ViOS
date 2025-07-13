kheap_init
==========

**Prototype:**

```c
void kheap_init(void);
```

**Type:** `Internal Kernel API`

Description
-----------

Initializes the kernel heap subsystem by creating and configuring the main kernel heap. This function sets up the heap table, calculates the required number of blocks, and creates the kernel heap structure used by `kmalloc`, `kzalloc`, and `kfree`. This is a critical initialization function that must be called before any dynamic memory allocation.

Parameters
----------

None.

Returns
-------

None (void function).

Notes
-----

- Must be called during kernel initialization before any memory allocation
- Creates the kernel heap at `VIOS_HEAP_ADDRESS` with size `VIOS_HEAP_SIZE_BYTES`
- Sets up the heap table at `VIOS_HEAP_TABLE_ADDRESS`
- Calculates the total number of blocks based on `VIOS_HEAP_BLOCK_SIZE`
- Calls `panic` if heap creation fails, as this is a critical system failure
- The kernel heap is used for all kernel dynamic memory allocation
- This function is called once during system startup
- After this call, `kmalloc`, `kzalloc`, and `kfree` become available for use
