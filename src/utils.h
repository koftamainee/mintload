#ifndef MINTLOAD_UTILS_H
#define MINTLOAD_UTILS_H

#include <stdint.h>
#include <string.h>

static inline uint32_t read_u32_le(const uint8_t* p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static inline float read_f32_le(const uint8_t* p) {
    uint32_t u = read_u32_le(p);
    float f;
    memcpy(&f, &u, 4);
    return f;
}

#endif
