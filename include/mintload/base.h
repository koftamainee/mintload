#ifndef MINTLOAD_BASE_H
#define MINTLOAD_BASE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const void* base;
    size_t      size;
    void*       _opaque[4];
} MintloadMapping;

typedef enum {
    MINTLOAD_SUCCESS = 0,
    MINTLOAD_ERR_INVALID_DATA,
    MINTLOAD_ERR_UNSUPPORTED_VERSION,
    MINTLOAD_ERR_IO,
    MINTLOAD_ERR_OUT_OF_MEMORY,
} MintloadResult;

#ifdef __cplusplus
}
#endif

#endif
