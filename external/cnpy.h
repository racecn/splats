#ifndef CNPY_H
#define CNPY_H

#include <stddef.h>  // For size_t

/**
 * Structure to store a loaded NumPy array.
 * - data: Pointer to the array data.
 * - shape: Pointer to the array's dimensions (size of each axis).
 * - ndim: Number of dimensions in the array.
 * - datatype: Character representing the data type ('f' for float, 'i' for int, etc.).
 */
typedef struct {
    void* data;        // Pointer to the raw array data
    size_t* shape;     // Array shape (e.g., dimensions like [100, 3])
    size_t ndim;       // Number of dimensions
    char datatype;     // Data type of the array ('f' for float, 'i' for int, etc.)
} cnpy_array;

/**
 * Load a specific variable from a .npz file (NumPy ZIP archive).
 * - fname: The filename of the .npz file.
 * - varname: The name of the variable to extract from the .npz archive.
 * Returns a cnpy_array struct containing the array data, shape, and metadata.
 */
cnpy_array cnpy_load_npz(const char* fname, const char* varname);
// Add this declaration at the top of cnpy.c or in cnpy.h
cnpy_array cnpy_load_npy_from_memory(const void* npy_data, size_t npy_size);

/**
 * Free the memory associated with a cnpy_array.
 * - arr: Pointer to the cnpy_array to free.
 * This function frees the data and shape pointers in the structure.
 */
void cnpy_free(cnpy_array* arr);

#endif // CNPY_H
