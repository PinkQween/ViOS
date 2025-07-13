vix_draw_pixel
==============

**Prototype:**

```c
int vix_draw_pixel(int x, int y, uint32_t color);
```

**Type:** `System Call`

Description
-----------

Draws a single pixel at the specified coordinates with the given color. This system call is part of the VIX graphics API and provides the most basic graphics primitive for pixel-level drawing operations.

Parameters
----------

*   `int x` — X coordinate of the pixel
*   `int y` — Y coordinate of the pixel
*   `uint32_t color` — RGB color value in 24-bit format

Returns
-------

Returns 0 on success, or an error code if the coordinates are out of bounds.

Notes
-----

- System call number: `SYSTEM_COMMAND11_VIX_DRAW_PIXEL` (11)
- Part of the VIX graphics API for 2D rendering
- Color format is RGB with 8 bits per channel
- Use `VIX_RGB(r, g, b)` macro to create color values
- Coordinates are checked against screen boundaries
- Changes are not visible until `vix_present_frame` is called
