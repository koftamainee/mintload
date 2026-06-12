#include "mintload/mintload.h"
#include "test_utils.h"
#include "test_mmat.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void generate_valid_mmat(uint8_t** out_data, size_t* out_size) {
    uint32_t textureCount = 1;
    size_t headerSize = 56;
    size_t slotsSize = textureCount * sizeof(MintTextureSlot);
    size_t total = headerSize + slotsSize;
    uint8_t* data = (uint8_t*)calloc(total, 1);

    uint32_t magic = MINT_MMAT_MAGIC;
    memcpy(data, &magic, 4);
    uint32_t version = MINT_MMAT_VERSION;
    memcpy(data + 4, &version, 4);

    float baseColor[4] = {1.0f, 0.5f, 0.2f, 1.0f};
    memcpy(data + 8, baseColor, 16);
    float metallic = 0.8f;
    memcpy(data + 24, &metallic, 4);
    float roughness = 0.3f;
    memcpy(data + 28, &roughness, 4);
    float emissive[3] = {0.0f, 0.0f, 0.0f};
    memcpy(data + 32, emissive, 12);
    float alphaCutoff = 0.5f;
    memcpy(data + 44, &alphaCutoff, 4);
    uint32_t flags = 0;
    memcpy(data + 48, &flags, 4);
    memcpy(data + 52, &textureCount, 4);

    MintTextureSlot slot;
    slot.type = MINT_TEX_BASECOLOR;
    strcpy(slot.path, "textures/base.ktx2");
    memcpy(data + 56, &slot, sizeof(slot));

    *out_data = data;
    *out_size = total;
}

static void test_mmat_load_valid(void) {
    uint8_t* file_data = NULL;
    size_t file_size = 0;
    generate_valid_mmat(&file_data, &file_size);
    char temp_path[256];
    FILE* f = create_temp_file(file_data, file_size, "mmat", temp_path, sizeof(temp_path));
    fclose(f);

    MintMaterialMemoryRequirements req;
    enum MintResult res = mintload_MmatGetMemoryRequirements(temp_path, &req);
    assert(res == MINT_SUCCESS);
    assert(req.slotsBufferSize == sizeof(MintTextureSlot));
    assert(req.slotsAlignment == 4);

    MintMaterial mat;
    void* slots = aligned_alloc(req.slotsAlignment, req.slotsBufferSize);
    assert(slots != NULL);

    res = mintload_MmatLoad(temp_path, &mat, slots, req.slotsBufferSize);
    assert(res == MINT_SUCCESS);

    assert(mat.magic == MINT_MMAT_MAGIC);
    assert(mat.version == MINT_MMAT_VERSION);
    assert(mat.baseColorFactor[0] == 1.0f);
    assert(mat.baseColorFactor[1] == 0.5f);
    assert(mat.metallicFactor == 0.8f);
    assert(mat.roughnessFactor == 0.3f);
    assert(mat.textureCount == 1);

    MintTextureSlot* slot = (MintTextureSlot*)slots;
    assert(slot->type == MINT_TEX_BASECOLOR);
    assert(strcmp(slot->path, "textures/base.ktx2") == 0);

    free(slots);
    remove(temp_path);
    free(file_data);
}

static void test_mmat_load_small_buffer(void) {
    uint8_t* file_data = NULL;
    size_t file_size = 0;
    generate_valid_mmat(&file_data, &file_size);
    char temp_path[256];
    FILE* f = create_temp_file(file_data, file_size, "mmat", temp_path, sizeof(temp_path));
    fclose(f);

    MintMaterialMemoryRequirements req;
    mintload_MmatGetMemoryRequirements(temp_path, &req);

    MintMaterial mat;
    void* slots = malloc(req.slotsBufferSize - 1);
    enum MintResult res = mintload_MmatLoad(temp_path, &mat, slots, req.slotsBufferSize - 1);
    assert(res == MINT_ERR_BUFFER_TOO_SMALL);

    free(slots);
    remove(temp_path);
    free(file_data);
}

void run_mmat_tests(void) {
    test_mmat_load_valid();
    test_mmat_load_small_buffer();
    printf("MMat tests passed.\n");
}
