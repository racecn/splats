#ifndef SPLAT_H
#define SPLAT_H

// Splat struct: represents a point in 3D space with a position, direction, color, opacity, and scale
typedef struct {
    float x, y, z;     // Position in 3D space
    float dx, dy, dz;  // Direction (normalized)
    float r, g, b, a;  // Color (RGB) and opacity (alpha)
    float scale;       // Scale factor for the size of the splat
} Splat;

/**
 * @brief Initializes a Splat with position, direction, color, opacity, and scale.
 *
 * @param splat Pointer to the Splat structure to initialize.
 * @param x X-coordinate of the splat's position.
 * @param y Y-coordinate of the splat's position.
 * @param z Z-coordinate of the splat's position.
 * @param dx X-component of the splat's direction (will be normalized).
 * @param dy Y-component of the splat's direction (will be normalized).
 * @param dz Z-component of the splat's direction (will be normalized).
 * @param r Red color value (0.0 - 1.0).
 * @param g Green color value (0.0 - 1.0).
 * @param b Blue color value (0.0 - 1.0).
 * @param a Alpha (opacity) value (0.0 - 1.0).
 * @param scale Size factor for the splat.
 */
void init_splat(Splat* splat, float x, float y, float z, float dx, float dy, float dz,
                float r, float g, float b, float a, float scale);

#endif // SPLAT_H
