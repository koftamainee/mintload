#include "mintload/mmesh.h"
#include "utils.h"
#include <string.h>

#define MMESH_HDR 64
#define SM_SIZE  52

static uint32_t smagic(void) {
    const uint8_t m[4] = {'M','M','S','H'};
    return read_u32_at(m);
}

MintloadResult mintload_MmeshLoad(const char* path, MintMesh* out) {
    if (!path || !out) return MINTLOAD_ERR_INVALID_DATA;

    MintloadResult r = mintload_map_file(path, &out->_m);
    if (r != MINTLOAD_SUCCESS) return r;

    const uint8_t* base = (const uint8_t*)out->_m.base;
    size_t size = out->_m.size;

    if (size < MMESH_HDR) { mintload_unmap_file(&out->_m); return MINTLOAD_ERR_INVALID_DATA; }

    uint32_t magic  = read_u32_at(base);
    uint32_t ver    = read_u32_at(base + 4);
    if (magic != smagic() || ver != 1) {
        mintload_unmap_file(&out->_m);
        return MINTLOAD_ERR_INVALID_DATA;
    }

    uint32_t sm_count = read_u32_at(base + 8);
    uint32_t flags    = read_u32_at(base + 12);
    uint32_t vtx_tot  = read_u32_at(base + 16);
    uint32_t idx_tot  = read_u32_at(base + 20);
    uint32_t lod_cnt  = read_u32_at(base + 24);

    uint32_t lod_first[4];
    for (int i = 0; i < 4; i++) lod_first[i] = read_u32_at(base + 28 + i * 4);

    if (sm_count > (size - MMESH_HDR) / SM_SIZE) {
        mintload_unmap_file(&out->_m); return MINTLOAD_ERR_INVALID_DATA;
    }
    size_t table_bytes = (size_t)sm_count * SM_SIZE;
    size_t data_off    = MMESH_HDR + table_bytes;

    if (size < data_off) { mintload_unmap_file(&out->_m); return MINTLOAD_ERR_INVALID_DATA; }

    size_t vtx_bytes = 0;
    for (uint32_t i = 0; i < sm_count; i++) {
        uint32_t vc = read_u32_at(base + MMESH_HDR + i * SM_SIZE + 4);
        uint16_t vs = read_u16_at(base + MMESH_HDR + i * SM_SIZE + 12);
        if (vs > 256 || vc > 1 << 24) {
            mintload_unmap_file(&out->_m); return MINTLOAD_ERR_INVALID_DATA;
        }
        size_t entry = (size_t)vc * vs;
        if (entry > size - data_off - vtx_bytes) {
            mintload_unmap_file(&out->_m); return MINTLOAD_ERR_INVALID_DATA;
        }
        vtx_bytes += entry;
    }

    size_t idx_bytes = (size_t)idx_tot * 4;
    if (idx_tot && (idx_bytes > size - data_off - vtx_bytes)) {
        mintload_unmap_file(&out->_m); return MINTLOAD_ERR_INVALID_DATA;
    }

    out->vertex_data = base + data_off;
    out->index_data  = idx_tot ? (base + data_off + vtx_bytes) : NULL;
    out->vertex_count       = vtx_tot;
    out->index_count        = idx_tot;
    out->sub_mesh_count     = sm_count;
    out->lod_count          = lod_cnt;
    for (int i = 0; i < 4; i++) out->lod_first_submesh[i] = lod_first[i];

    return MINTLOAD_SUCCESS;
}

void mintload_MmeshUnload(MintMesh* mesh) {
    if (mesh) mintload_unmap_file(&mesh->_m);
}

uint32_t mintload_MmeshSubMeshCount(const MintMesh* mesh) {
    return mesh ? mesh->sub_mesh_count : 0;
}

MintSubMesh mintload_MmeshSubMesh(const MintMesh* mesh, uint32_t index) {
    MintSubMesh sm;
    memset(&sm, 0, sizeof(sm));
    if (!mesh || index >= mesh->sub_mesh_count) return sm;

    const uint8_t* p = (const uint8_t*)mesh->_m.base + MMESH_HDR + index * SM_SIZE;

    sm.flags         = read_u32_at(p);
    sm.vertex_count  = read_u32_at(p + 4);
    sm.vertex_offset = read_u32_at(p + 8);
    sm.vertex_stride = read_u16_at(p + 12);
    sm.index_count   = read_u32_at(p + 16);
    sm.index_offset  = read_u32_at(p + 20);
    sm.mat_index     = read_i32_at(p + 24);
    sm.bounding_min[0] = read_f32_at(p + 28);
    sm.bounding_min[1] = read_f32_at(p + 32);
    sm.bounding_min[2] = read_f32_at(p + 36);
    sm.bounding_max[0] = read_f32_at(p + 40);
    sm.bounding_max[1] = read_f32_at(p + 44);
    sm.bounding_max[2] = read_f32_at(p + 48);

    return sm;
}
