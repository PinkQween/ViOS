vix_clear_screen
================

**Prototype:**

```c
int vix_clear_screen(uint32_t color);
```

**Type:** `System Call`

Description
-----------

Clears the entire screen by filling it with the specified color. This system call is part of the VIX graphics API and provides an efficient way to clear the framebuffer for new drawing operations.

Parameters
----------

*   `uint32_t color` — RGB color value to fill the screen with

Returns
-------

Returns 0 on success.

Notes
-----

- System call number: `SYSTEM_COMMAND14_VIX_CLEAR_SCREEN` (14)
- Part of the VIX graphics API for 2D rendering
- Color format is RGB with 8 bits per channel
- Use `VIX_RGB(r, g, b)` macro to create color values
- This operation affects the entire screen/framebuffer
- Changes are not visible until `vix_present_frame` is called
- More efficient than drawing individual pixels to clear the screen
