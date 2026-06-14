#ifndef MINTLOAD_MMESH_H
#define MINTLOAD_MMESH_H

#include "base.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MINTLOAD_ATTR_POSITION  (1u << 0)
#define MINTLOAD_ATTR_NORMAL    (1u << 1)
#define MINTLOAD_ATTR_TEXCOORD0 (1u << 2)
#define MINTLOAD_ATTR_COLOR0    (1u << 3)
#define MINTLOAD_ATTR_JOINTS0   (1u << 4)
#define MINTLOAD_ATTR_WEIGHTS0  (1u << 5)
#define MINTLOAD_ATTR_TANGENT   (1u << 6)
#define MINTLOAD_ATTR_TEXCOORD1 (1u << 7)

typedef struct {
    uint32_t  flags;
    uint32_t  vertex_count;
    uint32_t  vertex_offset;
    uint16_t  vertex_stride;
    uint32_t  index_count;
    uint32_t  index_offset;
    int32_t   mat_index;
    float     bounding_min[3];
    float     bounding_max[3];
} MintSubMesh;

typedef struct {
    const uint8_t*  vertex_data;
    const uint8_t*  index_data;
    uint32_t        vertex_count;
    uint32_t        index_count;
    uint32_t        sub_mesh_count;
    uint32_t        lod_count;
    uint32_t        lod_first_submesh[4];
    MintloadMapping _m;
} MintMesh;

MintloadResult mintload_MmeshLoad(const char* path, MintMesh* out);
void           mintload_MmeshUnload(MintMesh* mesh);
uint32_t       mintload_MmeshSubMeshCount(const MintMesh* mesh);
MintSubMesh    mintload_MmeshSubMesh(const MintMesh* mesh, uint32_t index);

#ifdef __cplusplus
}
#endif

#endif
