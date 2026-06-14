#ifndef MINTLOAD_MPROP_H
#define MINTLOAD_MPROP_H

#include "base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t     sub_mesh_index;
    int32_t      material_index;
    int32_t      skeleton_index;
    float        transform[16];
    const char*  name;
    uint32_t     name_len;
} MintPropEntry;

typedef struct {
    uint32_t        entry_count;
    uint32_t        skeleton_count;
    uint32_t        animation_count;
    MintloadMapping _m;
    uint64_t        _cache[2];
} MintProp;

MintloadResult mintload_MpropLoad(const char* path, MintProp* out);
void           mintload_MpropUnload(MintProp* prop);
uint32_t       mintload_MpropEntryCount(const MintProp* prop);
MintPropEntry  mintload_MpropEntry(const MintProp* prop, uint32_t index);

#ifdef __cplusplus
}
#endif

#endif
