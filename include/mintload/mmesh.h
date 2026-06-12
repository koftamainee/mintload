#ifndef MINTLOAD_MMESH_H
#define MINTLOAD_MMESH_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MINT_MMESH_MAGIC 0x4D4D5348u
#define MINT_MMESH_VERSION 1u

enum MintMeshFlags {
    MINT_MESH_HAS_POSITION  = 1 << 0,
    MINT_MESH_HAS_NORMAL    = 1 << 1,
    MINT_MESH_HAS_TEXCOORD0 = 1 << 2,
    MINT_MESH_HAS_COLOR0    = 1 << 3,
};

typedef struct MintMesh {
    uint32_t magic;
    uint32_t version;
    uint32_t flags;
    uint32_t vertexCount;
    uint32_t indexCount;
    uint32_t vertexStride;
    uint32_t indexFormat;
    float boundingMin[3];
    float boundingMax[3];
} MintMesh;

typedef struct MintMeshMemoryRequirements {
    size_t vertexBufferSize;
    size_t indexBufferSize;
    size_t vertexAlignment;
    size_t indexAlignment;
} MintMeshMemoryRequirements;

enum MintResult mintload_MmeshGetMemoryRequirements(const char* path, MintMeshMemoryRequirements* req);
enum MintResult mintload_MmeshLoad(
    const char* path,
    MintMesh* mesh,
    void* vertexBuffer, size_t vertexBufferSize,
    void* indexBuffer, size_t indexBufferSize
);

#ifdef __cplusplus
}
#endif

#endif
