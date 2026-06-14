#include "mintload/mmat.h"
#include "utils.h"
#include <string.h>

#define MMAT_HDR 64

static uint32_t smagic(void) {
    const uint8_t m[4] = {'M','M','A','T'};
    return read_u32_at(m);
}

MintloadResult mintload_MmatLoad(const char* path, MintMaterial* out) {
    if (!path || !out) return MINTLOAD_ERR_INVALID_DATA;

    MintloadResult r = mintload_map_file(path, &out->_m);
    if (r != MINTLOAD_SUCCESS) return r;

    const uint8_t* base = (const uint8_t*)out->_m.base;
    size_t size = out->_m.size;

    if (size < MMAT_HDR) { mintload_unmap_file(&out->_m); return MINTLOAD_ERR_INVALID_DATA; }

    uint32_t magic = read_u32_at(base);
    uint32_t ver   = read_u32_at(base + 4);
    if (magic != smagic() || ver != 1) {
        mintload_unmap_file(&out->_m);
        return MINTLOAD_ERR_INVALID_DATA;
    }

    for (int i = 0; i < 4; i++)
        out->base_color[i] = read_f32_at(base + 8 + i * 4);
    out->metallic   = read_f32_at(base + 24);
    out->roughness  = read_f32_at(base + 28);
    for (int i = 0; i < 3; i++)
        out->emissive[i] = read_f32_at(base + 32 + i * 4);
    out->alpha_cutoff      = read_f32_at(base + 44);
    out->flags             = read_u32_at(base + 48);
    out->texture_count     = read_u32_at(base + 52);
    out->normal_scale      = read_f32_at(base + 56);
    out->emissive_strength = read_f32_at(base + 60);

    if (out->texture_count > (size - MMAT_HDR) / 8) {
        mintload_unmap_file(&out->_m); return MINTLOAD_ERR_INVALID_DATA;
    }

    out->textures = (const MintTextureSlot*)(base + MMAT_HDR);

    return MINTLOAD_SUCCESS;
}

void mintload_MmatUnload(MintMaterial* mat) {
    if (mat) mintload_unmap_file(&mat->_m);
}
