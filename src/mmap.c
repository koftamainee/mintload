#include "utils.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

MintloadResult mintload_map_file(const char* path, MintloadMapping* mapping) {
#ifdef _WIN32
    HANDLE fd = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fd == INVALID_HANDLE_VALUE) return MINTLOAD_ERR_IO;

    LARGE_INTEGER li;
    if (!GetFileSizeEx(fd, &li)) {
        CloseHandle(fd);
        return MINTLOAD_ERR_IO;
    }
    size_t size = (size_t)li.QuadPart;

    HANDLE fm = CreateFileMapping(fd, NULL, PAGE_READONLY, 0, 0, NULL);
    CloseHandle(fd);
    if (!fm) return MINTLOAD_ERR_IO;

    void* base = MapViewOfFile(fm, FILE_MAP_READ, 0, 0, 0);
    if (!base) {
        CloseHandle(fm);
        return MINTLOAD_ERR_IO;
    }

    mapping->base = base;
    mapping->size = size;
    mapping->_opaque[0] = fm;
    return MINTLOAD_SUCCESS;
#else
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
#endif
}

void mintload_unmap_file(MintloadMapping* mapping) {
    if (!mapping->base) return;
#ifdef _WIN32
    UnmapViewOfFile(mapping->base);
    CloseHandle((HANDLE)mapping->_opaque[0]);
#else
    munmap((void*)mapping->base, mapping->size);
#endif
    mapping->base = NULL;
    mapping->size = 0;
}
