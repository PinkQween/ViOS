#ifndef VIRTIO_GPU_H
#define VIRTIO_GPU_H

#include <stdint.h>
#include <stddef.h>

// Forward declarations
struct virtio_gpu_device;
struct virtio_gpu_context;
struct virtio_gpu_buffer;
struct virtio_gpu_command_buffer;
struct virtio_gpu_shader;
struct virtio_gpu_pipeline;
struct virtio_gpu_texture;
struct virtio_gpu_render_target;
struct virtio_gpu_memory_pool;
struct virtio_gpu_acceleration_structure;

typedef enum
{
    VIRTIO_GPU_SUCCESS = 0,
    VIRTIO_GPU_ERROR_DEVICE_LOST,
    VIRTIO_GPU_ERROR_OUT_OF_MEMORY,
    VIRTIO_GPU_ERROR_INVALID_PARAMETER,
    VIRTIO_GPU_ERROR_UNSUPPORTED,
    VIRTIO_GPU_ERROR_SHADER_COMPILATION,
    VIRTIO_GPU_ERROR_TIMEOUT,
} virtio_gpu_result_t;

typedef enum
{
    VIRTIO_GPU_SHADER_VERTEX,
    VIRTIO_GPU_SHADER_FRAGMENT,
    VIRTIO_GPU_SHADER_COMPUTE,
    VIRTIO_GPU_SHADER_RAYGEN,
    VIRTIO_GPU_SHADER_MISS,
    VIRTIO_GPU_SHADER_CLOSEST_HIT,
} virtio_gpu_shader_type_t;

typedef enum
{
    VIRTIO_GPU_FORMAT_RGBA8,
    VIRTIO_GPU_FORMAT_RGB8,
    VIRTIO_GPU_FORMAT_DEPTH24,
} virtio_gpu_format_t;

typedef enum
{
    VIRTIO_GPU_MEM_DEVICE_LOCAL,
    VIRTIO_GPU_MEM_HOST_VISIBLE,
    VIRTIO_GPU_MEM_HOST_COHERENT,
} virtio_gpu_memory_type_t;

enum
{
    VIRTIO_GPU_FEATURE_3D = 1 << 0,
    VIRTIO_GPU_FEATURE_MSE = 1 << 1,
    VIRTIO_GPU_FEATURE_EDID = 1 << 2,
    VIRTIO_GPU_FEATURE_RESOURCE = 1 << 3,
    VIRTIO_GPU_FEATURE_CONTEXT = 1 << 4,
};

// === Device Management ===

int virtio_gpu_init_device(struct virtio_gpu_device *dev);
void virtio_gpu_shutdown_device(struct virtio_gpu_device *dev);
uint32_t virtio_gpu_get_features(struct virtio_gpu_device *dev);
int virtio_gpu_enable_feature(struct virtio_gpu_device *dev, uint32_t feature);

// === Context Management ===

struct virtio_gpu_context *virtio_gpu_create_context(struct virtio_gpu_device *dev);
void virtio_gpu_destroy_context(struct virtio_gpu_context *ctx);
int virtio_gpu_make_context_current(struct virtio_gpu_context *ctx);

// === Memory Management ===

struct virtio_gpu_memory_pool *virtio_gpu_create_memory_pool(struct virtio_gpu_device *dev, virtio_gpu_memory_type_t type, size_t size);
void virtio_gpu_destroy_memory_pool(struct virtio_gpu_memory_pool *pool);
void *virtio_gpu_allocate_memory(struct virtio_gpu_memory_pool *pool, size_t size, size_t alignment);
void virtio_gpu_free_memory(struct virtio_gpu_memory_pool *pool, void *ptr);

// === Buffer Management ===

struct virtio_gpu_buffer *virtio_gpu_create_buffer(struct virtio_gpu_device *dev, size_t size, uint32_t usage);
void virtio_gpu_destroy_buffer(struct virtio_gpu_buffer *buffer);
void *virtio_gpu_map_buffer(struct virtio_gpu_buffer *buffer);
void virtio_gpu_unmap_buffer(struct virtio_gpu_buffer *buffer);
void virtio_gpu_update_buffer(struct virtio_gpu_buffer *buffer, const void *data, size_t size, size_t offset);

// === Texture Management ===

struct virtio_gpu_texture *virtio_gpu_create_texture(struct virtio_gpu_device *dev, uint32_t width, uint32_t height, virtio_gpu_format_t format, uint32_t usage);
void virtio_gpu_destroy_texture(struct virtio_gpu_texture *texture);
void virtio_gpu_update_texture(struct virtio_gpu_texture *texture, const void *data, uint32_t width, uint32_t height, uint32_t layer);

// === Shader Management ===

struct virtio_gpu_shader *virtio_gpu_create_shader(struct virtio_gpu_device *dev, virtio_gpu_shader_type_t type, const void *code, size_t code_size);
void virtio_gpu_destroy_shader(struct virtio_gpu_shader *shader);
int virtio_gpu_compile_shader(struct virtio_gpu_shader *shader, const char *source);

// === Pipeline Management ===

struct virtio_gpu_pipeline *virtio_gpu_create_graphics_pipeline(struct virtio_gpu_device *dev, struct virtio_gpu_shader *vertex_shader, struct virtio_gpu_shader *fragment_shader);
struct virtio_gpu_pipeline *virtio_gpu_create_compute_pipeline(struct virtio_gpu_device *dev, struct virtio_gpu_shader *compute_shader);
struct virtio_gpu_pipeline *virtio_gpu_create_raytracing_pipeline(struct virtio_gpu_device *dev, struct virtio_gpu_shader **shaders, uint32_t shader_count);
void virtio_gpu_destroy_pipeline(struct virtio_gpu_pipeline *pipeline);

// === Render Target Management ===

struct virtio_gpu_render_target *virtio_gpu_create_render_target(struct virtio_gpu_device *dev, uint32_t width, uint32_t height, virtio_gpu_format_t format);
void virtio_gpu_destroy_render_target(struct virtio_gpu_render_target *target);

// === Command Buffer Management ===

struct virtio_gpu_command_buffer *virtio_gpu_create_command_buffer(struct virtio_gpu_device *dev);
void virtio_gpu_destroy_command_buffer(struct virtio_gpu_command_buffer *cmd_buffer);
void virtio_gpu_begin_command_buffer(struct virtio_gpu_command_buffer *cmd_buffer);
void virtio_gpu_end_command_buffer(struct virtio_gpu_command_buffer *cmd_buffer);

// === Rendering Commands ===

void virtio_gpu_cmd_bind_pipeline(struct virtio_gpu_command_buffer *cmd_buffer, struct virtio_gpu_pipeline *pipeline);
void virtio_gpu_cmd_bind_vertex_buffer(struct virtio_gpu_command_buffer *cmd_buffer, struct virtio_gpu_buffer *buffer);
void virtio_gpu_cmd_bind_index_buffer(struct virtio_gpu_command_buffer *cmd_buffer, struct virtio_gpu_buffer *buffer);
void virtio_gpu_cmd_bind_texture(struct virtio_gpu_command_buffer *cmd_buffer, struct virtio_gpu_texture *texture, uint32_t slot);
void virtio_gpu_cmd_set_viewport(struct virtio_gpu_command_buffer *cmd_buffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void virtio_gpu_cmd_clear(struct virtio_gpu_command_buffer *cmd_buffer, float r, float g, float b, float a);
void virtio_gpu_cmd_draw(struct virtio_gpu_command_buffer *cmd_buffer, uint32_t vertex_count, uint32_t instance_count);
void virtio_gpu_cmd_draw_indexed(struct virtio_gpu_command_buffer *cmd_buffer, uint32_t index_count, uint32_t instance_count);

// === Compute Commands ===

void virtio_gpu_cmd_dispatch(struct virtio_gpu_command_buffer *cmd_buffer, uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z);

// === Ray Tracing Commands ===

void virtio_gpu_cmd_trace_rays(struct virtio_gpu_command_buffer *cmd_buffer, uint32_t width, uint32_t height, uint32_t depth);
void virtio_gpu_cmd_build_acceleration_structure(struct virtio_gpu_command_buffer *cmd_buffer, struct virtio_gpu_acceleration_structure *as);

// === Synchronization ===

void virtio_gpu_cmd_barrier(struct virtio_gpu_command_buffer *cmd_buffer);
void virtio_gpu_device_wait_idle(struct virtio_gpu_device *dev);
void virtio_gpu_context_wait_idle(struct virtio_gpu_context *ctx);

// === Command Execution ===

void virtio_gpu_submit_command_buffer(struct virtio_gpu_context *ctx, struct virtio_gpu_command_buffer *cmd_buffer);
void virtio_gpu_flush_commands(struct virtio_gpu_context *ctx);

// === Presentation ===

void virtio_gpu_present(struct virtio_gpu_context *ctx, struct virtio_gpu_render_target *target);
void virtio_gpu_swap_buffers(struct virtio_gpu_context *ctx);

// === Debugging & Profiling ===

void virtio_gpu_set_debug_label(struct virtio_gpu_device *dev, void *object, const char *label);
void virtio_gpu_begin_debug_region(struct virtio_gpu_command_buffer *cmd_buffer, const char *name);
void virtio_gpu_end_debug_region(struct virtio_gpu_command_buffer *cmd_buffer);
void virtio_gpu_get_performance_counters(struct virtio_gpu_device *dev, uint64_t *counters);

// === Error Handling ===

virtio_gpu_result_t virtio_gpu_get_last_error(struct virtio_gpu_device *dev);
const char *virtio_gpu_get_error_string(virtio_gpu_result_t error);

// === Resource Management ===

int virtio_gpu_create_resource(void);
int virtio_gpu_attach_backing(int resource_id, void *framebuffer, size_t size);
int virtio_gpu_set_scanout(int resource_id);
int virtio_gpu_flush_resource(int resource_id);

#endif // VIRTIO_GPU_H
