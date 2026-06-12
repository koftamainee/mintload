#include "mintload/manim.h"
#include "utils.h"
#include <string.h>

static uint32_t smagic(void) {
    const uint8_t m[4] = {'M','A','N','M'};
    return read_u32_at(m);
}

MintloadResult mintload_ManimLoad(const char* path, MintAnimation* out) {
    if (!path || !out) return MINTLOAD_ERR_INVALID_DATA;

    MintloadResult r = mintload_map_file(path, &out->_m);
    if (r != MINTLOAD_SUCCESS) return r;

    const uint8_t* base = (const uint8_t*)out->_m.base;
    size_t size = out->_m.size;

    if (size < 12) { mintload_unmap_file(&out->_m); return MINTLOAD_ERR_INVALID_DATA; }

    uint32_t magic = read_u32_at(base);
    uint32_t ver   = read_u32_at(base + 4);
    if (magic != smagic() || ver != 1) {
        mintload_unmap_file(&out->_m);
        return MINTLOAD_ERR_INVALID_DATA;
    }

    uint32_t nl = read_u32_at(base + 8);
    uint32_t padded = ((nl + 3) & ~3);
    uint64_t off = 12 + padded;

    if (size < off + 12) { mintload_unmap_file(&out->_m); return MINTLOAD_ERR_INVALID_DATA; }

    out->name     = (const char*)(base + 12);
    out->name_len = nl;
    out->duration = read_f64_at(base + off);
    out->channel_count = read_u32_at(base + off + 8);

    out->_cache[0] = 0;
    out->_cache[1] = (uint64_t)-1;

    return MINTLOAD_SUCCESS;
}

void mintload_ManimUnload(MintAnimation* anim) {
    if (anim) mintload_unmap_file(&anim->_m);
}

uint32_t mintload_ManimChannelCount(const MintAnimation* anim) {
    return anim ? anim->channel_count : 0;
}

MintAnimChannel mintload_ManimChannel(const MintAnimation* anim, uint32_t index) {
    MintAnimChannel c;
    memset(&c, 0, sizeof(c));
    if (!anim || index >= anim->channel_count) return c;

    const uint8_t* base = (const uint8_t*)anim->_m.base;
    uint64_t* cache = (uint64_t*)anim->_cache;
    uint32_t last_idx = (uint32_t)(cache[1] & 0xFFFFFFFF);
    uint64_t off;
    uint32_t i;

    uint32_t name_len = read_u32_at(base + 8);
    uint32_t name_pad = ((name_len + 3) & ~3);
    uint64_t ch_start = 12 + name_pad + 8 + 4;

    if (last_idx != (uint32_t)-1 && index >= last_idx && index <= last_idx + 1) {
        off = cache[0];
        i = last_idx;
        if (index == i) goto parse;
        {
            uint32_t fc = read_u32_at(base + off + 10);
            uint16_t co = read_u16_at(base + off + 8);
            off += 14 + (size_t)fc * 4 + (size_t)fc * co * 4;
            i++;
        }
    } else {
        off = ch_start;
        i = 0;
    }

    while (i < index) {
        uint32_t fc = read_u32_at(base + off + 10);
        uint16_t co = read_u16_at(base + off + 8);
        off += 14 + (size_t)fc * 4 + (size_t)fc * co * 4;
        i++;
    }

    cache[0] = off;
    cache[1] = index;

parse:;
    const uint8_t* p = base + off;
    c.node_index    = read_u32_at(p);
    c.property      = p[4];
    c.interpolation = p[5];
    c.components    = read_u16_at(p + 8);
    c.frame_count   = read_u32_at(p + 10);
    c.times  = (const float*)(p + 14);
    c.values = (const float*)(p + 14 + (size_t)c.frame_count * 4);

    return c;
}
