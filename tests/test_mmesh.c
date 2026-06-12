#include "mintload/mintload.h"
#include "test_utils.h"
#include "test_mmesh.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void generate_valid_mmesh(uint8_t** out_data, size_t* out_size) {
    uint32_t vertexCount = 3;
    uint32_t indexCount = 3;
    uint32_t vertexStride = 12;
    uint32_t indexFormat = 0;
    uint32_t flags = MINT_MESH_HAS_POSITION;

    size_t verticesSize = (size_t)vertexCount * vertexStride;
    size_t indicesSize = (size_t)indexCount * (indexFormat == 0 ? 2 : 4);
    size_t total = 64 + verticesSize + indicesSize;
    uint8_t* data = (uint8_t*)malloc(total);
    memset(data, 0, total);

    uint32_t magic = MINT_MMESH_MAGIC;
    memcpy(data, &magic, 4);
    uint32_t version = MINT_MMESH_VERSION;
    memcpy(data + 4, &version, 4);
    memcpy(data + 8, &flags, 4);
    memcpy(data + 12, &vertexCount, 4);
    memcpy(data + 16, &indexCount, 4);
    memcpy(data + 20, &vertexStride, 4);
    memcpy(data + 24, &indexFormat, 4);
    float min[3] = {-1.0f, -1.0f, -1.0f};
    float max[3] = {1.0f, 1.0f, 1.0f};
    memcpy(data + 28, min, 12);
    memcpy(data + 40, max, 12);

    float vertices[9] = {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         0.0f,  1.0f, 0.0f
    };
    memcpy(data + 64, vertices, verticesSize);

    uint16_t indices[3] = {0,1,2};
    memcpy(data + 64 + verticesSize, indices, indicesSize);

    *out_data = data;
    *out_size = total;
}

static void test_mmesh_load_valid(void) {
    uint8_t* file_data = NULL;
    size_t file_size = 0;
    generate_valid_mmesh(&file_data, &file_size);
    assert(file_data != NULL);

    char temp_path[256];
    FILE* f = create_temp_file(file_data, file_size, "mmesh", temp_path, sizeof(temp_path));
    assert(f != NULL);
    fclose(f);

    MintMeshMemoryRequirements req;
    enum MintResult res = mintload_MmeshGetMemoryRequirements(temp_path, &req);
    assert(res == MINT_SUCCESS);
    assert(req.vertexBufferSize == 3 * 12);
    assert(req.indexBufferSize == 3 * 2);
    assert(req.vertexAlignment == 4);
    assert(req.indexAlignment == 2);

    MintMesh mesh;
    void* vertices = aligned_alloc(req.vertexAlignment, req.vertexBufferSize);
    void* indices = aligned_alloc(req.indexAlignment, req.indexBufferSize);
    assert(vertices != NULL);
    assert(indices != NULL);

    res = mintload_MmeshLoad(temp_path, &mesh, vertices, req.vertexBufferSize, indices, req.indexBufferSize);
    assert(res == MINT_SUCCESS);

    assert(mesh.magic == MINT_MMESH_MAGIC);
    assert(mesh.version == MINT_MMESH_VERSION);
    assert(mesh.flags == MINT_MESH_HAS_POSITION);
    assert(mesh.vertexCount == 3);
    assert(mesh.indexCount == 3);
    assert(mesh.vertexStride == 12);
    assert(mesh.indexFormat == 0);
    assert(mesh.boundingMin[0] == -1.0f);
    assert(mesh.boundingMax[0] == 1.0f);

    float* vdata = (float*)vertices;
    assert(vdata[0] == -1.0f && vdata[1] == -1.0f && vdata[2] == 0.0f);
    assert(vdata[3] ==  1.0f && vdata[4] == -1.0f && vdata[5] == 0.0f);
    assert(vdata[6] ==  0.0f && vdata[7] ==  1.0f && vdata[8] == 0.0f);

    uint16_t* idata = (uint16_t*)indices;
    assert(idata[0] == 0 && idata[1] == 1 && idata[2] == 2);

    free(vertices);
    free(indices);
    remove(temp_path);
    free(file_data);
}

static void test_mmesh_load_nonexistent(void) {
    MintMeshMemoryRequirements req;
    enum MintResult res = mintload_MmeshGetMemoryRequirements("/nonexistent/file.mmesh", &req);
    assert(res == MINT_ERR_IO);

    MintMesh mesh;
    void* buf = malloc(1024);
    res = mintload_MmeshLoad("/nonexistent/file.mmesh", &mesh, buf, 1024, buf, 1024);
    assert(res == MINT_ERR_IO);
    free(buf);
}

static void test_mmesh_load_small_buffer(void) {
    uint8_t* file_data = NULL;
    size_t file_size = 0;
    generate_valid_mmesh(&file_data, &file_size);
    char temp_path[256];
    FILE* f = create_temp_file(file_data, file_size, "mmesh", temp_path, sizeof(temp_path));
    fclose(f);

    MintMeshMemoryRequirements req;
    mintload_MmeshGetMemoryRequirements(temp_path, &req);

    MintMesh mesh;
    void* vertices = malloc(req.vertexBufferSize - 1);
    void* indices = malloc(req.indexBufferSize);
    enum MintResult res = mintload_MmeshLoad(temp_path, &mesh, vertices, req.vertexBufferSize - 1, indices, req.indexBufferSize);
    assert(res == MINT_ERR_BUFFER_TOO_SMALL);

    free(vertices);
    free(indices);
    remove(temp_path);
    free(file_data);
}

void run_mmesh_tests(void) {
    test_mmesh_load_valid();
    test_mmesh_load_nonexistent();
    test_mmesh_load_small_buffer();
    printf("MMesh tests passed.\n");
}
