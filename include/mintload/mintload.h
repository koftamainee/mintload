#ifndef MINTLOAD_H
#define MINTLOAD_H

#include "mmesh.h"
#include "mmat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum MintResult {
    MINT_SUCCESS = 0,
    MINT_ERR_INVALID_DATA,
    MINT_ERR_UNSUPPORTED_VERSION,
    MINT_ERR_BUFFER_TOO_SMALL,
    MINT_ERR_MISALIGNED_BUFFER,
    MINT_ERR_IO,
};

#ifdef __cplusplus
}
#endif

#endif
