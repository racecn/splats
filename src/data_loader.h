#ifndef DATA_LOADER_H
#define DATA_LOADER_H

#include <stddef.h>  // for size_t
#include "splat.h"   // Assuming you define Splat here

// Function to load a specific .npy file from a .npz (ZIP) archive
void* load_npy_from_zip(const char* zip_filename, const char* target_filename, size_t* file_size);

// Function to read the .npy array from the loaded file
void* load_npy_array(void* npy_data, size_t npy_size, size_t* array_size);

// Function to load splats from an .npz file
int load_splats_from_npz(const char* filename, Splat** splats);

#endif // DATA_LOADER_H
