sys_print
=========

**Prototype:**

```c
int sys_print(const char* message, int x, int y, int r, int g, int b, int scale);
```

**Type:** `System Call`

Description
-----------

Prints a text string to the screen at the specified coordinates with the given color and scale. This system call renders text using the Atari-style font system and allows precise control over text positioning and appearance.

Parameters
----------

*   `const char* message` — Pointer to the null-terminated string to print
*   `int x` — X coordinate for text placement
*   `int y` — Y coordinate for text placement  
*   `int r` — Red color component (0-255)
*   `int g` — Green color component (0-255)
*   `int b` — Blue color component (0-255)
*   `int scale` — Text scaling factor

Returns
-------

Returns 0 on success.

Notes
-----

- System call number: `SYSTEM_COMMAND1_PRINT` (1)
- The message buffer is safely copied from user space to kernel space
- Buffer size is limited to 1024 characters
- Uses the Atari-style font rendering system
- Color values should be in the range 0-255 for each RGB component
