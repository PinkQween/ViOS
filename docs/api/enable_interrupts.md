enable_interrupts
=================

**Prototype:**

```c
void enable_interrupts(void);
```

**Type:** `Internal Kernel API`

Description
-----------

Enables hardware interrupts by setting the interrupt flag in the CPU's EFLAGS register. This function allows the CPU to respond to hardware interrupts such as timer ticks, keyboard input, and mouse events. It is typically called after the IDT has been properly initialized and interrupt handlers have been set up.

Parameters
----------

None.

Returns
-------

None (void function).

Notes
-----

- Executes the `sti` (set interrupt flag) assembly instruction
- Must be called after `idt_init` to ensure proper interrupt handling
- Enables all maskable interrupts that are not disabled by the PIC
- Critical for system responsiveness and multitasking
- Should be called during kernel initialization once interrupt infrastructure is ready
- Can be paired with `disable_interrupts` to create critical sections
- Does not affect non-maskable interrupts (NMI)
- Essential for timer-based task scheduling and device input handling
