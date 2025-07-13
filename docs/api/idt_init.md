idt_init
========

**Prototype:**

```c
void idt_init(void);
```

**Type:** `Internal Kernel API`

Description
-----------

Initializes the Interrupt Descriptor Table (IDT) and sets up interrupt handling for the kernel. This function creates and configures the IDT with appropriate interrupt service routines, sets up hardware interrupt handlers, and enables interrupt processing. The IDT is crucial for handling hardware interrupts, exceptions, and system calls.

Parameters
----------

None.

Returns
-------

None (void function).

Notes
-----

- Creates and initializes the IDT with 256 entries (0-255)
- Sets up interrupt service routines for common exceptions (divide by zero, page fault, etc.)
- Configures hardware interrupt handlers for timer, keyboard, and mouse
- Installs the system call handler for interrupt 0x80
- Loads the IDTR register with the IDT base address and limit
- Must be called during kernel initialization before enabling interrupts
- The IDT remains active throughout the kernel's lifetime
- Interrupt handlers are implemented in assembly and C code
- System calls use interrupt 0x80 as the software interrupt mechanism
