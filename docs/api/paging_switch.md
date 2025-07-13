paging_switch
=============

**Prototype:**

```c
void paging_switch(struct paging_4gb_chunk* directory);
```

**Type:** `Internal Kernel API`

Description
-----------

Switches the CPU to use a different page directory for virtual memory translation. This function activates the specified paging directory by loading it into the CR3 register, effectively changing the virtual-to-physical address mapping for the entire system. This is a critical function for process switching and memory management.

Parameters
----------

*   `struct paging_4gb_chunk* directory` — Pointer to the paging directory structure to activate

Returns
-------

None (void function).

Notes
-----

- Immediately changes the active page directory in the CPU's CR3 register
- All virtual memory accesses after this call use the new page directory
- This function is called during process context switches
- The directory must have been created with `paging_new_4gb`
- Switching to an invalid or corrupted page directory can cause system crashes
- This function is used by the task scheduler to isolate process memory spaces
- The kernel maintains a global `current_directory` pointer for tracking
- Must be called with interrupts disabled in critical sections
