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

void render_scene(Renderer* renderer, Splat* splats, int splat_count, Camera* camera, DebugMode debug_mode, int debug_limit) {
    // Clear the framebuffer and depthbuffer efficiently
    memset(renderer->framebuffer, 0, renderer->width * renderer->height * 3 * sizeof(unsigned char));

    // Initialize the depth buffer with INFINITY
    for (int i = 0; i < renderer->width * renderer->height; i++) {
        renderer->depthbuffer[i] = INFINITY;
    }

    int visible_splats = 0;
    int splats_behind_camera = 0;
    int splats_outside_screen = 0;
    bool debug_enabled = (debug_mode != DEBUG_NONE);
    int debug_count = 0;

    // Precompute useful values
    float half_width = renderer->width / 2.0f;
    float half_height = renderer->height / 2.0f;

    float fov = 90.0f;  // Field of view in degrees
    float aspect_ratio = (float)renderer->width / (float)renderer->height;

    for (int i = 0; i < splat_count; i++) {
        if (debug_count >= debug_limit) {
            debug_enabled = false;
        }

        // Transform splat position to camera space
        vec3 pos_cam = {
            splats[i].x - camera->position.x,
            splats[i].y - camera->position.y,
            splats[i].z - camera->position.z
        };

        vec3 pos_cam_transformed = {
            pos_cam.x * camera->right.x + pos_cam.y * camera->right.y + pos_cam.z * camera->right.z,
            pos_cam.x * camera->up.x + pos_cam.y * camera->up.y + pos_cam.z * camera->up.z,
            pos_cam.x * camera->front.x + pos_cam.y * camera->front.y + pos_cam.z * camera->front.z
        };

        // Check if the splat is behind the camera
        if (pos_cam_transformed.z <= 0) {
            splats_behind_camera++;
            if (debug_enabled && (debug_mode & DEBUG_TRANSFORM) && debug_count < debug_limit) {
                printf("Splat %d is behind the camera: z = %f\n", i, pos_cam_transformed.z);
            }
            continue;
        }

        // Adjusted Perspective Projection Calculation
        float scale = tan(fov * 0.5f * M_PI / 180.0f) * pos_cam_transformed.z;
        float proj_x = (pos_cam_transformed.x / (aspect_ratio * scale)) * half_width + half_width;
        float proj_y = -(pos_cam_transformed.y / scale) * half_height + half_height;

        // Calculate splat radius in screen space
        float radius = splats[i].scale / pos_cam_transformed.z * renderer->width;

        // Check if the splat is outside the screen bounds
        if (proj_x + radius < 0 || proj_x - radius >= renderer->width || proj_y + radius < 0 || proj_y - radius >= renderer->height) {
            splats_outside_screen++;
            if (debug_enabled && (debug_mode & DEBUG_PROJECTION) && debug_count < debug_limit) {
                printf("Splat %d is outside screen bounds: proj_x = %f, proj_y = %f, radius = %f\n", i, proj_x, proj_y, radius);
            }
            continue;
        }

        visible_splats++;

        // Rasterize the splat within its circular bounds
        int min_x = (int)fmaxf(0, proj_x - radius);
        int max_x = (int)fminf(renderer->width - 1, proj_x + radius);
        int min_y = (int)fmaxf(0, proj_y - radius);
        int max_y = (int)fminf(renderer->height - 1, proj_y + radius);

        for (int y = min_y; y <= max_y; y++) {
            for (int x = min_x; x <= max_x; x++) {
                float dx = (x - proj_x) / radius;
                float dy = (y - proj_y) / radius;
                float dist_sq = dx * dx + dy * dy;

                if (dist_sq <= 1.0f) {
                    int buffer_index = (y * renderer->width + x) * 3;

                    if (pos_cam_transformed.z < renderer->depthbuffer[y * renderer->width + x]) {
                        float alpha = splats[i].a * expf(-dist_sq);
                        unsigned char* pixel = &renderer->framebuffer[buffer_index];

                        pixel[0] = (unsigned char)((1.0f - alpha) * pixel[0] + alpha * (splats[i].r * 255.0f));
                        pixel[1] = (unsigned char)((1.0f - alpha) * pixel[1] + alpha * (splats[i].g * 255.0f));
                        pixel[2] = (unsigned char)((1.0f - alpha) * pixel[2] + alpha * (splats[i].b * 255.0f));

                        renderer->depthbuffer[y * renderer->width + x] = pos_cam_transformed.z;
                    }
                }
            }
        }

        if (debug_enabled && (debug_mode & DEBUG_RENDERING) && debug_count < debug_limit) {
            printf("Rendered splat %d with radius %f\n", i, radius);
        }

        if (debug_enabled) {
            debug_count++;
        }
    }

    // Summary Logging: Print the total number of splats behind the camera and outside the screen
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