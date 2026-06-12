#include "mintload/mintload.h"
#include "utils.h"
#include <stdio.h>

enum MintResult mintload_MmatGetMemoryRequirements(const char* path, MintMaterialMemoryRequirements* req) {
    if (!path || !req) return MINT_ERR_INVALID_DATA;

    FILE* f = fopen(path, "rb");
    if (!f) return MINT_ERR_IO;

    uint8_t header[56];
    size_t n = fread(header, 1, 56, f);
    fclose(f);

    if (n < 56) return MINT_ERR_INVALID_DATA;

    uint32_t magic = read_u32_le(header);
    if (magic != MINT_MMAT_MAGIC) return MINT_ERR_INVALID_DATA;

    uint32_t version = read_u32_le(header + 4);
    if (version != MINT_MMAT_VERSION) return MINT_ERR_UNSUPPORTED_VERSION;

    uint32_t textureCount = read_u32_le(header + 52);
    req->slotsBufferSize = (size_t)textureCount * sizeof(MintTextureSlot);
    req->slotsAlignment = 4;
    return MINT_SUCCESS;
}

enum MintResult mintload_MmatLoad(
    const char* path,
    MintMaterial* material,
    void* slotsBuffer, size_t slotsBufferSize)
{
    if (!path || !material) return MINT_ERR_INVALID_DATA;

    MintMaterialMemoryRequirements req;
    enum MintResult r = mintload_MmatGetMemoryRequirements(path, &req);
    if (r != MINT_SUCCESS) return r;

    if (slotsBufferSize < req.slotsBufferSize) return MINT_ERR_BUFFER_TOO_SMALL;
    if (slotsBuffer && ((uintptr_t)slotsBuffer & (req.slotsAlignment - 1))) return MINT_ERR_MISALIGNED_BUFFER;

    FILE* f = fopen(path, "rb");
    if (!f) return MINT_ERR_IO;

    uint8_t header[56];
    size_t n = fread(header, 1, 56, f);
    if (n != 56) {
        fclose(f);
        return MINT_ERR_INVALID_DATA;
    }

    uint32_t magic = read_u32_le(header);
    uint32_t version = read_u32_le(header + 4);
    if (magic != MINT_MMAT_MAGIC || version != MINT_MMAT_VERSION) {
        fclose(f);
        return MINT_ERR_INVALID_DATA;
    }

    float baseColor[4], metallic, roughness, emissive[3], alphaCutoff;
    for (int i = 0; i < 4; ++i)
        baseColor[i] = read_f32_le(header + 8 + i*4);
    metallic = read_f32_le(header + 24);
    roughness = read_f32_le(header + 28);
    for (int i = 0; i < 3; ++i)
        emissive[i] = read_f32_le(header + 32 + i*4);
    alphaCutoff = read_f32_le(header + 44);
    uint32_t flags = read_u32_le(header + 48);
    uint32_t textureCount = read_u32_le(header + 52);

    material->magic = MINT_MMAT_MAGIC;
    material->version = MINT_MMAT_VERSION;
    for (int i = 0; i < 4; ++i) material->baseColorFactor[i] = baseColor[i];
    material->metallicFactor = metallic;
    material->roughnessFactor = roughness;
    for (int i = 0; i < 3; ++i) material->emissiveFactor[i] = emissive[i];
    material->alphaCutoff = alphaCutoff;
    material->flags = flags;
    material->textureCount = textureCount;

    if (req.slotsBufferSize > 0 && slotsBuffer) {
        fseek(f, 56, SEEK_SET);
        n = fread(slotsBuffer, 1, req.slotsBufferSize, f);
        if (n != req.slotsBufferSize) {
            fclose(f);
            return MINT_ERR_INVALID_DATA;
        }
    }

    fclose(f);
    return MINT_SUCCESS;
}
