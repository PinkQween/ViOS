#include "vigfx.h"
#include "vesa.h"

// VBE Information Block Structure
typedef struct VBEInfoBlockStruct
{
    unsigned short mode_attribute;
    unsigned char win_a_attribute;
    unsigned char win_b_attribute;
    unsigned short win_granuality;
    unsigned short win_size;
    unsigned short win_a_segment;
    unsigned short win_b_segment;
    unsigned int win_func_ptr;
    unsigned short bytes_per_scan_line;
    unsigned short x_resolution;
    unsigned short y_resolution;
    unsigned char char_x_size;
    unsigned char char_y_size;
    unsigned char number_of_planes;
    unsigned char bits_per_pixel;
    unsigned char number_of_banks;
    unsigned char memory_model;
    unsigned char bank_size;
    unsigned char number_of_image_pages;
    unsigned char b_reserved;
    unsigned char red_mask_size;
    unsigned char red_field_position;
    unsigned char green_mask_size;
    unsigned char green_field_position;
    unsigned char blue_mask_size;
    unsigned char blue_field_position;
    unsigned char reserved_mask_size;
    unsigned char reserved_field_position;
    unsigned char direct_color_info;
    unsigned int screen_ptr;
} VBEInfoBlock;

#define VBEInfoAddress 0x900
#define ScreenBufferAddress 0x00A00000
#define ScreenBufferAddressSecondary 0x00B00000

static int rgb(int r, int g, int b)
{
    // Convert 8-bit per channel to 5:6:5
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

static int width = 0;
static int height = 0;

static void vesa_draw_pixel(int x, int y, int r, int g, int b)
{
    VBEInfoBlock *VBE = (VBEInfoBlock *)VBEInfoAddress;
    unsigned short *buffer = (unsigned short *)ScreenBufferAddress;
    int index = y * VBE->x_resolution + x;
    buffer[index] = rgb(r, g, b);
}

static void vesa_clear(int r, int g, int b)
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            vesa_draw_pixel(x, y, r, g, b);
        }
    }
}

static uint32_t vesa_get_pixel(int x, int y)
{
    VBEInfoBlock *VBE = (VBEInfoBlock *)VBEInfoAddress;
    unsigned short *buffer = (unsigned short *)ScreenBufferAddress;
    int index = y * VBE->x_resolution + x;
    unsigned short pixel = buffer[index];
    
    // Convert from 5:6:5 format to 8-bit per channel and pack into 0xRRGGBB
    uint8_t r = ((pixel >> 8) & 0xF8) | ((pixel >> 13) & 0x07);
    uint8_t g = ((pixel >> 3) & 0xFC) | ((pixel >> 9) & 0x03);
    uint8_t b = ((pixel << 3) & 0xF8) | ((pixel >> 2) & 0x07);

    return (r << 16) | (g << 8) | b;
}

static void vesa_flush()
{
    VBEInfoBlock *VBE = (VBEInfoBlock *)VBEInfoAddress;
    unsigned short *buffer = (unsigned short *)ScreenBufferAddress;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int index = y * width + x;
            ((unsigned short *)VBE->screen_ptr)[index] = buffer[index];
        }
    }
}

static struct graphics_device vesa_device = {
    .draw_pixel = vesa_draw_pixel,
    .clear = vesa_clear,
    .flush = vesa_flush,
    .get_pixel = vesa_get_pixel,
    .width = 0, // filled on init
    .height = 0};

struct graphics_device *vesa_init()
{
    VBEInfoBlock *VBE = (VBEInfoBlock *)VBEInfoAddress;
    width = VBE->x_resolution;
    height = VBE->y_resolution;

    vesa_device.width = width;
    vesa_device.height = height;

    return &vesa_device;
}
