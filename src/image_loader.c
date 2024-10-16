#include "image_loader.h"
#include <stb_image.h>  // stb_image.h needs to be added to your project

unsigned char* load_png_image(const char* file_path, int* width, int* height, int* channels) {
    unsigned char* image_data = stbi_load(file_path, width, height, channels, 0);
    if (!image_data) {
        printf("Failed to load PNG image: %s\n", file_path);
    }
    return image_data;
}
