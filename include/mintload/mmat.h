#ifndef MINTLOAD_MMAT_H
#define MINTLOAD_MMAT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MINT_MMAT_MAGIC 0x4D4D4154u
#define MINT_MMAT_VERSION 1u

enum MintAlphaMode {
    MINT_ALPHA_OPAQUE = 0,
    MINT_ALPHA_MASK   = 1,
    MINT_ALPHA_BLEND  = 2
};

enum MintTextureType {
    MINT_TEX_BASECOLOR = 0,
    MINT_TEX_NORMAL    = 1,
    MINT_TEX_METALLIC_ROUGHNESS = 2,
    MINT_TEX_EMISSIVE  = 3,
    MINT_TEX_OCCLUSION = 4
};

typedef struct MintTextureSlot {
    uint32_t type;
    char path[124];
} MintTextureSlot;

typedef struct MintMaterial {
    uint32_t magic;
    uint32_t version;
    float baseColorFactor[4];
    float metallicFactor;
    float roughnessFactor;
    float emissiveFactor[3];
    float alphaCutoff;
    uint32_t flags;
    uint32_t textureCount;
} MintMaterial;

typedef struct MintMaterialMemoryRequirements {
    size_t slotsBufferSize;
    size_t slotsAlignment;
} MintMaterialMemoryRequirements;

enum MintResult mintload_MmatGetMemoryRequirements(const char* path, MintMaterialMemoryRequirements* req);
enum MintResult mintload_MmatLoad(
    const char* path,
    MintMaterial* material,
    void* slotsBuffer, size_t slotsBufferSize
);

#ifdef __cplusplus
}
#endif

#endif
