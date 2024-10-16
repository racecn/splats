// File: src/renderer.c
#include "renderer.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <glad/glad.h>
#include <stdio.h>

const char* vertex_shader_source = "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "out vec2 TexCoord;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
    "   TexCoord = aTexCoord;\n"
    "}\0";

const char* fragment_shader_source = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec2 TexCoord;\n"
    "uniform sampler2D ourTexture;\n"
    "void main()\n"
    "{\n"
    "   FragColor = texture(ourTexture, TexCoord);\n"
    "}\0";

void check_shader_compilation(unsigned int shader, const char* type) {
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("%s shader compilation failed: %s\n", type, infoLog);
    }
}
void init_renderer(Renderer* renderer, int width, int height) {
    // Initialize renderer parameters
    renderer->width = width;
    renderer->height = height;

    // Allocate memory for framebuffer and depthbuffer
    renderer->framebuffer = (unsigned char*)malloc(width * height * 3 * sizeof(unsigned char));
    renderer->depthbuffer = (float*)malloc(width * height * sizeof(float));
    if (!renderer->framebuffer || !renderer->depthbuffer) {
        printf("Error: Failed to allocate memory for framebuffer or depthbuffer.\n");
        exit(EXIT_FAILURE);
    }

    // Generate and configure the texture
    glGenTextures(1, &renderer->texture);
    glBindTexture(GL_TEXTURE_2D, renderer->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Check for texture creation errors
    if (!renderer->texture) {
        printf("Error: Failed to generate OpenGL texture.\n");
        exit(EXIT_FAILURE);
    }

    // Create and compile shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertexShader);
    check_shader_compilation(vertexShader, "Vertex");

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragmentShader);
    check_shader_compilation(fragmentShader, "Fragment");

    // Create and link the shader program
    renderer->shaderProgram = glCreateProgram();
    glAttachShader(renderer->shaderProgram, vertexShader);
    glAttachShader(renderer->shaderProgram, fragmentShader);
    glLinkProgram(renderer->shaderProgram);

    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(renderer->shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(renderer->shaderProgram, 512, NULL, infoLog);
        printf("Error: Shader program linking failed: %s\n", infoLog);
        exit(EXIT_FAILURE);
    }

    // Validate the shader program
    if (!glIsProgram(renderer->shaderProgram)) {
        printf("Error: Shader program is not valid.\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Shader program is valid.\n");
    }

    // Clean up shaders as they are linked into the program
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Set up vertex data (fullscreen quad) and buffers
    float vertices[] = {
        // Positions   // Texture coordinates
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    // Generate and bind vertex array and buffer objects
    glGenVertexArrays(1, &renderer->VAO);
    glGenBuffers(1, &renderer->VBO);
    glGenBuffers(1, &renderer->EBO);

    glBindVertexArray(renderer->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind VBO and VAO to prevent accidental modification
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Ensure OpenGL errors are handled
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("OpenGL error during renderer initialization: 0x%x\n", error);
        exit(EXIT_FAILURE);
    }
}


void clear_renderer(Renderer* renderer) {
    memset(renderer->framebuffer, 0, renderer->width * renderer->height * 3 * sizeof(unsigned char));
    for (int i = 0; i < renderer->width * renderer->height; i++) {
        renderer->depthbuffer[i] = INFINITY;
    }
}
#include <stdbool.h> // For true/false usage

#include <stdio.h>
#include <float.h>  // Add this to resolve FLT_MAX
#include <math.h>   // Ensure you have this for math functions like fminf, fmaxf

#include <immintrin.h>  // For SSE intrinsics
#include <omp.h>        // For OpenMP parallelization

void render_scene(Renderer* renderer, Splat* splats, int splat_count, Camera* camera, DebugMode debug_mode, int debug_limit) {
    // Clear the framebuffer and depthbuffer efficiently using memset
    memset(renderer->framebuffer, 0, renderer->width * renderer->height * 3 * sizeof(unsigned char));
    
    // Use SSE to initialize the depth buffer with INFINITY
    __m128 inf = _mm_set1_ps(INFINITY);
    for (int i = 0; i < renderer->width * renderer->height; i += 4) {
        _mm_store_ps(&renderer->depthbuffer[i], inf);
    }

    int visible_splats = 0, splats_behind_camera = 0, splats_outside_screen = 0;
    bool debug_enabled = (debug_mode != DEBUG_NONE);
    int debug_count = 0;

    // Precompute useful values
    float half_width = renderer->width * 0.5f;
    float half_height = renderer->height * 0.5f;
    float fov_tan = tanf(45.0f * M_PI / 180.0f);  // Precompute tan(fov/2)
    float aspect_ratio = (float)renderer->width / (float)renderer->height;

    // Precompute camera vectors
    vec3 cam_right = camera->right;
    vec3 cam_up = camera->up;
    vec3 cam_front = camera->front;
    vec3 cam_pos = camera->position;

    #pragma omp parallel for reduction(+:visible_splats,splats_behind_camera,splats_outside_screen) schedule(dynamic, 64)
    for (int i = 0; i < splat_count; i++) {
        // Transform splat position to camera space
        vec3 pos_cam = {
            splats[i].x - cam_pos.x,
            splats[i].y - cam_pos.y,
            splats[i].z - cam_pos.z
        };

        vec3 pos_cam_transformed = {
            pos_cam.x * cam_right.x + pos_cam.y * cam_right.y + pos_cam.z * cam_right.z,
            pos_cam.x * cam_up.x + pos_cam.y * cam_up.y + pos_cam.z * cam_up.z,
            pos_cam.x * cam_front.x + pos_cam.y * cam_front.y + pos_cam.z * cam_front.z
        };

        // Check if the splat is behind the camera
        if (pos_cam_transformed.z <= 0) {
            splats_behind_camera++;
            continue;
        }

        // Perspective Projection Calculation
        float inv_z = 1.0f / pos_cam_transformed.z;
        float scale = fov_tan * pos_cam_transformed.z;
        float proj_x = (pos_cam_transformed.x / (aspect_ratio * scale)) * half_width + half_width;
        float proj_y = -(pos_cam_transformed.y / scale) * half_height + half_height;

        // Calculate splat radius in screen space
        float radius = splats[i].scale * inv_z * renderer->width;

        // Check if the splat is outside the screen bounds
        if (proj_x + radius < 0 || proj_x - radius >= renderer->width || 
            proj_y + radius < 0 || proj_y - radius >= renderer->height) {
            splats_outside_screen++;
            continue;
        }

        visible_splats++;

        // Rasterize the splat within its circular bounds
        int min_x = (int)fmaxf(0, proj_x - radius);
        int max_x = (int)fminf(renderer->width - 1, proj_x + radius);
        int min_y = (int)fmaxf(0, proj_y - radius);
        int max_y = (int)fminf(renderer->height - 1, proj_y + radius);

        float inv_radius = 1.0f / radius;
        __m128 splat_color = _mm_set_ps(splats[i].a, splats[i].b * 255.0f, splats[i].g * 255.0f, splats[i].r * 255.0f);
        __m128 pos_z = _mm_set1_ps(pos_cam_transformed.z);

        for (int y = min_y; y <= max_y; y++) {
            float dy = (y - proj_y) * inv_radius;
            float dy_sq = dy * dy;

            for (int x = min_x; x <= max_x; x++) {
                float dx = (x - proj_x) * inv_radius;
                float dist_sq = dx * dx + dy_sq;

                if (dist_sq <= 1.0f) {
                    int buffer_index = (y * renderer->width + x);
                    float* depth = &renderer->depthbuffer[buffer_index];

                    if (pos_cam_transformed.z < *depth) {
                        float alpha = splats[i].a * expf(-dist_sq);
                        unsigned char* pixel = &renderer->framebuffer[buffer_index * 3];

                        __m128 curr_color = _mm_set_ps(1.0f, pixel[2], pixel[1], pixel[0]);
                        __m128 alpha_vec = _mm_set1_ps(alpha);
                        __m128 result = _mm_add_ps(
                            _mm_mul_ps(_mm_sub_ps(_mm_set1_ps(1.0f), alpha_vec), curr_color),
                            _mm_mul_ps(alpha_vec, splat_color)
                        );

                        _mm_store_ss(depth, pos_z);
                        __m128i result_int = _mm_cvtps_epi32(result);
                        pixel[0] = _mm_extract_epi8(result_int, 0);
                        pixel[1] = _mm_extract_epi8(result_int, 4);
                        pixel[2] = _mm_extract_epi8(result_int, 8);
                    }
                }
            }
        }
    }

    // Summary Logging
    printf("Total splats behind camera: %d\n", splats_behind_camera);
    printf("Total splats outside screen bounds: %d\n", splats_outside_screen);
    printf("Visible splats: %d\n", visible_splats);

    // Update the OpenGL texture with the rendered framebuffer
    glBindTexture(GL_TEXTURE_2D, renderer->texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, renderer->width, renderer->height, GL_RGB, GL_UNSIGNED_BYTE, renderer->framebuffer);
}


void draw_fullscreen_quad(Renderer* renderer) {
    glUseProgram(renderer->shaderProgram);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("OpenGL error after glUseProgram: 0x%x\n", error);
    }

    glBindTexture(GL_TEXTURE_2D, renderer->texture);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("OpenGL error after glBindTexture: 0x%x\n", error);
    }

    glBindVertexArray(renderer->VAO);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("OpenGL error after glBindVertexArray: 0x%x\n", error);
    }

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("OpenGL error after glDrawElements: 0x%x\n", error);
    }

    glBindVertexArray(0);
}

void free_renderer(Renderer* renderer) {
    free(renderer->framebuffer);
    free(renderer->depthbuffer);
    glDeleteVertexArrays(1, &renderer->VAO);
    glDeleteBuffers(1, &renderer->VBO);
    glDeleteBuffers(1, &renderer->EBO);
    glDeleteProgram(renderer->shaderProgram);
    glDeleteTextures(1, &renderer->texture);
}