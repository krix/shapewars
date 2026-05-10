#ifndef EMBEDDED_ASSETS_H
#define EMBEDDED_ASSETS_H

#include <stdint.h>

typedef struct {
    const char *name;
    const unsigned char *data;
    int size;
} embedded_asset_t;

const unsigned char *embedded_asset_load(const char *name, int *size);

#endif
