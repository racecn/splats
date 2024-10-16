#include "splat.h"
#include <math.h>
#include <stdlib.h>

// Helper function to clamp a value within a specific range
static float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// Helper function to normalize the direction vector
static void normalize_direction(float* dx, float* dy, float* dz) {
    float length = sqrtf((*dx) * (*dx) + (*dy) * (*dy) + (*dz) * (*dz));
    if (length > 0.0f) {  // Avoid division by zero
        *dx /= length;
        *dy /= length;
        *dz /= length;
    } else {
        *dx = *dy = *dz = 0.0f;  // In case length is zero, set direction to zero vector
    }
}

// Initialize a splat with given position, direction, color, alpha, and scale
void init_splat(Splat* splat, float x, float y, float z, float dx, float dy, float dz,
                float r, float g, float b, float a, float scale) {
    if (!splat) return;  // Safety check: Ensure splat is not NULL

    // Set position
    splat->x = x;
    splat->y = y;
    splat->z = z;

    // Normalize and set direction
    normalize_direction(&dx, &dy, &dz);
    splat->dx = dx;
    splat->dy = dy;
    splat->dz = dz;

    // Clamp and set color (RGB) and alpha values
    splat->r = clamp(r, 0.0f, 1.0f);  // Ensure color components are between 0 and 1
    splat->g = clamp(g, 0.0f, 1.0f);
    splat->b = clamp(b, 0.0f, 1.0f);
    splat->a = clamp(a, 0.0f, 1.0f);  // Ensure alpha is between 0 and 1

    // Set scale (no need to clamp unless there are specific restrictions on size)
    splat->scale = scale > 0.0f ? scale : 1.0f;  // Ensure scale is positive; default to 1.0 if invalid
}
