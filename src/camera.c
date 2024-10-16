// File: src/camera.c
#include "camera.h"
#include <math.h>

// Helper function to normalize vectors
vec3 normalize(vec3 v) {
    float length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return (vec3){v.x / length, v.y / length, v.z / length};
}

void camera_init(Camera* camera) {
    camera->position = (vec3){0.0f, 0.0f, 5.0f};
    camera->front = (vec3){0.0f, 0.0f, -1.0f};
    camera->up = (vec3){0.0f, 1.0f, 0.0f};
    camera->yaw = -90.0f;  // Default yaw facing towards negative Z axis
    camera->pitch = 0.0f;
    camera_update_vectors(camera);
}

void camera_update_vectors(Camera* camera) {
    // Calculate the new front vector
    vec3 front;
    front.x = cos(camera->yaw * M_PI / 180.0f) * cos(camera->pitch * M_PI / 180.0f);
    front.y = sin(camera->pitch * M_PI / 180.0f);
    front.z = sin(camera->yaw * M_PI / 180.0f) * cos(camera->pitch * M_PI / 180.0f);
    camera->front = normalize(front);  // Normalize the front vector

    // Recalculate right and up vectors
    vec3 worldUp = {0.0f, 1.0f, 0.0f};
    camera->right = normalize((vec3){
        camera->front.y * worldUp.z - camera->front.z * worldUp.y,
        camera->front.z * worldUp.x - camera->front.x * worldUp.z,
        camera->front.x * worldUp.y - camera->front.y * worldUp.x
    });

    // Recalculate the up vector
    camera->up = normalize((vec3){
        camera->right.y * camera->front.z - camera->right.z * camera->front.y,
        camera->right.z * camera->front.x - camera->right.x * camera->front.z,
        camera->right.x * camera->front.y - camera->right.y * camera->front.x
    });
}

void camera_move(Camera* camera, Camera_Movement direction, float speed) {
    // Define a speed multiplier to make the camera move faster
    float speed_multiplier = 2.0f; // Increase this value to move faster

    // Apply the multiplier to the speed
    float adjusted_speed = speed * speed_multiplier;

    // Move the camera in the direction based on the camera's front, right, or backward vectors
    if (direction == CAMERA_FORWARD)
        camera->position = (vec3){
            camera->position.x + camera->front.x * adjusted_speed,
            camera->position.y + camera->front.y * adjusted_speed,
            camera->position.z + camera->front.z * adjusted_speed
        };
    if (direction == CAMERA_BACKWARD)
        camera->position = (vec3){
            camera->position.x - camera->front.x * adjusted_speed,
            camera->position.y - camera->front.y * adjusted_speed,
            camera->position.z - camera->front.z * adjusted_speed
        };
    if (direction == CAMERA_LEFT)
        camera->position = (vec3){
            camera->position.x - camera->right.x * adjusted_speed,
            camera->position.y - camera->right.y * adjusted_speed,
            camera->position.z - camera->right.z * adjusted_speed
        };
    if (direction == CAMERA_RIGHT)
        camera->position = (vec3){
            camera->position.x + camera->right.x * adjusted_speed,
            camera->position.y + camera->right.y * adjusted_speed,
            camera->position.z + camera->right.z * adjusted_speed
        };
}


void camera_process_mouse_movement(Camera* camera, float xoffset, float yoffset, bool constrainPitch) {
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    camera->yaw += xoffset;
    camera->pitch += yoffset;

    // Constrain the pitch to avoid flipping over
    if (constrainPitch) {
        if (camera->pitch > 89.0f)
            camera->pitch = 89.0f;
        if (camera->pitch < -89.0f)
            camera->pitch = -89.0f;
    }

    camera_update_vectors(camera);
}
