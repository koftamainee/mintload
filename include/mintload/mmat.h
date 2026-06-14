#ifndef MINTLOAD_MMAT_H
#define MINTLOAD_MMAT_H

#include "base.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MINTLOAD_ALPHA_OPAQUE 0
#define MINTLOAD_ALPHA_MASK   1
#define MINTLOAD_ALPHA_BLEND  2

#define MINTLOAD_FLAG_DOUBLE_SIDED    (1u << 0)
#define MINTLOAD_FLAG_ALPHA_MODE_MASK (3u << 1)
#define MINTLOAD_FLAG_UNLIT           (1u << 3)

#define MINTLOAD_TEX_BASECOLOR           0
#define MINTLOAD_TEX_NORMAL              1
#define MINTLOAD_TEX_METALLIC_ROUGHNESS  2
#define MINTLOAD_TEX_EMISSIVE            3
#define MINTLOAD_TEX_OCCLUSION           4

typedef struct {
    uint32_t type;
    int32_t  texture_index;
} MintTextureSlot;

typedef struct {
    float                 base_color[4];
    float                 metallic;
    float                 roughness;
    float                 emissive[3];
    float                 alpha_cutoff;
    uint32_t              flags;
    uint32_t              texture_count;
    float                 normal_scale;
    float                 emissive_strength;
    const MintTextureSlot* textures;
    MintloadMapping       _m;
} MintMaterial;

MintloadResult mintload_MmatLoad(const char* path, MintMaterial* out);
void           mintload_MmatUnload(MintMaterial* mat);

#ifdef __cplusplus
}
#endif

#endif
