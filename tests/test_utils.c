#include "test_utils.h"
#include <string.h>
#include <time.h>
#include <assert.h>

#ifdef _WIN32
#include <windows.h>
static int get_temp_path(char* buf, size_t size) {
    DWORD len = GetTempPathA((DWORD)size, buf);
    return len > 0 && len < size ? 0 : -1;
}
#else
#include <unistd.h>
static int get_temp_path(char* buf, size_t size) {
    const char* tmpdir = getenv("TMPDIR");
    if (!tmpdir) tmpdir = "/tmp";
    size_t len = strlen(tmpdir);
    if (len + 2 > size) return -1;
    strcpy(buf, tmpdir);
    if (buf[len-1] != '/') strcat(buf, "/");
    return 0;
}
#endif

FILE* create_temp_file(const uint8_t* data, size_t size, const char* suffix, char* out_path, size_t path_size) {
    char temp_dir[256];
    if (get_temp_path(temp_dir, sizeof(temp_dir)) != 0) return NULL;

    char temp_name[512];
    static unsigned long counter = 0;
    sprintf(temp_name, "%smintload_test_%lu_%s", temp_dir, (unsigned long)time(NULL) + counter++, suffix);
    if (out_path && path_size > 0) {
        strncpy(out_path, temp_name, path_size - 1);
        out_path[path_size - 1] = '\0';
    }

    FILE* f = fopen(temp_name, "wb");
    if (!f) return NULL;
    if (fwrite(data, 1, size, f) != size) {
        fclose(f);
        remove(temp_name);
        return NULL;
    }
    rewind(f);
    return f;
}
