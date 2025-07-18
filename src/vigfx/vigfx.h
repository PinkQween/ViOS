#ifndef VIGFX_H
#define VIGFX_H

#include <stddef.h>
#include <stdint.h>

// === ENUMS & FLAGS ===

typedef enum
{
    VIGFX_SUCCESS = 0,
    VIGFX_ERROR_DEVICE_LOST,
    VIGFX_ERROR_OUT_OF_MEMORY,
    VIGFX_ERROR_INVALID_PARAMETER,
    VIGFX_ERROR_FEATURE_NOT_SUPPORTED,
    VIGFX_ERROR_SHADER_COMPILE_FAILED,
    VIGFX_ERROR_TIMEOUT,
} vigfx_result_t;

typedef enum
{
    VIGFX_SHADER_VERTEX,
    VIGFX_SHADER_FRAGMENT,
    VIGFX_SHADER_COMPUTE,
    VIGFX_SHADER_RAYGEN,
    VIGFX_SHADER_MISS,
    VIGFX_SHADER_CLOSEST_HIT,
} vigfx_shader_type_t;

typedef enum
{
    VIGFX_FORMAT_RGBA8,
    VIGFX_FORMAT_RGB8,
    VIGFX_FORMAT_DEPTH24,
} vigfx_format_t;

typedef enum
{
    VIGFX_MEMORY_DEVICE_LOCAL,
    VIGFX_MEMORY_HOST_VISIBLE,
    VIGFX_MEMORY_HOST_COHERENT,
} vigfx_memory_type_t;

enum
{
    VIGFX_FEATURE_RASTERIZATION = 1 << 0,
    VIGFX_FEATURE_RAYTRACING = 1 << 1,
    VIGFX_FEATURE_COMPUTE = 1 << 2,
    VIGFX_FEATURE_PRESENT = 1 << 3,
    VIGFX_FEATURE_DOUBLE_BUFFER = 1 << 4,
    VIGFX_FEATURE_TESSELLATION = 1 << 5,
    VIGFX_FEATURE_GEOMETRY_SHADER = 1 << 6,
    VIGFX_FEATURE_MESH_SHADER = 1 << 7,
};

// === FORWARD STRUCT DECLARATIONS ===

struct vigfx_device;
struct vigfx_context;
struct vigfx_memory_pool;
struct vigfx_buffer;
struct vigfx_texture;
struct vigfx_shader;
struct vigfx_pipeline;
struct vigfx_render_target;
struct vigfx_command_buffer;
struct vigfx_acceleration_structure;

// === INITIALIZATION ===

void vigfx_virtio_gpu_register(void);
int vigfx_init_device(struct vigfx_device *dev);
void vigfx_shutdown_device(struct vigfx_device *dev);
uint32_t vigfx_get_device_features(struct vigfx_device *dev);
int vigfx_enable_feature(struct vigfx_device *dev, uint32_t feature);

// === CONTEXT MANAGEMENT ===

struct vigfx_context *vigfx_create_context(struct vigfx_device *dev);
void vigfx_destroy_context(struct vigfx_context *ctx);
int vigfx_make_context_current(struct vigfx_context *ctx);

// === MEMORY MANAGEMENT ===

struct vigfx_memory_pool *vigfx_create_memory_pool(struct vigfx_device *dev, vigfx_memory_type_t type, size_t size);
void vigfx_destroy_memory_pool(struct vigfx_memory_pool *pool);
void *vigfx_allocate_memory(struct vigfx_memory_pool *pool, size_t size, size_t alignment);
void vigfx_free_memory(struct vigfx_memory_pool *pool, void *ptr);

// === BUFFER MANAGEMENT ===

struct vigfx_buffer *vigfx_create_buffer(struct vigfx_device *dev, size_t size, uint32_t usage);
void vigfx_destroy_buffer(struct vigfx_buffer *buffer);
void *vigfx_map_buffer(struct vigfx_buffer *buffer);
void vigfx_unmap_buffer(struct vigfx_buffer *buffer);
void vigfx_update_buffer(struct vigfx_buffer *buffer, const void *data, size_t size, size_t offset);

// === TEXTURE MANAGEMENT ===

struct vigfx_texture *vigfx_create_texture(struct vigfx_device *dev, uint32_t width, uint32_t height, vigfx_format_t format, uint32_t usage);
void vigfx_destroy_texture(struct vigfx_texture *texture);
void vigfx_update_texture(struct vigfx_texture *texture, const void *data, uint32_t width, uint32_t height, uint32_t layer);

// === SHADER MANAGEMENT ===

struct vigfx_shader *vigfx_create_shader(struct vigfx_device *dev, vigfx_shader_type_t type, const void *code, size_t code_size);
void vigfx_destroy_shader(struct vigfx_shader *shader);
int vigfx_compile_shader(struct vigfx_shader *shader, const char *source);

// === PIPELINE MANAGEMENT ===

struct vigfx_pipeline *vigfx_create_graphics_pipeline(struct vigfx_device *dev, struct vigfx_shader *vertex_shader, struct vigfx_shader *fragment_shader);
struct vigfx_pipeline *vigfx_create_compute_pipeline(struct vigfx_device *dev, struct vigfx_shader *compute_shader);
struct vigfx_pipeline *vigfx_create_raytracing_pipeline(struct vigfx_device *dev, struct vigfx_shader **shaders, uint32_t shader_count);
void vigfx_destroy_pipeline(struct vigfx_pipeline *pipeline);

// === RENDER TARGET MANAGEMENT ===

struct vigfx_render_target *vigfx_create_render_target(struct vigfx_device *dev, uint32_t width, uint32_t height, vigfx_format_t format);
void vigfx_destroy_render_target(struct vigfx_render_target *target);

// === COMMAND BUFFER MANAGEMENT ===

struct vigfx_command_buffer *vigfx_create_command_buffer(struct vigfx_device *dev);
void vigfx_destroy_command_buffer(struct vigfx_command_buffer *cmd_buffer);
void vigfx_begin_command_buffer(struct vigfx_command_buffer *cmd_buffer);
void vigfx_end_command_buffer(struct vigfx_command_buffer *cmd_buffer);

// === RENDERING COMMANDS ===

void vigfx_cmd_bind_pipeline(struct vigfx_command_buffer *cmd_buffer, struct vigfx_pipeline *pipeline);
void vigfx_cmd_bind_vertex_buffer(struct vigfx_command_buffer *cmd_buffer, struct vigfx_buffer *buffer);
void vigfx_cmd_bind_index_buffer(struct vigfx_command_buffer *cmd_buffer, struct vigfx_buffer *buffer);
void vigfx_cmd_bind_texture(struct vigfx_command_buffer *cmd_buffer, struct vigfx_texture *texture, uint32_t slot);
void vigfx_cmd_set_viewport(struct vigfx_command_buffer *cmd_buffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void vigfx_cmd_clear(struct vigfx_command_buffer *cmd_buffer, float r, float g, float b, float a);
void vigfx_cmd_draw(struct vigfx_command_buffer *cmd_buffer, uint32_t vertex_count, uint32_t instance_count);
void vigfx_cmd_draw_indexed(struct vigfx_command_buffer *cmd_buffer, uint32_t index_count, uint32_t instance_count);

// === COMPUTE COMMANDS ===

void vigfx_cmd_dispatch(struct vigfx_command_buffer *cmd_buffer, uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z);

// === RAY TRACING COMMANDS ===

void vigfx_cmd_trace_rays(struct vigfx_command_buffer *cmd_buffer, uint32_t width, uint32_t height, uint32_t depth);
void vigfx_cmd_build_acceleration_structure(struct vigfx_command_buffer *cmd_buffer, struct vigfx_acceleration_structure *as);

// === SYNCHRONIZATION ===

void vigfx_cmd_barrier(struct vigfx_command_buffer *cmd_buffer);
void vigfx_device_wait_idle(struct vigfx_device *dev);
void vigfx_context_wait_idle(struct vigfx_context *ctx);

// === COMMAND EXECUTION ===

void vigfx_submit_command_buffer(struct vigfx_context *ctx, struct vigfx_command_buffer *cmd_buffer);
void vigfx_flush_commands(struct vigfx_context *ctx);

// === PRESENTATION ===

void vigfx_present(struct vigfx_context *ctx, struct vigfx_render_target *target);
void vigfx_swap_buffers(struct vigfx_context *ctx);

// === DEBUGGING & PROFILING ===

void vigfx_set_debug_label(struct vigfx_device *dev, void *object, const char *label);
void vigfx_begin_debug_region(struct vigfx_command_buffer *cmd_buffer, const char *name);
void vigfx_end_debug_region(struct vigfx_command_buffer *cmd_buffer);
void vigfx_get_performance_counters(struct vigfx_device *dev, uint64_t *counters);

// === ERROR HANDLING ===

vigfx_result_t vigfx_get_last_error(struct vigfx_device *dev);
const char *vigfx_get_error_string(vigfx_result_t error);

// === LEGACY ===

void vigfx_init(struct vigfx_device *dev);
void vigfx_process(struct vigfx_context *ctx);

#endif // VIGFX_H