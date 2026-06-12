#ifndef MINTLOAD_MANIM_H
#define MINTLOAD_MANIM_H

#include "base.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MINTLOAD_ANIM_TRANSLATION  0
#define MINTLOAD_ANIM_ROTATION     1
#define MINTLOAD_ANIM_SCALE        2
#define MINTLOAD_ANIM_WEIGHTS      3

#define MINTLOAD_ANIM_LINEAR 0
#define MINTLOAD_ANIM_STEP   1
#define MINTLOAD_ANIM_CUBIC  2

typedef struct {
    uint32_t      node_index;
    uint8_t       property;
    uint8_t       interpolation;
    uint16_t      components;
    uint32_t      frame_count;
    const float*  times;
    const float*  values;
} MintAnimChannel;

typedef struct {
    const char*     name;
    uint32_t        name_len;
    double          duration;
    uint32_t        channel_count;
    MintloadMapping _m;
    uint64_t        _cache[2];
} MintAnimation;

MintloadResult mintload_ManimLoad(const char* path, MintAnimation* out);
void           mintload_ManimUnload(MintAnimation* anim);
uint32_t       mintload_ManimChannelCount(const MintAnimation* anim);
MintAnimChannel mintload_ManimChannel(const MintAnimation* anim, uint32_t index);

#ifdef __cplusplus
}
#endif

#endif
