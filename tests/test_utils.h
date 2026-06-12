#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

FILE* create_temp_file(const uint8_t* data, size_t size, const char* suffix, char* out_path, size_t path_size);

#endif
