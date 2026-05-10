#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_LINEAR
#include "high_impact/libs/stb_image.h"
#define QOI_IMPLEMENTATION
#include "high_impact/libs/qoi.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: png2qoi <input.png> <output.qoi>\n");
        return 1;
    }
    int w, h, channels;
    if (!stbi_info(argv[1], &w, &h, &channels)) {
        printf("Couldn't read header %s\n", argv[1]);
        return 1;
    }
    channels = 4;
    void *pixels = stbi_load(argv[1], &w, &h, NULL, channels);
    if (!pixels) {
        printf("Couldn't load %s\n", argv[1]);
        return 1;
    }
    qoi_desc desc = { w, h, channels, QOI_SRGB };
    int encoded = qoi_write(argv[2], pixels, &desc);
    free(pixels);
    if (!encoded) {
        printf("Couldn't write %s\n", argv[2]);
        return 1;
    }
    printf("Converted %s -> %s (%dx%d)\n", argv[1], argv[2], w, h);
    return 0;
}
