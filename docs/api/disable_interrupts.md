disable_interrupts
==================

**Prototype:**

```c
void disable_interrupts(void);
```

**Type:** `Internal Kernel API`

Description
-----------

Disables hardware interrupts by clearing the interrupt flag in the CPU's EFLAGS register. This function prevents the CPU from responding to maskable hardware interrupts, creating a critical section where kernel code can execute atomically without being interrupted by hardware events.

Parameters
----------

None.

Returns
-------

None (void function).

Notes
-----

- Executes the `cli` (clear interrupt flag) assembly instruction
- Creates critical sections for atomic operations
- Should be used sparingly and for short durations to maintain system responsiveness
- Often paired with `enable_interrupts` to restore interrupt handling
- Does not affect non-maskable interrupts (NMI)
- Essential for protecting critical kernel data structures from race conditions
- Used during context switches and other sensitive kernel operations
- Prolonged use can cause system unresponsiveness and missed interrupts
