#ifndef MINTLOAD_UTILS_H
#define MINTLOAD_UTILS_H

#include "mintload/base.h"
#include <stdint.h>
#include <string.h>

static inline uint32_t read_u32_le(const uint8_t** p) {
    const uint8_t* d = *p;
    uint32_t v = (uint32_t)d[0] | ((uint32_t)d[1] << 8) |
                 ((uint32_t)d[2] << 16) | ((uint32_t)d[3] << 24);
    *p += 4;
    return v;
}

static inline uint32_t read_u32_at(const uint8_t* p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static inline uint16_t read_u16_at(const uint8_t* p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static inline uint16_t read_u16_le(const uint8_t** p) {
    uint16_t v = read_u16_at(*p);
    *p += 2;
    return v;
}

static inline int32_t read_i32_le(const uint8_t** p) {
    return (int32_t)read_u32_le(p);
}

static inline int32_t read_i32_at(const uint8_t* p) {
    return (int32_t)read_u32_at(p);
}

static inline float read_f32_at(const uint8_t* p) {
    uint32_t u = read_u32_at(p);
    float f;
    memcpy(&f, &u, 4);
    return f;
}

static inline double read_f64_at(const uint8_t* p) {
    uint64_t u = (uint64_t)read_u32_at(p) | ((uint64_t)read_u32_at(p + 4) << 32);
    double d;
    memcpy(&d, &u, 8);
    return d;
}

static inline float read_f32_le(const uint8_t** p) {
    uint32_t u = read_u32_le(p);
    float f;
    memcpy(&f, &u, 4);
    return f;
}

static inline double read_f64_le(const uint8_t** p) {
    uint64_t lo = read_u32_le(p);
    uint64_t hi = read_u32_le(p);
    uint64_t u = lo | (hi << 32);
    double d;
    memcpy(&d, &u, 8);
    return d;
}

MintloadResult mintload_map_file(const char* path, MintloadMapping* mapping);
void mintload_unmap_file(MintloadMapping* mapping);

#endif
