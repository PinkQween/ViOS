paging_new_4gb
==============

**Prototype:**

```c
struct paging_4gb_chunk* paging_new_4gb(uint8_t flags);
```

**Type:** `Internal Kernel API`

Description
-----------

Creates a new 4GB paging directory structure that maps the entire 4GB address space. This function allocates and initializes a complete page directory with 1024 page tables, each containing 1024 page entries, providing identity mapping for the full 32-bit address space. This is used for process isolation and memory management.

Parameters
----------

*   `uint8_t flags` — Page flags to apply to all page entries (combination of `PAGING_*` flags)

Returns
-------

Returns a pointer to the newly created paging structure on success, or NULL if allocation fails.

Notes
-----

- Creates a complete 4GB identity mapping (virtual address = physical address)
- Allocates 1024 page tables, each with 1024 entries (1,048,576 total page entries)
- Each page entry covers 4KB of memory (`PAGING_PAGE_SIZE`)
- Common flags include `PAGING_IS_PRESENT`, `PAGING_IS_WRITEABLE`, `PAGING_ACCESS_FROM_ALL`
- The returned structure can be used with `paging_switch` to activate the page directory
- Memory is allocated using `kzalloc` and must be freed with `paging_free_4gb`
- This function is called during task creation and kernel initialization
- Each process typically has its own 4GB paging structure for memory isolation
