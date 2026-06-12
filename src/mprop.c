#include "mintload/mprop.h"
#include "utils.h"
#include <string.h>

#define MPROP_HDR 20
#define MPROP_ENTRY_FIXED 80

static uint32_t smagic(void) {
    const uint8_t m[4] = {'M','P','R','P'};
    return read_u32_at(m);
}

MintloadResult mintload_MpropLoad(const char* path, MintProp* out) {
    if (!path || !out) return MINTLOAD_ERR_INVALID_DATA;

    MintloadResult r = mintload_map_file(path, &out->_m);
    if (r != MINTLOAD_SUCCESS) return r;

    const uint8_t* base = (const uint8_t*)out->_m.base;
    size_t size = out->_m.size;

    if (size < MPROP_HDR) { mintload_unmap_file(&out->_m); return MINTLOAD_ERR_INVALID_DATA; }

    uint32_t magic = read_u32_at(base);
    uint32_t ver   = read_u32_at(base + 4);
    if (magic != smagic() || ver != 1) {
        mintload_unmap_file(&out->_m);
        return MINTLOAD_ERR_INVALID_DATA;
    }

    out->entry_count    = read_u32_at(base + 8);
    out->skeleton_count = read_u32_at(base + 12);
    out->animation_count = read_u32_at(base + 16);

    out->_cache[0] = 0;
    out->_cache[1] = (uint64_t)-1;

    return MINTLOAD_SUCCESS;
}

void mintload_MpropUnload(MintProp* prop) {
    if (prop) mintload_unmap_file(&prop->_m);
}

uint32_t mintload_MpropEntryCount(const MintProp* prop) {
    return prop ? prop->entry_count : 0;
}

MintPropEntry mintload_MpropEntry(const MintProp* prop, uint32_t index) {
    MintPropEntry e;
    memset(&e, 0, sizeof(e));
    if (!prop || index >= prop->entry_count) return e;

    const uint8_t* base = (const uint8_t*)prop->_m.base;
    uint64_t* cache = (uint64_t*)prop->_cache;
    uint32_t last_idx = (uint32_t)(cache[1] & 0xFFFFFFFF);
    uint64_t off;
    uint32_t i;

    if (last_idx != (uint32_t)-1 && index >= last_idx && index <= last_idx + 1) {
        off = cache[0];
        i = last_idx;
        if (index == i) {
            goto parse;
        }
        {
            uint32_t nl = read_u32_at(base + off + 76);
            off += MPROP_ENTRY_FIXED + ((nl + 3) & ~3);
            i++;
        }
    } else {
        off = MPROP_HDR;
        i = 0;
    }

    while (i < index) {
        uint32_t nl = read_u32_at(base + off + 76);
        off += MPROP_ENTRY_FIXED + ((nl + 3) & ~3);
        i++;
    }

    cache[0] = off;
    cache[1] = index;

parse:;
    const uint8_t* p = base + off;
    e.sub_mesh_index  = read_i32_at(p);
    e.material_index  = read_i32_at(p + 4);
    e.skeleton_index  = read_i32_at(p + 8);
    for (int j = 0; j < 16; j++)
        e.transform[j] = read_f32_at(p + 12 + j * 4);
    e.name     = (const char*)(p + MPROP_ENTRY_FIXED);
    e.name_len = read_u32_at(p + 76);

    return e;
}
