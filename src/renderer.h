#ifndef RENDERER_H
#define RENDERER_H

#include "splat.h"
#include "camera.h"

// Declare DebugMode enum here
typedef enum {
    DEBUG_NONE = 0,
    DEBUG_TRANSFORM = 1,
    DEBUG_PROJECTION = 2,
    DEBUG_RENDERING = 4
} DebugMode;

// Update Renderer struct in renderer.h
typedef struct {
    unsigned char* framebuffer;
    float* depthbuffer;
    int width;
    int height;
    unsigned int texture;
    unsigned int shaderProgram;  // Add this
    unsigned int VAO, VBO, EBO;  // Add these for rendering
} Renderer;

void init_renderer(Renderer* renderer, int width, int height);
void free_renderer(Renderer* renderer);

// Update the declaration to match the definition with DebugMode parameter
void render_scene(Renderer* renderer, Splat* splats, int splat_count, Camera* camera, DebugMode debug_mode, int debug_limit);

void draw_fullscreen_quad(Renderer* renderer);

#endif
