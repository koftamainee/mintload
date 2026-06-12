#ifndef MINTLOAD_MSKEL_H
#define MINTLOAD_MSKEL_H

#include "base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* name;
    uint32_t    name_len;
    int32_t     parent_index;
    float       inverse_bind_matrix[16];
} MintBone;

typedef struct {
    uint32_t        bone_count;
    MintloadMapping _m;
    uint64_t        _cache[2];
} MintSkeleton;

MintloadResult mintload_MskelLoad(const char* path, MintSkeleton* out);
void           mintload_MskelUnload(MintSkeleton* skel);
uint32_t       mintload_MskelBoneCount(const MintSkeleton* skel);
MintBone       mintload_MskelBone(const MintSkeleton* skel, uint32_t index);

#ifdef __cplusplus
}
#endif

#endif
