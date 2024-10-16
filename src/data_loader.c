#include <stdio.h>  // For printf
#include <stdlib.h> // For malloc, free
#include "data_loader.h"
#include "cnpy.h"   // Include cnpy.h for cnpy_array, cnpy_load_npz, cnpy_free

int load_splats_from_npz(const char* filename, Splat** splats) {
    // Load the npz file using the cnpy library
    cnpy_array result = cnpy_load_npz(filename, "arr_0");

    if (result.data == NULL) {
        printf("Failed to load 'arr_0' data from %s\n", filename);
        return 0;
    }

    // Debug: Print the number of dimensions and the shape
    printf("Number of dimensions: %zu\n", result.ndim);
    printf("Array shape: ");
    for (size_t i = 0; i < result.ndim; ++i) {
        printf("%zu ", result.shape[i]);
    }
    printf("\n");

    // Calculate the number of splats by multiplying all dimensions
    size_t num_splats = 1;
    for (size_t i = 0; i < result.ndim; ++i) {
        num_splats *= result.shape[i];
    }
    printf("Calculated number of splats: %zu\n", num_splats);

    // Ensure that the number of elements is greater than zero
    if (num_splats == 0) {
        printf("Error: No splats found. Exiting.\n");
        cnpy_free(&result);
        return 0;
    }

    // Allocate memory for the splats
    *splats = (Splat*)malloc(num_splats * sizeof(Splat));
    if (*splats == NULL) {
        printf("Error: Failed to allocate memory for splats.\n");
        cnpy_free(&result);
        return 0;
    }

    // Populate the splats with position and color data
    for (size_t i = 0; i < num_splats; i++) {
        size_t row = i / result.shape[1];
        size_t col = i % result.shape[1];

        (*splats)[i].x = (float)col;              // X = column index
        (*splats)[i].y = (float)row;              // Y = row index
        (*splats)[i].z = ((float*)result.data)[i]; // Z = depth value (from arr_0.npy)
        (*splats)[i].r = 1.0f;                    // Default color: white
        (*splats)[i].g = 1.0f;
        (*splats)[i].b = 1.0f;
        (*splats)[i].scale = 1.0f;                // Default scale
        (*splats)[i].a = 1.0f;                    // Default opacity
    }

    printf("Loaded %zu splats successfully from %s.\n", num_splats, filename);

    // Free the array data
    cnpy_free(&result);

    return num_splats;
}
