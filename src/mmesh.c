#include "mintload/mintload.h"
#include "utils.h"
#include <stdio.h>

enum MintResult mintload_MmeshGetMemoryRequirements(const char* path, MintMeshMemoryRequirements* req) {
    if (!path || !req) return MINT_ERR_INVALID_DATA;

    FILE* f = fopen(path, "rb");
    if (!f) return MINT_ERR_IO;

    uint8_t header[64];
    size_t n = fread(header, 1, 64, f);
    fclose(f);

    if (n < 64) return MINT_ERR_INVALID_DATA;

    uint32_t magic = read_u32_le(header);
    if (magic != MINT_MMESH_MAGIC) return MINT_ERR_INVALID_DATA;

    uint32_t version = read_u32_le(header + 4);
    if (version != MINT_MMESH_VERSION) return MINT_ERR_UNSUPPORTED_VERSION;

    uint32_t vertexCount = read_u32_le(header + 12);
    uint32_t indexCount = read_u32_le(header + 16);
    uint32_t vertexStride = read_u32_le(header + 20);
    uint32_t indexFormat = read_u32_le(header + 24);

    req->vertexBufferSize = (size_t)vertexCount * vertexStride;
    req->indexBufferSize = (indexCount > 0) ? (size_t)indexCount * (indexFormat == 0 ? 2 : 4) : 0;
    req->vertexAlignment = 4;
    req->indexAlignment = (indexFormat == 0) ? 2 : 4;
    return MINT_SUCCESS;
}

enum MintResult mintload_MmeshLoad(
    const char* path,
    MintMesh* mesh,
    void* vertexBuffer, size_t vertexBufferSize,
    void* indexBuffer, size_t indexBufferSize)
{
    if (!path || !mesh) return MINT_ERR_INVALID_DATA;

    MintMeshMemoryRequirements req;
    enum MintResult r = mintload_MmeshGetMemoryRequirements(path, &req);
    if (r != MINT_SUCCESS) return r;

    if (vertexBufferSize < req.vertexBufferSize) return MINT_ERR_BUFFER_TOO_SMALL;
    if (indexBufferSize < req.indexBufferSize) return MINT_ERR_BUFFER_TOO_SMALL;
    if ((uintptr_t)vertexBuffer & (req.vertexAlignment - 1)) return MINT_ERR_MISALIGNED_BUFFER;
    if (indexBuffer && ((uintptr_t)indexBuffer & (req.indexAlignment - 1))) return MINT_ERR_MISALIGNED_BUFFER;

    FILE* f = fopen(path, "rb");
    if (!f) return MINT_ERR_IO;

    uint8_t header[64];
    size_t n = fread(header, 1, 64, f);
    if (n != 64) {
        fclose(f);
        return MINT_ERR_INVALID_DATA;
    }

    uint32_t magic = read_u32_le(header);
    uint32_t version = read_u32_le(header + 4);
    if (magic != MINT_MMESH_MAGIC || version != MINT_MMESH_VERSION) {
        fclose(f);
        return MINT_ERR_INVALID_DATA;
    }

    uint32_t flags = read_u32_le(header + 8);
    uint32_t vertexCount = read_u32_le(header + 12);
    uint32_t indexCount = read_u32_le(header + 16);
    uint32_t vertexStride = read_u32_le(header + 20);
    uint32_t indexFormat = read_u32_le(header + 24);
    float bmin[3], bmax[3];
    for (int i = 0; i < 3; ++i) {
        bmin[i] = read_f32_le(header + 28 + i*4);
        bmax[i] = read_f32_le(header + 40 + i*4);
    }

    mesh->magic = MINT_MMESH_MAGIC;
    mesh->version = MINT_MMESH_VERSION;
    mesh->flags = flags;
    mesh->vertexCount = vertexCount;
    mesh->indexCount = indexCount;
    mesh->vertexStride = vertexStride;
    mesh->indexFormat = indexFormat;
    for (int i = 0; i < 3; ++i) {
        mesh->boundingMin[i] = bmin[i];
        mesh->boundingMax[i] = bmax[i];
    }

    fseek(f, 64, SEEK_SET);
    size_t vsize = req.vertexBufferSize;
    if (vsize > 0 && vertexBuffer) {
        n = fread(vertexBuffer, 1, vsize, f);
        if (n != vsize) {
            fclose(f);
            return MINT_ERR_INVALID_DATA;
        }
    }

    if (req.indexBufferSize > 0 && indexBuffer) {
        n = fread(indexBuffer, 1, req.indexBufferSize, f);
        if (n != req.indexBufferSize) {
            fclose(f);
            return MINT_ERR_INVALID_DATA;
        }
    }

    fclose(f);
    return MINT_SUCCESS;
}
