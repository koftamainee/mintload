#include "utils.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

MintloadResult mintload_map_file(const char* path, MintloadMapping* mapping) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return MINTLOAD_ERR_IO;

    struct stat st;
    if (fstat(fd, &st) < 0) {
        close(fd);
        return MINTLOAD_ERR_IO;
    }

    size_t size = (size_t)st.st_size;
    void* base = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (base == MAP_FAILED) return MINTLOAD_ERR_IO;

    mapping->base = base;
    mapping->size = size;
    return MINTLOAD_SUCCESS;
}

void mintload_unmap_file(MintloadMapping* mapping) {
    if (mapping->base) {
        munmap((void*)mapping->base, mapping->size);
        mapping->base = NULL;
        mapping->size = 0;
    }
}
