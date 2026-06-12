#include "mintload/mskel.h"
#include "utils.h"
#include <string.h>

#define MSKEL_HDR 12

static uint32_t smagic(void) {
    const uint8_t m[4] = {'M','S','K','L'};
    return read_u32_at(m);
}

MintloadResult mintload_MskelLoad(const char* path, MintSkeleton* out) {
    if (!path || !out) return MINTLOAD_ERR_INVALID_DATA;

    MintloadResult r = mintload_map_file(path, &out->_m);
    if (r != MINTLOAD_SUCCESS) return r;

    const uint8_t* base = (const uint8_t*)out->_m.base;
    size_t size = out->_m.size;

    if (size < MSKEL_HDR) { mintload_unmap_file(&out->_m); return MINTLOAD_ERR_INVALID_DATA; }

    uint32_t magic = read_u32_at(base);
    uint32_t ver   = read_u32_at(base + 4);
    if (magic != smagic() || ver != 1) {
        mintload_unmap_file(&out->_m);
        return MINTLOAD_ERR_INVALID_DATA;
    }

    out->bone_count = read_u32_at(base + 8);
    out->_cache[0] = 0;
    out->_cache[1] = (uint64_t)-1;

    return MINTLOAD_SUCCESS;
}

void mintload_MskelUnload(MintSkeleton* skel) {
    if (skel) mintload_unmap_file(&skel->_m);
}

uint32_t mintload_MskelBoneCount(const MintSkeleton* skel) {
    return skel ? skel->bone_count : 0;
}

MintBone mintload_MskelBone(const MintSkeleton* skel, uint32_t index) {
    MintBone b;
    memset(&b, 0, sizeof(b));
    if (!skel || index >= skel->bone_count) return b;

    const uint8_t* base = (const uint8_t*)skel->_m.base;
    uint64_t* cache = (uint64_t*)skel->_cache;
    uint32_t last_idx = (uint32_t)(cache[1] & 0xFFFFFFFF);
    uint64_t off;
    uint32_t i;

    if (last_idx != (uint32_t)-1 && index >= last_idx && index <= last_idx + 1) {
        off = cache[0];
        i = last_idx;
        if (index == i) goto parse;
        {
            uint32_t nl = read_u32_at(base + off);
            off += 4 + ((nl + 3) & ~3) + 4 + 64;
            i++;
        }
    } else {
        off = MSKEL_HDR;
        i = 0;
    }

    while (i < index) {
        uint32_t nl = read_u32_at(base + off);
        off += 4 + ((nl + 3) & ~3) + 4 + 64;
        i++;
    }

    cache[0] = off;
    cache[1] = index;

parse:;
    const uint8_t* p = base + off;
    uint32_t nl = read_u32_at(p);
    b.name     = (const char*)(p + 4);
    b.name_len = nl;
    uint32_t padded = ((nl + 3) & ~3);
    b.parent_index = read_i32_at(p + 4 + padded);
    for (int j = 0; j < 16; j++)
        b.inverse_bind_matrix[j] = read_f32_at(p + 4 + padded + 4 + j * 4);

    return b;
}
