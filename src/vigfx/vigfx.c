#include "vigfx.h"
#include "memory/heap/kheap.h"
#include "debug/simple_serial.h"
#include <stdint.h>
#include "memory/memory.h"
#include "virtio_gpu.h"

// Minimal struct definitions for allocation
struct vigfx_device
{
    void *pci_device;  // Pointer to underlying PCI device
    uint32_t device_id;
    uint32_t vendor_id;
    uint32_t features;
    int initialized;
};

struct vigfx_context
{
    int dummy;
};
struct vigfx_memory_pool
{
    int dummy;
};
struct vigfx_buffer
{
    int dummy;
};
struct vigfx_texture
{
    int dummy;
};
struct vigfx_shader
{
    int dummy;
};
struct vigfx_pipeline
{
    int dummy;
};
struct vigfx_render_target
{
    int dummy;
};
struct vigfx_command_buffer
{
    int dummy;
};

// Global device instance
static struct vigfx_device *global_device = NULL;

// --- Framebuffer globals ---
static uint32_t vigfx_fb_width = 0;
static uint32_t vigfx_fb_height = 0;
#define VIGFX_FB_BPP 4 // RGBA8
static uint8_t *vigfx_framebuffer = NULL;

// --- Query display info from VirtIO GPU ---
#include "pci/virtio_gpu_pci.h"
struct virtio_gpu_ctrl_hdr
{
    uint32_t type;
    uint32_t flags;
    uint64_t fence_id;
    uint32_t ctx_id;
    uint32_t padding;
};
struct virtio_gpu_display_one
{
    uint32_t enabled;
    uint32_t flags;
    uint32_t width;
    uint32_t height;
    uint32_t x;
    uint32_t y;
    uint32_t r;
    uint32_t g;
    uint32_t b;
    uint32_t a;
};
struct virtio_gpu_resp_display_info
{
    struct virtio_gpu_ctrl_hdr hdr;
    struct virtio_gpu_display_one pmodes[16];
};

static void vigfx_query_display_info()
{
    struct virtio_gpu_ctrl_hdr cmd = {0};
    cmd.type = 0x0100; // VIRTIO_GPU_CMD_GET_DISPLAY_INFO
    struct virtio_gpu_resp_display_info resp = {0};
    virtio_gpu_pci_send_command(&cmd, sizeof(cmd), &resp, sizeof(resp));
    // Use the first enabled mode
    for (int i = 0; i < 16; ++i)
    {
        if (resp.pmodes[i].enabled && resp.pmodes[i].width > 0 && resp.pmodes[i].height > 0)
        {
            vigfx_fb_width = resp.pmodes[i].width;
            vigfx_fb_height = resp.pmodes[i].height;
            break;
        }
    }
    // Fallback if not found
    if (vigfx_fb_width == 0 || vigfx_fb_height == 0)
    {
        vigfx_fb_width = 1024;
        vigfx_fb_height = 768;
    }
}

// --- Framebuffer allocation ---
static int vigfx_gpu_resource_id = 0;
static int vigfx_gpu_initialized = 0;

static void vigfx_init_framebuffer()
{
    if (vigfx_fb_width == 0 || vigfx_fb_height == 0)
    {
        vigfx_query_display_info();
    }
    if (!vigfx_framebuffer)
    {
        vigfx_framebuffer = (uint8_t *)kmalloc(vigfx_fb_width * vigfx_fb_height * VIGFX_FB_BPP);
        memset(vigfx_framebuffer, 0, vigfx_fb_width * vigfx_fb_height * VIGFX_FB_BPP);
        simple_serial_puts("DEBUG: Framebuffer allocated\n");
    }
    
    // Initialize GPU resources if not already done
    if (!vigfx_gpu_initialized)
    {
        simple_serial_puts("DEBUG: Initializing GPU resources\n");
        
        // Create GPU resource
        vigfx_gpu_resource_id = virtio_gpu_create_resource();
        if (vigfx_gpu_resource_id <= 0)
        {
            simple_serial_puts("DEBUG: Failed to create GPU resource\n");
            return;
        }
        simple_serial_puts("DEBUG: GPU resource created\n");
        
        // Attach backing store
        size_t fb_size = vigfx_fb_width * vigfx_fb_height * VIGFX_FB_BPP;
        if (virtio_gpu_attach_backing(vigfx_gpu_resource_id, vigfx_framebuffer, fb_size) != 0)
        {
            simple_serial_puts("DEBUG: Failed to attach backing store\n");
            return;
        }
        simple_serial_puts("DEBUG: Backing store attached\n");
        
        // Set scanout
        if (virtio_gpu_set_scanout(vigfx_gpu_resource_id) != 0)
        {
            simple_serial_puts("DEBUG: Failed to set scanout\n");
            return;
        }
        simple_serial_puts("DEBUG: Scanout set\n");
        
        vigfx_gpu_initialized = 1;
    }
}

// === INITIALIZATION & DEVICE MANAGEMENT ===
int vigfx_init_device(struct vigfx_device *dev)
{
    if (!dev)
        return -1;

    global_device = dev;
    simple_serial_puts("DEBUG: VirGFX device initialized\n");
    return 0;
}

// VirGFX registration function
void vigfx_virtio_gpu_register(void)
{
    simple_serial_puts("DEBUG: VirGFX VirtIO GPU registered\n");
}

void vigfx_shutdown_device(struct vigfx_device *dev)
{
    if (!dev)
        return;
    // Cleanup device resources
    global_device = NULL;
}

uint32_t vigfx_get_device_features(struct vigfx_device *dev)
{
    if (!dev)
        return 0;
    return VIGFX_FEATURE_RASTERIZATION | VIGFX_FEATURE_RAYTRACING |
           VIGFX_FEATURE_COMPUTE | VIGFX_FEATURE_PRESENT |
           VIGFX_FEATURE_DOUBLE_BUFFER | VIGFX_FEATURE_TESSELLATION |
           VIGFX_FEATURE_GEOMETRY_SHADER | VIGFX_FEATURE_MESH_SHADER;
}

int vigfx_enable_feature(struct vigfx_device *dev, uint32_t feature)
{
    if (!dev)
        return -1;
    // Enable specific feature
    return 0;
}

// === CONTEXT MANAGEMENT ===
struct vigfx_context *vigfx_create_context(struct vigfx_device *dev)
{
    // Allow NULL device for now, use global device if available
    if (!dev)
        dev = global_device;
    
    // If still no device, create a dummy one for basic functionality
    if (!dev)
    {
        simple_serial_puts("DEBUG: Creating context with no device (using dummy)\n");
    }

    struct vigfx_context *ctx = kmalloc(sizeof(struct vigfx_context));
    if (!ctx)
        return NULL;

    // Initialize context
    return ctx;
}

void vigfx_destroy_context(struct vigfx_context *ctx)
{
    if (!ctx)
        return;
    kfree(ctx);
}

int vigfx_make_context_current(struct vigfx_context *ctx)
{
    if (!ctx)
        return -1;
    return 0;
}

// === MEMORY MANAGEMENT ===
struct vigfx_memory_pool *vigfx_create_memory_pool(struct vigfx_device *dev,
                                                   vigfx_memory_type_t type,
                                                   size_t size)
{
    if (!dev || size == 0)
        return NULL;

    struct vigfx_memory_pool *pool = kmalloc(sizeof(struct vigfx_memory_pool));
    if (!pool)
        return NULL;

    // Initialize memory pool based on type
    return pool;
}

void vigfx_destroy_memory_pool(struct vigfx_memory_pool *pool)
{
    if (!pool)
        return;
    kfree(pool);
}

void *vigfx_allocate_memory(struct vigfx_memory_pool *pool, size_t size, size_t alignment)
{
    if (!pool || size == 0)
        return NULL;
    return kmalloc(size);
}

void vigfx_free_memory(struct vigfx_memory_pool *pool, void *ptr)
{
    if (!pool || !ptr)
        return;
    kfree(ptr);
}

// === BUFFER MANAGEMENT ===
struct vigfx_buffer *vigfx_create_buffer(struct vigfx_device *dev, size_t size, uint32_t usage)
{
    if (!dev || size == 0)
        return NULL;

    struct vigfx_buffer *buffer = kmalloc(sizeof(struct vigfx_buffer));
    if (!buffer)
        return NULL;

    // Allocate buffer data
    void *data = kmalloc(size);
    if (!data)
    {
        kfree(buffer);
        return NULL;
    }

    return buffer;
}

void vigfx_destroy_buffer(struct vigfx_buffer *buffer)
{
    if (!buffer)
        return;
    kfree(buffer);
}

void *vigfx_map_buffer(struct vigfx_buffer *buffer)
{
    if (!buffer)
        return NULL;
    return buffer; // Return buffer data pointer
}

void vigfx_unmap_buffer(struct vigfx_buffer *buffer)
{
    if (!buffer)
        return;
    // Unmap buffer
}

void vigfx_update_buffer(struct vigfx_buffer *buffer, const void *data, size_t size, size_t offset)
{
    if (!buffer || !data || size == 0)
        return;
    // Update buffer data
}

// === TEXTURE MANAGEMENT ===
struct vigfx_texture *vigfx_create_texture(struct vigfx_device *dev,
                                           uint32_t width, uint32_t height,
                                           vigfx_format_t format, uint32_t usage)
{
    if (!dev || width == 0 || height == 0)
        return NULL;

    struct vigfx_texture *texture = kmalloc(sizeof(struct vigfx_texture));
    if (!texture)
        return NULL;

    // Calculate texture size based on format
    size_t texture_size = width * height * 4; // Assuming 4 bytes per pixel
    void *texture_data = kmalloc(texture_size);
    if (!texture_data)
    {
        kfree(texture);
        return NULL;
    }

    return texture;
}

void vigfx_destroy_texture(struct vigfx_texture *texture)
{
    if (!texture)
        return;
    kfree(texture);
}

void vigfx_update_texture(struct vigfx_texture *texture, const void *data,
                          uint32_t width, uint32_t height, uint32_t layer)
{
    if (!texture || !data)
        return;
    // Update texture data
}

// === SHADER MANAGEMENT ===
struct vigfx_shader *vigfx_create_shader(struct vigfx_device *dev,
                                         vigfx_shader_type_t type,
                                         const void *code, size_t code_size)
{
    if (!dev || !code || code_size == 0)
        return NULL;

    struct vigfx_shader *shader = kmalloc(sizeof(struct vigfx_shader));
    if (!shader)
        return NULL;

    // Compile shader code
    return shader;
}

void vigfx_destroy_shader(struct vigfx_shader *shader)
{
    if (!shader)
        return;
    kfree(shader);
}

int vigfx_compile_shader(struct vigfx_shader *shader, const char *source)
{
    if (!shader || !source)
        return -1;
    // Compile shader from source
    return 0;
}

// === PIPELINE MANAGEMENT ===
struct vigfx_pipeline *vigfx_create_graphics_pipeline(struct vigfx_device *dev,
                                                      struct vigfx_shader *vertex_shader,
                                                      struct vigfx_shader *fragment_shader)
{
    if (!dev || !vertex_shader || !fragment_shader)
        return NULL;

    struct vigfx_pipeline *pipeline = kmalloc(sizeof(struct vigfx_pipeline));
    if (!pipeline)
        return NULL;

    // Create graphics pipeline
    return pipeline;
}

struct vigfx_pipeline *vigfx_create_compute_pipeline(struct vigfx_device *dev,
                                                     struct vigfx_shader *compute_shader)
{
    if (!dev || !compute_shader)
        return NULL;

    struct vigfx_pipeline *pipeline = kmalloc(sizeof(struct vigfx_pipeline));
    if (!pipeline)
        return NULL;

    // Create compute pipeline
    return pipeline;
}

struct vigfx_pipeline *vigfx_create_raytracing_pipeline(struct vigfx_device *dev,
                                                        struct vigfx_shader **shaders,
                                                        uint32_t shader_count)
{
    if (!dev || !shaders || shader_count == 0)
        return NULL;

    struct vigfx_pipeline *pipeline = kmalloc(sizeof(struct vigfx_pipeline));
    if (!pipeline)
        return NULL;

    // Create raytracing pipeline
    return pipeline;
}

void vigfx_destroy_pipeline(struct vigfx_pipeline *pipeline)
{
    if (!pipeline)
        return;
    kfree(pipeline);
}

// === RENDER TARGET MANAGEMENT ===
struct vigfx_render_target *vigfx_create_render_target(struct vigfx_device *dev,
                                                       uint32_t width, uint32_t height,
                                                       vigfx_format_t format)
{
    if (!dev || width == 0 || height == 0)
        return NULL;

    struct vigfx_render_target *target = kmalloc(sizeof(struct vigfx_render_target));
    if (!target)
        return NULL;

    // Create render target
    return target;
}

void vigfx_destroy_render_target(struct vigfx_render_target *target)
{
    if (!target)
        return;
    kfree(target);
}

// === COMMAND BUFFER MANAGEMENT ===
struct vigfx_command_buffer *vigfx_create_command_buffer(struct vigfx_device *dev)
{
    // Allow NULL device for now, use global device if available
    if (!dev)
        dev = global_device;
    
    // If still no device, create a dummy one for basic functionality
    if (!dev)
    {
        simple_serial_puts("DEBUG: Creating command buffer with no device (using dummy)\n");
    }

    struct vigfx_command_buffer *cmd_buffer = kmalloc(sizeof(struct vigfx_command_buffer));
    if (!cmd_buffer)
        return NULL;

    // Initialize command buffer
    return cmd_buffer;
}

void vigfx_destroy_command_buffer(struct vigfx_command_buffer *cmd_buffer)
{
    if (!cmd_buffer)
        return;
    kfree(cmd_buffer);
}

void vigfx_begin_command_buffer(struct vigfx_command_buffer *cmd_buffer)
{
    if (!cmd_buffer)
        return;
    // Begin recording commands
}

void vigfx_end_command_buffer(struct vigfx_command_buffer *cmd_buffer)
{
    if (!cmd_buffer)
        return;
    // End recording commands
}

// === RENDERING COMMANDS ===
void vigfx_cmd_bind_pipeline(struct vigfx_command_buffer *cmd_buffer,
                             struct vigfx_pipeline *pipeline)
{
    if (!cmd_buffer || !pipeline)
        return;
    // Bind pipeline
}

void vigfx_cmd_bind_vertex_buffer(struct vigfx_command_buffer *cmd_buffer,
                                  struct vigfx_buffer *buffer)
{
    if (!cmd_buffer || !buffer)
        return;
    // Bind vertex buffer
}

void vigfx_cmd_bind_index_buffer(struct vigfx_command_buffer *cmd_buffer,
                                 struct vigfx_buffer *buffer)
{
    if (!cmd_buffer || !buffer)
        return;
    // Bind index buffer
}

void vigfx_cmd_bind_texture(struct vigfx_command_buffer *cmd_buffer,
                            struct vigfx_texture *texture, uint32_t slot)
{
    if (!cmd_buffer || !texture)
        return;
    // Bind texture to slot
}

void vigfx_cmd_set_viewport(struct vigfx_command_buffer *cmd_buffer,
                            uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    if (!cmd_buffer)
        return;
    // Set viewport
}

// --- Update vigfx_cmd_clear to fill framebuffer ---
void vigfx_cmd_clear(struct vigfx_command_buffer *cmd_buffer, float r, float g, float b, float a)
{
    if (!vigfx_framebuffer)
        vigfx_init_framebuffer();
    uint8_t cr = (uint8_t)(r * 255.0f);
    uint8_t cg = (uint8_t)(g * 255.0f);
    uint8_t cb = (uint8_t)(b * 255.0f);
    uint8_t ca = (uint8_t)(a * 255.0f);
    
    simple_serial_puts("DEBUG: Clearing framebuffer with color RGBA: ");
    print_hex32(cr); simple_serial_puts(" ");
    print_hex32(cg); simple_serial_puts(" ");
    print_hex32(cb); simple_serial_puts(" ");
    print_hex32(ca); simple_serial_puts("\n");
    
    // Fill framebuffer with BGRA format (VirtIO GPU typically uses BGRA)
    for (uint32_t y = 0; y < vigfx_fb_height; ++y)
    {
        for (uint32_t x = 0; x < vigfx_fb_width; ++x)
        {
            uint32_t idx = (y * vigfx_fb_width + x) * VIGFX_FB_BPP;
            vigfx_framebuffer[idx + 0] = cb; // Blue
            vigfx_framebuffer[idx + 1] = cg; // Green
            vigfx_framebuffer[idx + 2] = cr; // Red
            vigfx_framebuffer[idx + 3] = ca; // Alpha
        }
    }
    simple_serial_puts("DEBUG: Framebuffer filled with color data\n");
}

// === TEXTURE MANAGEMENT ===
void vigfx_cmd_draw(struct vigfx_command_buffer *cmd_buffer,
                    uint32_t vertex_count, uint32_t instance_count)
{
    if (!cmd_buffer)
        return;
    // Draw vertices
}

void vigfx_cmd_draw_indexed(struct vigfx_command_buffer *cmd_buffer,
                            uint32_t index_count, uint32_t instance_count)
{
    if (!cmd_buffer)
        return;
    // Draw indexed vertices
}

// === COMPUTE COMMANDS ===
void vigfx_cmd_dispatch(struct vigfx_command_buffer *cmd_buffer,
                        uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z)
{
    if (!cmd_buffer)
        return;
    // Dispatch compute shader
}

// === RAY TRACING COMMANDS ===
void vigfx_cmd_trace_rays(struct vigfx_command_buffer *cmd_buffer,
                          uint32_t width, uint32_t height, uint32_t depth)
{
    if (!cmd_buffer)
        return;
    // Trace rays
}

void vigfx_cmd_build_acceleration_structure(struct vigfx_command_buffer *cmd_buffer,
                                            struct vigfx_acceleration_structure *as)
{
    if (!cmd_buffer || !as)
        return;
    // Build acceleration structure for ray tracing
}

// === SYNCHRONIZATION ===
void vigfx_cmd_barrier(struct vigfx_command_buffer *cmd_buffer)
{
    if (!cmd_buffer)
        return;
    // Memory barrier
}

void vigfx_device_wait_idle(struct vigfx_device *dev)
{
    if (!dev)
        return;
    // Wait for device to be idle
}

void vigfx_context_wait_idle(struct vigfx_context *ctx)
{
    if (!ctx)
        return;
    // Wait for context to be idle
}

// === COMMAND EXECUTION ===
void vigfx_submit_command_buffer(struct vigfx_context *ctx,
                                 struct vigfx_command_buffer *cmd_buffer)
{
    if (!ctx || !cmd_buffer)
        return;
    // Submit command buffer for execution
    simple_serial_puts("DEBUG: VirGFX submitting command buffer\n");
}

void vigfx_flush_commands(struct vigfx_context *ctx)
{
    if (!ctx)
        return;
    // Flush pending commands
}

// --- Update vigfx_present to send framebuffer to GPU ---
struct virtio_gpu_transfer_to_host_2d_cmd
{
    uint32_t type;
    uint32_t resource_id;
    uint32_t rect[4];
    uint64_t offset;
};

void vigfx_present(struct vigfx_context *ctx, struct vigfx_render_target *target)
{
    if (!vigfx_framebuffer)
    {
        simple_serial_puts("DEBUG: vigfx_present: no framebuffer\n");
        return;
    }
    if (!vigfx_gpu_initialized)
    {
        simple_serial_puts("DEBUG: vigfx_present: GPU not initialized\n");
        return;
    }
    struct virtio_gpu_transfer_to_host_2d_cmd cmd = {0};
    cmd.type = 0x0300;   // VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D
    cmd.resource_id = vigfx_gpu_resource_id; // Use the actual resource ID
    cmd.rect[0] = 0;
    cmd.rect[1] = 0;
    cmd.rect[2] = vigfx_fb_width;
    cmd.rect[3] = vigfx_fb_height;
    cmd.offset = 0;
    virtio_gpu_pci_send_command(&cmd, sizeof(cmd), NULL, 0);
    simple_serial_puts("DEBUG: vigfx_present: framebuffer sent to GPU\n");
    
    // Flush the resource to ensure it's displayed
    virtio_gpu_flush_resource(vigfx_gpu_resource_id);
    simple_serial_puts("DEBUG: vigfx_present: resource flushed\n");
}

void vigfx_swap_buffers(struct vigfx_context *ctx)
{
    if (!ctx)
        return;
    // Swap front and back buffers
}

// === DEBUGGING & PROFILING ===
void vigfx_set_debug_label(struct vigfx_device *dev, void *object, const char *label)
{
    if (!dev || !object || !label)
        return;
    // Set debug label for object
}

void vigfx_begin_debug_region(struct vigfx_command_buffer *cmd_buffer, const char *name)
{
    if (!cmd_buffer || !name)
        return;
    // Begin debug region
}

void vigfx_end_debug_region(struct vigfx_command_buffer *cmd_buffer)
{
    if (!cmd_buffer)
        return;
    // End debug region
}

void vigfx_get_performance_counters(struct vigfx_device *dev, uint64_t *counters)
{
    if (!dev || !counters)
        return;
    // Get performance counters
}

// === ERROR HANDLING ===
vigfx_result_t vigfx_get_last_error(struct vigfx_device *dev)
{
    if (!dev)
        return VIGFX_ERROR_INVALID_PARAMETER;
    return VIGFX_SUCCESS;
}

const char *vigfx_get_error_string(vigfx_result_t error)
{
    switch (error)
    {
    case VIGFX_SUCCESS:
        return "Success";
    case VIGFX_ERROR_DEVICE_LOST:
        return "Device lost";
    case VIGFX_ERROR_OUT_OF_MEMORY:
        return "Out of memory";
    case VIGFX_ERROR_INVALID_PARAMETER:
        return "Invalid parameter";
    case VIGFX_ERROR_FEATURE_NOT_SUPPORTED:
        return "Feature not supported";
    case VIGFX_ERROR_SHADER_COMPILE_FAILED:
        return "Shader compile failed";
    case VIGFX_ERROR_TIMEOUT:
        return "Timeout";
    default:
        return "Unknown error";
    }
}

// Legacy compatibility functions
void vigfx_init(struct vigfx_device *dev)
{
    vigfx_init_device(dev);
}

void vigfx_process(struct vigfx_context *ctx)
{
    if (!ctx)
        return;
    simple_serial_puts("DEBUG: VirGFX processing GPU commands\n");
}
