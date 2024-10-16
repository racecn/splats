#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "splat.h"
#include "renderer.h"
#include "data_loader.h"
#include "camera.h"
#include "image_loader.h"  // Include image loading utility
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#define WIDTH 800
#define HEIGHT 600

Camera camera;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;

void processInput(GLFWwindow *window, Camera *camera);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

void processInput(GLFWwindow *window, Camera *camera) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 0.05f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera_move(camera, CAMERA_FORWARD, cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera_move(camera, CAMERA_BACKWARD, cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera_move(camera, CAMERA_LEFT, cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera_move(camera, CAMERA_RIGHT, cameraSpeed);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 

    lastX = xpos;
    lastY = ypos;

    camera_process_mouse_movement(&camera, xoffset, yoffset, true);
}

int main() {
    printf("Gaussian Splats Renderer\n");

    if (!glfwInit()) {
        printf("Failed to initialize GLFW\n");
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Gaussian Splats Renderer", NULL, NULL);
    if (!window) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    printf("OpenGL version: %s\n", glGetString(GL_VERSION));
    printf("GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    camera_init(&camera);
    camera.position = (vec3){0.0f, 0.0f, 3.0f};
    camera_update_vectors(&camera);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Renderer renderer;
    init_renderer(&renderer, WIDTH, HEIGHT);

    const char* npz_file_path = "B:\\splats\\data\\SF_6thAndMission_medium0\\train\\depth\\midsize_muscle_02-000.npz";

    Splat* splats;
    int splat_count = load_splats_from_npz(npz_file_path, &splats);

    if (splat_count == 0) {
        printf("Failed to load splats from %s. Exiting.\n", npz_file_path);
        glfwTerminate();
        return 1;
    }

    printf("Loaded %d splats successfully from %s.\n", splat_count, npz_file_path);

    // Load the corresponding RGB image for the first frame
    int image_width, image_height, image_channels;

    // Extract the base name from the .npz file path to create the RGB image path
    char rgb_image_path[512];
    snprintf(rgb_image_path, sizeof(rgb_image_path), 
            "B:\\splats\\data\\SF_6thAndMission_medium0\\train\\rgb\\%s.png", 
            "midsize_muscle_02-000");  // Use the correct naming format

    unsigned char* rgb_image = load_png_image(rgb_image_path, &image_width, &image_height, &image_channels);

    if (!rgb_image) {
        printf("Failed to load corresponding RGB image: %s. Exiting.\n", rgb_image_path);
        glfwTerminate();
        return 1;
    }

    printf("Loaded RGB image successfully: %s (Width: %d, Height: %d, Channels: %d)\n",
        rgb_image_path, image_width, image_height, image_channels);

    while (!glfwWindowShouldClose(window)) {
        processInput(window, &camera);

        glClear(GL_COLOR_BUFFER_BIT);

        // Render splats and apply the RGB texture as needed
        render_scene(&renderer, splats, splat_count, &camera, DEBUG_NONE, 10);

        draw_fullscreen_quad(&renderer);

        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            printf("OpenGL error: 0x%x\n", error);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    stbi_image_free(rgb_image);  // Free the loaded image memory
    free_renderer(&renderer);
    free(splats);
    glfwTerminate();

    return 0;
}
