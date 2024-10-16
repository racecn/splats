#include "cnpy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <zip.h>  // Use libzip for handling ZIP archives

cnpy_array cnpy_load_npz(const char* fname, const char* varname) {
    cnpy_array result = {NULL, NULL, 0, '\0'};
    
    printf("Attempting to open NPZ file: %s\n", fname);

    // Open the NPZ archive using libzip
    int err = 0;
    zip_t* zip_archive = zip_open(fname, ZIP_RDONLY, &err);
    if (zip_archive == NULL) {
        printf("Error: Unable to open NPZ file %s\n", fname);
        return result;
    }

    printf("Successfully opened NPZ file: %s\n", fname);

    // Print all file names in the archive
    zip_int64_t num_files = zip_get_num_entries(zip_archive, 0);
    printf("Files in the NPZ archive:\n");
    for (zip_int64_t i = 0; i < num_files; i++) {
        const char* file_name = zip_get_name(zip_archive, i, 0);
        printf(" - %s\n", file_name);
    }

    // Look for the first array, `arr_0.npy`
    for (zip_int64_t i = 0; i < num_files; i++) {
        const char* file_name = zip_get_name(zip_archive, i, 0);
        if (strstr(file_name, "arr_0.npy")) {
            printf("Found matching variable: %s\n", file_name);

            // Extract the NPY file from the archive
            zip_file_t* npy_file = zip_fopen_index(zip_archive, i, 0);
            if (!npy_file) {
                printf("Error: Unable to extract NPY file %s\n", file_name);
                zip_close(zip_archive);
                return result;
            }

            // Read the contents of the NPY file
            struct zip_stat st;
            zip_stat_index(zip_archive, i, 0, &st);
            size_t npy_size = st.size;
            void* npy_data = malloc(npy_size);
            if (!npy_data) {
                printf("Error: Memory allocation failed for NPY data.\n");
                zip_fclose(npy_file);
                zip_close(zip_archive);
                return result;
            }

            zip_fread(npy_file, npy_data, npy_size);
            zip_fclose(npy_file);

            // Now load the NPY data from memory
            result = cnpy_load_npy_from_memory(npy_data, npy_size);

            // Free the extracted NPY data
            free(npy_data);

            break;
        }
    }

    zip_close(zip_archive);

    if (!result.data) {
        printf("Error: 'arr_0' not found in NPZ file\n");
    } else {
        // Inspect the loaded array
        printf("Loaded array with %lu elements, ndim: %lu\n", result.shape ? result.shape[0] : 0, result.ndim);
    }

    // Debug: Print out the ndim and shape from the file header
    printf("Array dimensions (ndim): %zu\n", result.ndim);
    for (size_t i = 0; i < result.ndim; ++i) {
        printf("Shape[%zu]: %zu\n", i, result.shape[i]);
    }

    return result;
}

cnpy_array cnpy_load_npy_from_memory(const void* npy_data, size_t npy_size) {
    cnpy_array result = {NULL, NULL, 0, '\0'};
    const char* data_ptr = (const char*)npy_data;

    // Check the NPY magic string
    if (strncmp(data_ptr, "\x93NUMPY", 6) != 0) {
        printf("Error: Invalid NPY file format\n");
        return result;
    }

    // Read the version
    unsigned char major_version = data_ptr[6];
    unsigned char minor_version = data_ptr[7];
    printf("NPY Version: %d.%d\n", major_version, minor_version);

    // Read the header length (different depending on the version)
    size_t header_len;
    if (major_version == 1 && minor_version == 0) {
        header_len = *(unsigned short*)(data_ptr + 8);
    } else {
        header_len = *(unsigned int*)(data_ptr + 8);
    }

    // Read the header string
    char* header_str = malloc(header_len + 1);
    if (!header_str) {
        printf("Error: Memory allocation failed for NPY header.\n");
        return result;
    }

    memcpy(header_str, data_ptr + 10, header_len);
    header_str[header_len] = '\0';

    printf("NPY Header: %s\n", header_str);

    // Extract array shape from the header string
    // Assuming header contains something like 'shape': (480, 640, 1)
    char* shape_str = strstr(header_str, "'shape':");
    if (shape_str) {
        // Count how many dimensions are present
        size_t ndim = 0;
        const char* dim_str = strchr(shape_str, '(');
        if (dim_str) {
            ndim = 1;  // Start with one dimension
            for (const char* p = dim_str + 1; *p && *p != ')'; p++) {
                if (*p == ',') ndim++;
            }
        }
        result.ndim = ndim;

        // Allocate memory for the shape array based on the number of dimensions
        result.shape = (size_t*)malloc(result.ndim * sizeof(size_t));
        if (!result.shape) {
            printf("Error: Memory allocation failed for shape.\n");
            free(header_str);
            return result;
        }

        // Extract shape values based on the number of dimensions
        if (ndim == 1) {
            sscanf(shape_str, "'shape': (%zu)", &result.shape[0]);
        } else if (ndim == 2) {
            sscanf(shape_str, "'shape': (%zu, %zu)", &result.shape[0], &result.shape[1]);
        } else if (ndim == 3) {
            sscanf(shape_str, "'shape': (%zu, %zu, %zu)", &result.shape[0], &result.shape[1], &result.shape[2]);
        } else {
            printf("Error: Unsupported number of dimensions (%zu)\n", ndim);
            free(header_str);
            return result;
        }

        printf("Parsed shape: ");
        for (size_t i = 0; i < result.ndim; ++i) {
            printf("%zu ", result.shape[i]);
        }
        printf("\n");

    } else {
        printf("Error: Could not find shape in the header\n");
        free(header_str);
        return result;
    }

    // Now read the data based on the shape and dtype
    size_t data_offset = 10 + header_len;
    size_t data_size = npy_size - data_offset;

    result.data = malloc(data_size);
    if (!result.data) {
        printf("Error: Memory allocation failed for NPY data.\n");
        free(header_str);
        return result;
    }

    memcpy(result.data, data_ptr + data_offset, data_size);

    free(header_str);

    return result;
}

void cnpy_free(cnpy_array* arr) {
    if (arr->data) free(arr->data);
    if (arr->shape) free(arr->shape);
    arr->data = NULL;
    arr->shape = NULL;
    arr->ndim = 0;
    arr->datatype = '\0';
}
