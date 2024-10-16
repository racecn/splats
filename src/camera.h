#ifndef CAMERA_H
#define CAMERA_H

#include <stdbool.h>

typedef struct {
    float x, y, z;
} vec3;

typedef enum {
    CAMERA_FORWARD,
    CAMERA_BACKWARD,
    CAMERA_LEFT,
    CAMERA_RIGHT
} Camera_Movement;

typedef struct {
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;
    float yaw;
    float pitch;
} Camera;

void camera_init(Camera* camera);
void camera_update_vectors(Camera* camera);
void camera_move(Camera* camera, Camera_Movement direction, float deltaTime);
void camera_process_mouse_movement(Camera* camera, float xoffset, float yoffset, bool constrainPitch);

#endif // CAMERA_H