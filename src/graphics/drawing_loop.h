#ifndef DRAWING_LOOP_H
#define DRAWING_LOOP_H

#include <stdint.h>
#include <stdbool.h>
#include "graphics/graphics.h"

// Drawing loop constants
#define DRAWING_LOOP_MAX_LAYERS 32
#define DRAWING_LOOP_MAX_UPDATES 256
#define DRAWING_LOOP_TARGET_FPS 60
#define DRAWING_LOOP_FRAME_TIME_MS (1000 / DRAWING_LOOP_TARGET_FPS)

// Drawing priority levels
typedef enum {
    DRAWING_PRIORITY_BACKGROUND = 0,
    DRAWING_PRIORITY_NORMAL = 1,
    DRAWING_PRIORITY_UI = 2,
    DRAWING_PRIORITY_CURSOR = 3,
    DRAWING_PRIORITY_DEBUG = 4
} DrawingPriority;

// Drawing operation types
typedef enum {
    DRAWING_OP_CLEAR,
    DRAWING_OP_PIXEL,
    DRAWING_OP_LINE,
    DRAWING_OP_RECT,
    DRAWING_OP_FILLED_RECT,
    DRAWING_OP_CIRCLE,
    DRAWING_OP_FILLED_CIRCLE,
    DRAWING_OP_TEXT,
    DRAWING_OP_SURFACE_BLIT,
    DRAWING_OP_CUSTOM
} DrawingOperationType;

// Forward declarations
struct DrawingLayer;
struct DrawingUpdate;
struct DrawingContext;

// Custom drawing function typedef
typedef void (*DrawingCustomFunction)(struct DrawingContext *ctx, void *data);

// Drawing operation structure
typedef struct DrawingOperation {
    DrawingOperationType type;
    DrawingPriority priority;
    Rectangle bounds;           // Bounding box for dirty region tracking
    bool visible;
    uint32_t timestamp;
    
    union {
        struct {
            Color color;
        } clear;
        
        struct {
            Point position;
            Color color;
        } pixel;
        
        struct {
            Point start;
            Point end;
            Color color;
        } line;
        
        struct {
            Rectangle rect;
            Color color;
            bool filled;
        } rect;
        
        struct {
            Point center;
            int radius;
            Color color;
            bool filled;
        } circle;
        
        struct {
            Point position;
            char text[256];
            Color color;
            int scale;
        } text;
        
        struct {
            GraphicsSurface *surface;
            Rectangle src_rect;
            Point dest_point;
            uint8_t alpha;
        } blit;
        
        struct {
            DrawingCustomFunction function;
            void *data;
        } custom;
    };
} DrawingOperation;

// Drawing layer structure
typedef struct DrawingLayer {
    uint32_t layer_id;
    DrawingPriority priority;
    bool visible;
    bool dirty;
    Rectangle bounds;
    GraphicsSurface *surface;
    
    // Operations queue for this layer
    DrawingOperation operations[DRAWING_LOOP_MAX_UPDATES];
    int operation_count;
    
    // Layer properties
    uint8_t opacity;
    bool clip_to_bounds;
    
    // Linked list pointers
    struct DrawingLayer *next;
    struct DrawingLayer *prev;
} DrawingLayer;

// Drawing update structure for dirty region tracking
typedef struct DrawingUpdate {
    Rectangle region;
    uint32_t timestamp;
    DrawingPriority min_priority;
    bool processed;
} DrawingUpdate;

// Drawing context structure
typedef struct DrawingContext {
    // Core graphics context
    GraphicsContext *graphics_ctx;
    
    // Layer management
    DrawingLayer *layers[DRAWING_LOOP_MAX_LAYERS];
    DrawingLayer *layer_head;
    int layer_count;
    uint32_t next_layer_id;
    
    // Update tracking
    DrawingUpdate updates[DRAWING_LOOP_MAX_UPDATES];
    int update_count;
    Rectangle dirty_regions[DRAWING_LOOP_MAX_UPDATES];
    int dirty_region_count;
    
    // Compositing surfaces
    GraphicsSurface *composite_surface;
    GraphicsSurface *temp_surface;
    
    // Loop state
    bool running;
    bool vsync_enabled;
    uint32_t frame_count;
    uint32_t last_frame_time;
    
    // Performance metrics
    uint32_t render_time_ms;
    uint32_t composite_time_ms;
    uint32_t present_time_ms;
    uint32_t total_operations;
    
    // Debug options
    bool show_dirty_regions;
    bool show_layer_bounds;
    bool show_performance_overlay;
    
} DrawingContext;

// =================== CORE DRAWING LOOP API ===================

// Initialize the drawing loop system
bool drawing_loop_init(void);

// Shutdown the drawing loop system
void drawing_loop_shutdown(void);

// Start the main drawing loop (blocks until stopped)
void drawing_loop_run(void);

// Stop the drawing loop
void drawing_loop_stop(void);

// Force a single frame update
void drawing_loop_update_frame(void);

// =================== LAYER MANAGEMENT ===================

// Create a new drawing layer
DrawingLayer* drawing_layer_create(DrawingPriority priority, Rectangle bounds);

// Destroy a drawing layer
void drawing_layer_destroy(DrawingLayer *layer);

// Show/hide a layer
void drawing_layer_set_visible(DrawingLayer *layer, bool visible);

// Set layer opacity (0-255)
void drawing_layer_set_opacity(DrawingLayer *layer, uint8_t opacity);

// Move layer to different priority
void drawing_layer_set_priority(DrawingLayer *layer, DrawingPriority priority);

// Get layer by ID
DrawingLayer* drawing_layer_get(uint32_t layer_id);

// =================== DRAWING OPERATIONS ===================

// Add drawing operations to layers
void drawing_layer_clear(DrawingLayer *layer, Color color);
void drawing_layer_draw_pixel(DrawingLayer *layer, Point position, Color color);
void drawing_layer_draw_line(DrawingLayer *layer, Point start, Point end, Color color);
void drawing_layer_draw_rect(DrawingLayer *layer, Rectangle rect, Color color);
void drawing_layer_fill_rect(DrawingLayer *layer, Rectangle rect, Color color);
void drawing_layer_draw_circle(DrawingLayer *layer, Point center, int radius, Color color);
void drawing_layer_fill_circle(DrawingLayer *layer, Point center, int radius, Color color);
void drawing_layer_draw_text(DrawingLayer *layer, Point position, const char *text, Color color, int scale);
void drawing_layer_blit_surface(DrawingLayer *layer, GraphicsSurface *surface, Rectangle src_rect, Point dest_point);
void drawing_layer_custom_draw(DrawingLayer *layer, DrawingCustomFunction function, void *data, Rectangle bounds);

// =================== DIRTY REGION MANAGEMENT ===================

// Mark a region as dirty for redrawing
void drawing_mark_dirty(Rectangle region);

// Mark entire screen as dirty
void drawing_mark_all_dirty(void);

// Clear all dirty regions
void drawing_clear_dirty_regions(void);

// =================== PERFORMANCE AND DEBUG ===================

// Get current FPS
uint32_t drawing_loop_get_fps(void);

// Get performance statistics
void drawing_loop_get_stats(uint32_t *render_time, uint32_t *composite_time, uint32_t *present_time);

// Enable/disable debug overlays
void drawing_loop_set_debug_overlay(bool show_dirty, bool show_bounds, bool show_performance);

// =================== UTILITY FUNCTIONS ===================

// Check if two rectangles intersect
bool drawing_rect_intersect(Rectangle a, Rectangle b);

// Merge two rectangles into bounding box
Rectangle drawing_rect_union(Rectangle a, Rectangle b);

// Get current drawing context
DrawingContext* drawing_get_context(void);

// Internal functions (not for external use)
void _drawing_loop_render_layers(void);
void _drawing_loop_composite_frame(void);
void _drawing_loop_present_frame(void);
void _drawing_loop_process_updates(void);
void _drawing_loop_optimize_dirty_regions(void);
bool _drawing_operation_intersects_region(DrawingOperation *op, Rectangle region);

#endif // DRAWING_LOOP_H
