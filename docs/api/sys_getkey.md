sys_getkey
==========

**Prototype:**

```c
char sys_getkey(void);
```

**Type:** `System Call`

Description
-----------

Retrieves the next character from the keyboard input buffer. This system call provides a blocking interface to keyboard input, returning the next available character from the keyboard buffer. If no key is available, the behavior depends on the keyboard buffer implementation.

Parameters
----------

None.

Returns
-------

Returns the ASCII character code of the next key pressed, or 0 if no key is available.

Notes
-----

- System call number: `SYSTEM_COMMAND2_GETKEY` (2)
- This function pops the next character from the keyboard buffer
- The keyboard buffer is managed by the kernel's keyboard driver
- May return 0 if no key is currently available in the buffer
- Special keys and key combinations may be handled differently by the keyboard driver
