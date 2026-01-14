#include "graphics/window.h"
#include "graphics/graphics.h"
#include "lib/vector/vector.h"
#include "memory/heap/kheap.h"
#include "keyboard/keyboard.h"
// include the mouse mouse.h
#include "memory/memory.h"
#include "string/string.h"
#include "graphics/font.h"
#include "task/process.h"
// include tsc.h
#include "status.h"
#include "kernel.h"

// vector of struct window*
struct vector *windows_vector;

// close icon image
struct image *close_icon = NULL;

// Which window is currently moving
struct window *window_moving = NULL;

// Which window currently has focus
struct window *focused_window = NULL;

int window_autoincrement_id_current = 100000;
size_t window_get_largest_zindex();
int window_recalculate_zindexes();

int window_system_initialize()
{
    int res = 0;

    windows_vector = vector_new(sizeof(struct window *), 10, 0);
    if (!windows_vector)
    {
        res = -ENOMEM;
        goto out;
    }

    close_icon = graphics_image_load("@:/clsicon.bmp");
    if (!close_icon)
    {
        res = -EIO;
        goto out;
    }

    window_moving = NULL;
    focused_window = NULL;
    window_autoincrement_id_current = 100000;
out:
    return res;
}

void window_screen_mouse_move_handler(struct mouse *mouse, int moved_to_x, int moved_to_y)
{
    if (window_moving)
    {
        if (window_moving->title_bar_graphics)
        {
            size_t abs_x = moved_to_x - (window_moving->title_bar_graphics->width / 2);
            size_t abs_y = moved_to_y - (window_moving->title_bar_graphics->height / 2);
            window_position_set(window_moving, abs_x, abs_y);
        }

        size_t rel_x = moved_to_x - window_moving->root_graphics->starting_x;
        size_t rel_y = moved_to_y - window_moving->root_graphics->starting_y;

        struct window_event event = {0};
        event.type = WINDOW_EVENT_TYPE_MOUSE_MOVE;
        event.data.move.x = rel_x;
        event.data.move.y = rel_y;
        window_event_push(window_moving, &event);
    }
}

struct window *window_get_from_graphics(struct graphics_info *graphics)
{
    struct window *window = NULL;
    size_t total_windows = vector_count(windows_vector);
    for (size_t i = 0; i < total_windows; i++)
    {
        struct window *win = NULL;
        vector_at(windows_vector, i, &win, sizeof(win));
        if (win && window_owns_graphics(win, graphics))
        {
            window = win;
            break;
        }
    }

    return window;
}

struct window *window_get_at_position(size_t abs_x, size_t abs_y, struct window *ignore_window)
{
    size_t total_windows = vector_count(windows_vector);
    for (size_t i = 0; i < total_windows; i++)
    {
        struct window *win = NULL;
        vector_at(windows_vector, i, &win, sizeof(win));

        if (win && win != ignore_window)
        {
            size_t whole_win_width = win->root_graphics->width;
            size_t whole_win_height = win->root_graphics->height;
            size_t end_abs_x = win->root_graphics->starting_x + whole_win_width;
            size_t end_abs_y = win->root_graphics->starting_y + whole_win_height;
            if (abs_x >= win->x && abs_x < end_abs_x && abs_y >= win->y && abs_y < end_abs_y)
            {
                // This was the window that was clicked
                return win;
            }
        }
    }

    return NULL;
}
void window_click_handler(struct mouse *mouse, int abs_x, int abs_y, MOUSE_CLICK_TYPE type)
{
    struct window *win = window_get_at_position(abs_x, abs_y, mouse->graphic.window);
    if (win)
    {
        int rel_x = abs_x - win->root_graphics->starting_x;
        int rel_y = abs_y - win->root_graphics->starting_y;
        window_click(win, rel_x, rel_y, type);
        window_focus(win);
    }
}

int window_system_initialize_stage2()
{
    mouse_register_move_handler(NULL, window_screen_mouse_move_handler);
    mouse_register_click_handler(NULL, window_click_handler);
    // TODO: Implement keyboard functionality
    return 0;
}

struct terminal *window_terminal(struct window *window)
{
    return window->terminal;
}
void window_draw_title_bar(struct window *window, struct framebuffer_pixel title_bar_bg_color)
{
    if (!window || !window->title_bar_graphics)
    {
        return;
    }

    size_t total_window_width_bounds = window->title_bar_graphics->width;
    size_t icon_pos_x = window->title_bar_components.close_btn.x;
    size_t icon_pos_y = window->title_bar_components.close_btn.y;
    const char *title = window->title;

    // draww the background of the title bar
    terminal_draw_rect(window->title_bar_terminal, 0, 0, total_window_width_bounds, WINDOW_TITLE_BAR_HEIGHT, title_bar_bg_color);

    // Draw the title text
    terminal_cursor_set(window->title_bar_terminal, 0, 0);
    terminal_print(window->title_bar_terminal, title);

    // Draw the close icon ignoring white
    struct framebuffer_pixel white_color = {0};
    white_color.red = 0xff;
    white_color.green = 0xff;
    white_color.blue = 0xff;
    terminal_ignore_color(window->title_bar_terminal, white_color);
    terminal_draw_image(window->title_bar_terminal, icon_pos_x, icon_pos_y, close_icon);
    terminal_ignore_color_finish(window->title_bar_terminal);
}

int window_reorder(void *first_elem, void *second_elem)
{
    struct window *win1 = *(struct window **)(first_elem);
    struct window *win2 = *(struct window **)(second_elem);

    return (win1->zindex < win2->zindex);
}

void window_set_z_index(struct window *window, int zindex)
{
    graphics_set_z_index(window->root_graphics, zindex);

    // We need to reorder the windows vector now that zindex changed
    vector_reorder(windows_vector, window_reorder);
}

void window_unfocus(struct window *old_focused_window)
{
    print_early("[window_unfocus] called\n");
    if (!old_focused_window) {
        print_early("[window_unfocus] old_focused_window is NULL\n");
        return;
    }
    struct framebuffer_pixel black = {0};
    black.red = 0x00;
    black.green = 0x00;
    black.blue = 0x00;
    if (old_focused_window->title_bar_graphics) {
        window_draw_title_bar(old_focused_window, black);
        graphics_redraw_region(graphics_screen_info(), old_focused_window->root_graphics->starting_x, old_focused_window->root_graphics->starting_y, old_focused_window->root_graphics->width, old_focused_window->root_graphics->height);
    } else {
        print_early("[window_unfocus] old_focused_window->title_bar_graphics is NULL\n");
    }
    struct window_event event = {0};
    event.type = WINDOW_EVENT_TYPE_LOST_FOCUS;
    window_event_push(old_focused_window, &event);
}

void window_bring_to_top(struct window *window)
{
    size_t last_index = 0;
    struct graphics_info *screen_graphics = graphics_screen_info();
    size_t child_count = vector_count(screen_graphics->children);
    if (child_count > 0)
    {
        struct graphics_info *child_graphics = NULL;
        size_t child_index = child_count - 1;
        vector_at(screen_graphics->children, child_index, &child_graphics, sizeof(child_graphics));
        if (child_graphics)
        {
            last_index = child_graphics->z_index;
        }
    }

    window_set_z_index(window, last_index + 1);
}

void window_focus(struct window *window)
{
    print_early("[window_focus] called\n");
    if (!window) {
        print_early("[window_focus] window is NULL\n");
        return;
    }
    if (focused_window == window)
    {
        print_early("[window_focus] already focused\n");
        return;
    }
    struct window *old_focused_window = focused_window;
    focused_window = window;
    struct framebuffer_pixel red = {0};
    red.red = 0xff;
    red.green = 0x00;
    red.blue = 0x00;
    if (old_focused_window && old_focused_window->title_bar_graphics)
    {
        window_unfocus(old_focused_window);
    } else if (old_focused_window) {
        print_early("[window_focus] old_focused_window->title_bar_graphics is NULL\n");
    }
    // Bring the new window to the top
    window_bring_to_top(window);
    // Update the new windows title bar to red
    if (window->title_bar_graphics)
    {
        window_draw_title_bar(window, red);
        print_early("[window_focus] window->title_bar_graphics is valid in focus\n");
    } else {
        print_early("[window_focus] window->title_bar_graphics is NULL\n");
    }
    
    // SKIP: This causes infinite recursion! window_create already calls graphics_redraw_all()
    // graphics_redraw_graphics_to_screen(window->root_graphics, 0, 0, window->root_graphics->width, window->root_graphics->height);
    print_early("[window_focus] skipped graphics_redraw_graphics_to_screen\n");
    
    // TEMPORARILY SKIP EVENT SYSTEM - may be causing hang
    // struct window_event event = {0};
    // event.type = WINDOW_EVENT_TYPE_FOCUS;
    // window_event_push(window, &event);
    
    print_early("[window_focus] done\n");
}

void window_event_handler_unregister(struct window *window, WINDOW_EVENT_HANDLER handler)
{
    vector_pop_element(window->event_handlers.handlers, &handler, sizeof(handler));
}

void window_event_handler_register(struct window *window, WINDOW_EVENT_HANDLER handler)
{
    vector_push(window->event_handlers.handlers, &handler);
}

void window_drop_event_handlers(struct window *window)
{
    WINDOW_EVENT_HANDLER handler = NULL;
    vector_at(window->event_handlers.handlers, 0, &handler, sizeof(handler));
    while (handler)
    {
        // This function will pop from the handler vector
        window_event_handler_unregister(window, handler);

        vector_at(window->event_handlers.handlers, 0, &handler, sizeof(handler));
    }
}
void window_free(struct window *window)
{
    // drop the event handlers
    window_drop_event_handlers(window);
    // free the event handlers vector
    vector_free(window->event_handlers.handlers);

    // Pop the window pointer from the vector
    vector_pop_element(windows_vector, &window, sizeof(window));
    terminal_free(window->terminal);

    // free the title terminal
    terminal_free(window->title_bar_terminal);

    // Free the root graphics which will free aall children
    graphics_info_free(window->root_graphics);
    kfree(window);
}

void window_event_push(struct window *window, struct window_event *event)
{
    event->window = window;
    event->win_id = window->id;

    // Loop through all the event handlers and push the event to them
    size_t total_handlers = vector_count(window->event_handlers.handlers);
    for (size_t i = 0; i < total_handlers; i++)
    {
        WINDOW_EVENT_HANDLER handler = NULL;
        vector_at(window->event_handlers.handlers, i, &handler, sizeof(handler));
        if (handler)
        {
            handler(window, event);
        }
    }
}

void window_click(struct window *window, int rel_x, int rel_y, MOUSE_CLICK_TYPE type)
{
    struct window_event event = {0};
    event.type = WINDOW_EVENT_TYPE_MOUSE_CLICK;
    event.data.click.x = rel_x;
    event.data.click.y = rel_y;
    window_event_push(window, &event);
}

void window_close(struct window *window)
{
    struct window_event event = {0};
    event.type = WINDOW_EVENT_TYPE_WINDOW_CLOSE;
    window_event_push(window, &event);

    window_free(window);
    graphics_redraw_all();
}

int window_event_handler(struct window *window, struct window_event *win_event)
{
    // do nothing for now
    return 0;
}

int window_position_set(struct window *window, size_t new_x, size_t new_y)
{
    int res = 0;

    int x_redraw_x = 0;
    int x_redraw_y = 0;
    int x_redraw_width = 0;
    int x_redraw_height = 0;

    int y_redraw_x = 0;
    int y_redraw_y = 0;
    int y_redraw_width = 0;
    int y_redraw_height = 0;

    struct graphics_info *screen = graphics_screen_info();
    size_t ending_x = new_x + window->width;
    size_t ending_y = new_y + window->height;
    if (ending_x > screen->width)
    {
        new_x = screen->width - window->width - 1;
    }

    if (ending_y > screen->height)
    {
        new_y = screen->height - window->height - 1;
    }

    int old_screen_x = window->root_graphics->starting_x;
    int old_screen_y = window->root_graphics->starting_y;

    window->root_graphics->relative_x = new_x;
    window->root_graphics->relative_y = new_y;
    window->root_graphics->starting_x = new_x;
    window->root_graphics->starting_y = new_y;

    window->x = new_x;
    window->y = new_y;

    window_bring_to_top(window);

    graphics_info_recalculate(window->root_graphics);

    int x_gap = old_screen_x - (int)window->root_graphics->starting_x;
    int y_gap = old_screen_y - (int)window->root_graphics->starting_y;
    bool moved_left = x_gap >= 0;
    bool moved_up = y_gap >= 0;
    x_redraw_x = window->root_graphics->starting_x + window->root_graphics->width;
    x_redraw_width = x_gap;
    x_redraw_y = old_screen_y;
    x_redraw_height = window->root_graphics->height;

    if (!moved_left)
    {
        x_redraw_x = window->root_graphics->starting_x + x_gap;
        // negate the x_gap
        x_redraw_width = -x_gap;
    }

    y_redraw_x = old_screen_x;
    y_redraw_y = window->root_graphics->starting_y + window->root_graphics->height;

    y_redraw_width = window->root_graphics->width;
    y_redraw_height = y_gap;
    if (!moved_up)
    {
        y_redraw_y = window->root_graphics->starting_y + y_gap;
        y_redraw_height = -y_gap;
    }

    if ((x_redraw_width > window->root_graphics->width) ||
        (x_redraw_height > window->root_graphics->height) ||
        (y_redraw_width > window->root_graphics->width) ||
        (y_redraw_height > window->root_graphics->height))
    {
        graphics_redraw_region(graphics_screen_info(), old_screen_x, old_screen_y, window->root_graphics->width, window->root_graphics->height);
    }
    else
    {
        graphics_redraw_region(graphics_screen_info(), x_redraw_x, x_redraw_y, x_redraw_width, x_redraw_height);
        graphics_redraw_region(graphics_screen_info(), y_redraw_x, y_redraw_y, y_redraw_width, y_redraw_height);
    }

    window_redraw(window);
out:
    return res;
}

void window_redraw(struct window *window)
{
    graphics_redraw(window->root_graphics);
}

void window_redraw_body_region(struct window *window, int x, int y, int width, int height)
{
    graphics_redraw_region(window->graphics, x, y, width, height);
}

void window_redraw_region(struct window *window, int x, int y, int width, int height)
{
    graphics_redraw_region(window->root_graphics, x, y, width, height);
}

void window_title_set(struct window *window, const char *title)
{
    strncpy(window->title, title, sizeof(window->title));
    // Black
    struct framebuffer_pixel title_bar_bg_color = {0};

    window_draw_title_bar(window, title_bar_bg_color);
    window_redraw(window);
}

size_t window_get_largest_zindex()
{
    size_t z_index = 0;
    size_t total_windows = vector_count(windows_vector);
    if (total_windows > 0)
    {
        struct window *win = NULL;
        vector_at(windows_vector, 0, &win, sizeof(win));
        if (win)
        {
            z_index = win->zindex;
        }
    }

    return z_index;
}

int window_recalculate_zindexes()
{
    size_t total_windows = vector_count(windows_vector);
    size_t last_zindex = 0;
    for (size_t i = 0; i < total_windows; i++)
    {
        struct window *child_window = NULL;
        vector_at(windows_vector, i, &child_window, sizeof(child_window));
        if (child_window)
        {
            size_t z_index = vector_count(child_window->root_graphics->children) + i + 1;
            graphics_set_z_index(child_window->root_graphics, z_index);
            last_zindex = z_index;
        }
    }

    return last_zindex;
}

struct window *window_focused()
{
    return focused_window;
}

bool window_owns_graphics(struct window *win, struct graphics_info *graphics)
{
    if (graphics == win->root_graphics)
        return true;

    return graphics_has_ancestor(graphics, win->root_graphics);
}

void window_title_bar_mouse_moved(struct graphics_info *title_graphics, size_t rel_x, size_t rel_y, size_t abs_x, size_t abs_y)
{
    // do nothing.
}

void window_title_bar_clicked(struct graphics_info *title_graphics, size_t rel_x, size_t rel_y, MOUSE_CLICK_TYPE type)
{
    struct window *win = window_get_from_graphics(title_graphics);
    if (win)
    {
        size_t close_btn_x = win->title_bar_components.close_btn.x;
        size_t close_btn_y = win->title_bar_components.close_btn.y;
        size_t close_btn_width = win->title_bar_components.close_btn.width;
        size_t close_btn_height = win->title_bar_components.close_btn.height;
        size_t close_btn_ending_x = close_btn_x + close_btn_width;
        size_t close_btn_ending_y = close_btn_y + close_btn_height;
        if (rel_x >= close_btn_x &&
            rel_x < close_btn_ending_x &&
            rel_y >= close_btn_y &&
            rel_y < close_btn_ending_y)
        {
            window_close(win);
            win = NULL;
        }
        else
        {
            // If they click the window they are already moving
            // toggle it as no longer moving.
            if (window_moving == win)
            {
                window_moving = NULL;
            }
            else
            {
                // Ok this was not the currently moving window
                // therefore set the new moving window to the one we clicked.
                window_moving = win;
            }
        }
    }
}
struct window *window_create(struct graphics_info *graphics_info, struct font *font, const char *title, size_t x, size_t y, size_t width, size_t height, uint64_t flags, uint64_t id)
{
    print_early("[window_create] called\n");
    {
        char buf[256];
        buf[0] = '['; buf[1] = 'p'; buf[2] = 'a'; buf[3] = 'r'; buf[4] = 'a'; buf[5] = 'm'; buf[6] = 's'; buf[7] = ':';
        buf[8] = ' '; buf[9] = 'w'; buf[10] = '=';
        unsigned long val = (unsigned long)width;
        int n = 11, i;
        for (i = 15; i >= 0; i--) {
            int v = (val >> (i * 4)) & 0xF;
            buf[n++] = (v < 10) ? ('0' + v) : ('A' + v - 10);
        }
        buf[n++] = ' '; buf[n++] = 'h'; buf[n++] = '=';
        val = (unsigned long)height;
        for (i = 15; i >= 0; i--) {
            int v = (val >> (i * 4)) & 0xF;
            buf[n++] = (v < 10) ? ('0' + v) : ('A' + v - 10);
        }
        buf[n++] = ' '; buf[n++] = 'f'; buf[n++] = 'l'; buf[n++] = 'a'; buf[n++] = 'g'; buf[n++] = 's'; buf[n++] = '=';
        val = (unsigned long)flags;
        for (i = 15; i >= 0; i--) {
            int v = (val >> (i * 4)) & 0xF;
            buf[n++] = (v < 10) ? ('0' + v) : ('A' + v - 10);
        }
        buf[n++] = ']'; buf[n++] = '\n'; buf[n] = 0;
        print_early(buf);
    }
    int res = 0;
    if (!windows_vector)
    {
        print_early("[window_create] windows_vector not initialized\n");
        panic("Window system was not initialized\n");
    }
    if (width < 1 || height < 1)
    {
        print_early("[window_create] invalid width/height\n");
        res = -EINVARG;
        goto out;
    }
    if (!font)
    {
        print_early("[window_create] no font, using system font\n");
        font = font_get_system_font();
    }
    struct window *window = kzalloc(sizeof(struct window));
    if (!window)
    {
        print_early("[window_create] window alloc failed\n");
        res = -ENOMEM;
        goto out;
    }
    if (id == -1)
    {
        id = window_autoincrement_id_current;
        window_autoincrement_id_current++;
    }
    strncpy(window->title, title, sizeof(window->title));
    window->x = x;
    window->y = y;
    window->width = width;
    window->height = height;
    window->flags = flags;
    {
        char buf[64];
        buf[0] = '['; buf[1] = 'f'; buf[2] = 'l'; buf[3] = 'a'; buf[4] = 'g'; buf[5] = 's'; buf[6] = '_'; buf[7] = 's'; buf[8] = 'e'; buf[9] = 't'; buf[10] = '=';
        unsigned long val = (unsigned long)flags;
        int n = 11, i;
        for (i = (sizeof(unsigned long) * 2) - 1; i >= 0; i--) {
            int v = (val >> (i * 4)) & 0xF;
            buf[n++] = (v < 10) ? ('0' + v) : ('A' + v - 10);
        }
        buf[n++] = ']'; buf[n++] = '\n'; buf[n] = 0;
        print_early(buf);
    }
    window->id = id;
    print_early("[window_create] after struct init\n");
    window->event_handlers.handlers = vector_new(sizeof(WINDOW_EVENT_HANDLER), 4, 0);
    if (!window->event_handlers.handlers) {
        print_early("[window_create] event_handlers alloc failed\n");
        res = -ENOMEM;
        goto out;
    }
    print_early("[window_create] event_handlers vector created\n");

    size_t total_window_width_bounds = width;
    size_t total_window_height_bounds = height;
    size_t window_body_height_offset = 0;
    size_t window_body_width_offset = 0;

    struct graphics_info *title_bar_graphics_info = NULL;
    struct graphics_info *border_left_graphics_info = NULL;
    struct graphics_info *border_right_graphics_info = NULL;
    struct graphics_info *border_bottom_graphics_info = NULL;

    if (!(flags & WINDOW_FLAG_BORDERLESS))
    {
        total_window_width_bounds += (WINDOW_BORDER_PIXEL_SIZE * 2);
        total_window_height_bounds += WINDOW_TITLE_BAR_HEIGHT + WINDOW_BORDER_PIXEL_SIZE;
        window_body_height_offset = WINDOW_TITLE_BAR_HEIGHT;
        window_body_width_offset = WINDOW_BORDER_PIXEL_SIZE;
    }

    struct graphics_info *root_graphics_info = graphics_info_create_relative(graphics_info, x, y, total_window_width_bounds, total_window_height_bounds, GRAPHICS_FLAG_DO_NOT_COPY_PIXELS);
    if (!root_graphics_info)
    {
        res = -ENOMEM;
        goto out;
    }
    print_early("[window_create] root_graphics_info created\n");

    if (flags & WINDOW_FLAG_BACKGROUND_TRANSPARENT)
    {
        struct framebuffer_pixel transparency_key = {0};
        transparency_key.blue = 0xff;
        transparency_key.green = 0xff;
        transparency_key.red = 0xff;
        graphics_transparency_key_set(root_graphics_info, transparency_key);
        graphics_draw_rect(root_graphics_info, 0, 0, root_graphics_info->width, root_graphics_info->height, transparency_key);
    }

    window->root_graphics = root_graphics_info;
    {
        char buf[64];
        buf[0] = '['; buf[1] = 'f'; buf[2] = 'l'; buf[3] = 'a'; buf[4] = 'g'; buf[5] = 's'; buf[6] = '=';
        unsigned long val = (unsigned long)flags;
        int n = 7, i;
        for (i = (sizeof(unsigned long) * 2) - 1; i >= 0; i--) {
            int v = (val >> (i * 4)) & 0xF;
            buf[n++] = (v < 10) ? ('0' + v) : ('A' + v - 10);
        }
        buf[n++] = ']'; buf[n++] = '\n'; buf[n] = 0;
        print_early(buf);
    }
    if (!(flags & WINDOW_FLAG_BORDERLESS))
    {
        print_early("[window_create] NOT borderless - creating title_bar_graphics_info...\n");
        title_bar_graphics_info =
            graphics_info_create_relative(root_graphics_info, WINDOW_BORDER_PIXEL_SIZE, 0, width, WINDOW_TITLE_BAR_HEIGHT, 0);
        if (!title_bar_graphics_info)
        {
            print_early("[window_create] ERROR: title_bar_graphics_info creation failed!\n");
            res = -ENOMEM;
            goto out;
        }
        print_early("[window_create] title_bar_graphics_info created\n");

        // click handler
        graphics_click_handler_set(title_bar_graphics_info, window_title_bar_clicked);
        // move handler
        graphics_move_handler_set(title_bar_graphics_info, window_title_bar_mouse_moved);

        window->title_bar_graphics = title_bar_graphics_info;
        if (window->title_bar_graphics) {
            print_early("[window_create] window->title_bar_graphics assigned\n");
        } else {
            print_early("[window_create] window->title_bar_graphics is NULL after assignment!\n");
        }

        border_left_graphics_info =
            graphics_info_create_relative(root_graphics_info, 0, WINDOW_TITLE_BAR_HEIGHT, WINDOW_BORDER_PIXEL_SIZE, height, 0);
        if (!border_left_graphics_info)
        {
            res = -ENOMEM;
            goto out;
        }
        print_early("[window_create] border_left_graphics_info created\n");

        struct graphics_info *border_right_graphics_info =
            graphics_info_create_relative(root_graphics_info, total_window_width_bounds - WINDOW_BORDER_PIXEL_SIZE, WINDOW_TITLE_BAR_HEIGHT, WINDOW_BORDER_PIXEL_SIZE, height, 0);
        if (!border_right_graphics_info)
        {
            res = -ENOMEM;
            goto out;
        }
        print_early("[window_create] border_right_graphics_info created\n");

        struct graphics_info *border_bottom_graphics_info =
            graphics_info_create_relative(root_graphics_info, 0, total_window_height_bounds - WINDOW_BORDER_PIXEL_SIZE, width, WINDOW_BORDER_PIXEL_SIZE, 0);
        if (!border_bottom_graphics_info)
        {
            res = -ENOMEM;
            goto out;
        }
        print_early("[window_create] border_bottom_graphics_info created\n");
    }

    struct graphics_info *window_graphics_info = graphics_info_create_relative(root_graphics_info, window_body_width_offset, window_body_height_offset, width, height, 0);
    if (!window_graphics_info)
    {
        res = -ENOMEM;
        goto out;
    }
    print_early("[window_create] window_graphics_info created\n");

    window->graphics = window_graphics_info;

    if (!(flags & WINDOW_FLAG_BORDERLESS))
    {
        struct framebuffer_pixel title_bar_font_color = {0};
        title_bar_font_color.red = 0xff;
        title_bar_font_color.blue = 0xff;
        title_bar_font_color.green = 0xff;

        window->title_bar_terminal = terminal_create(title_bar_graphics_info, 0, 0, total_window_width_bounds, WINDOW_TITLE_BAR_HEIGHT, font, title_bar_font_color, 0);
        if (!window->title_bar_terminal)
        {
            res = -ENOMEM;
            goto out;
        }
        print_early("[window_create] title_bar_terminal created\n");
    }

    struct framebuffer_pixel pixel_color = {0};
    pixel_color.red = 0xff;
    window->terminal = terminal_create(window_graphics_info, 0, 0, width, height, font, pixel_color, TERMINAL_FLAG_BACKSPACE_ALLOWED);
    if (!window->terminal)
    {
        res = -ENOMEM;
        goto out;
    }
    print_early("[window_create] terminal created\n");

    struct framebuffer_pixel bg_color = {0};
    bg_color.red = 0xff;
    bg_color.blue = 0xff;
    bg_color.green = 0xff;
    terminal_draw_rect(window->terminal, 0, 0, width, height, bg_color);

    // Save the background of the terminal incase of backspaces
    terminal_background_save(window->terminal);
    print_early("[window_create] terminal_background_save done\n");
    if (flags & WINDOW_FLAG_BACKGROUND_TRANSPARENT)
    {
        terminal_transparency_key_set(window->terminal, bg_color);
        print_early("[window_create] terminal_transparency_key_set done\n");
    }
    if (!(flags & WINDOW_FLAG_BORDERLESS))
    {
        size_t icon_pos_x = window->title_bar_terminal->bounds.width - close_icon->width - (close_icon->width / 2);
        size_t icon_pos_y = (window->title_bar_terminal->bounds.height / 2) - (close_icon->height / 2);

        window->title_bar_components.close_btn.x = icon_pos_x;
        window->title_bar_components.close_btn.y = icon_pos_y;
        window->title_bar_components.close_btn.width = close_icon->width;
        window->title_bar_components.close_btn.height = close_icon->height;

        struct framebuffer_pixel title_bar_bg_color = {0};
        title_bar_bg_color.red = 0x00;
        title_bar_bg_color.blue = 0x00;
        title_bar_bg_color.green = 0x00;

        window_draw_title_bar(window, title_bar_bg_color);
        print_early("[window_create] window_draw_title_bar done\n");
        struct framebuffer_pixel border_color = {0};
        graphics_draw_rect(border_left_graphics_info, 0, 0, border_left_graphics_info->width, border_left_graphics_info->height, border_color);
        graphics_draw_rect(border_right_graphics_info, 0, 0, border_right_graphics_info->width, border_right_graphics_info->height, border_color);
        graphics_draw_rect(border_bottom_graphics_info, 0, 0, border_bottom_graphics_info->width, border_bottom_graphics_info->height, border_color);
        print_early("[window_create] border draw done\n");
    }
    vector_push(windows_vector, &window);
    print_early("[window_create] vector_push done\n");
    size_t child_count = vector_count(window->root_graphics->children);
    window_set_z_index(window, child_count + 1);
    print_early("[window_create] window_set_z_index done\n");
    window_event_handler_register(window, window_event_handler);
    print_early("[window_create] window_event_handler_register done\n");
    window_focus(window);
    print_early("[window_create] window_focus done\n");
    graphics_redraw_all();
    print_early("[window_create] graphics_redraw_all done\n");

out:
    if (res < 0)
    {
        if (window)
        {
            if (window->terminal)
            {
                terminal_free(window->terminal);
                window->terminal = NULL;
            }
            if (window->title_bar_terminal)
            {
                terminal_free(window->title_bar_terminal);
                window->title_bar_terminal = NULL;
            }

            vector_pop_element(windows_vector, &window, sizeof(struct window *));
            kfree(window);
            window = NULL;
        }
    }

    return window;
}