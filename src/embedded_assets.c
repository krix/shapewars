#include "embedded_assets.h"
#include "embedded_font_qoi.h"
#include "embedded_font_json.h"
#include <string.h>

static const embedded_asset_t assets[] = {
    {"font_sans-serif.qoi", font_sans_serif_qoi_data, font_sans_serif_qoi_size},
    {"font_sans-serif.json", font_sans_serif_json_data, font_sans_serif_json_size},
};

static const int asset_count = sizeof(assets) / sizeof(assets[0]);

const unsigned char *embedded_asset_load(const char *name, int *size) {
    for (int i = 0; i < asset_count; i++) {
        if (strcmp(name, assets[i].name) == 0) {
            *size = assets[i].size;
            return assets[i].data;
        }
    }
    *size = 0;
    return NULL;
}
