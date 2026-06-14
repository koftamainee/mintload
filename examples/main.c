#include "mintload/mintload.h"
#include <stdio.h>
#include <string.h>

static const char* tex_type_name(uint32_t t) {
    switch (t) {
        case MINTLOAD_TEX_BASECOLOR:          return "BaseColor";
        case MINTLOAD_TEX_NORMAL:             return "Normal";
        case MINTLOAD_TEX_METALLIC_ROUGHNESS: return "MetallicRoughness";
        case MINTLOAD_TEX_EMISSIVE:           return "Emissive";
        case MINTLOAD_TEX_OCCLUSION:          return "Occlusion";
        default:                              return "Unknown";
    }
}

static const char* anim_prop_name(uint8_t p) {
    switch (p) {
        case MINTLOAD_ANIM_TRANSLATION: return "Translation";
        case MINTLOAD_ANIM_ROTATION:    return "Rotation";
        case MINTLOAD_ANIM_SCALE:       return "Scale";
        case MINTLOAD_ANIM_WEIGHTS:     return "Weights";
        default:                        return "Unknown";
    }
}

static const char* anim_interp_name(uint8_t i) {
    switch (i) {
        case MINTLOAD_ANIM_LINEAR: return "Linear";
        case MINTLOAD_ANIM_STEP:   return "Step";
        case MINTLOAD_ANIM_CUBIC:  return "Cubic";
        default:                   return "Unknown";
    }
}

static void parse_prop_path(const char* prop_path, char* dir, size_t dir_sz, char* stem, size_t stem_sz) {
    size_t len = strlen(prop_path);
    size_t base_len = len;
    if (base_len > 6 && memcmp(prop_path + base_len - 6, ".mprop", 6) == 0)
        base_len -= 6;

    const char* s = prop_path + base_len - 1;
    while (s > prop_path && s[-1] != '/' && s[-1] != '\\')
        s--;
    size_t stem_len = (prop_path + base_len) - s;
    if (stem_len >= stem_sz) stem_len = stem_sz - 1;
    memcpy(stem, s, stem_len);
    stem[stem_len] = '\0';

    size_t parent_len = s - prop_path;
    size_t dir_len = parent_len + stem_len + 1;
    if (dir_len >= dir_sz) dir_len = dir_sz - 1;
    memcpy(dir, prop_path, parent_len);
    memcpy(dir + parent_len, stem, stem_len);
    dir[parent_len + stem_len] = '/';
    dir[dir_len] = '\0';
}

static void load_and_print_mesh(const char* path) {
    MintMesh mesh;
    MintloadResult r = mintload_MmeshLoad(path, &mesh);
    if (r != MINTLOAD_SUCCESS) {
        printf("  [FAIL] mintload_MmeshLoad: %d\n", r);
        return;
    }

    printf("  vertex_data:  %p\n", mesh.vertex_data);
    printf("  index_data:   %p\n", mesh.index_data);
    printf("  vertices:     %u\n", mesh.vertex_count);
    printf("  indices:      %u\n", mesh.index_count);
    printf("  sub_meshes:   %u\n", mesh.sub_mesh_count);
    printf("  lod_count:    %u\n", mesh.lod_count);
    printf("  lod_first:    %u %u %u %u\n",
           mesh.lod_first_submesh[0], mesh.lod_first_submesh[1],
           mesh.lod_first_submesh[2], mesh.lod_first_submesh[3]);
    printf("  size:         %zu bytes (mapped)\n", mesh._m.size);

    uint32_t sm_count = mintload_MmeshSubMeshCount(&mesh);
    printf("  sub-meshes (%u):\n", sm_count);
    for (uint32_t i = 0; i < sm_count && i < 5; i++) {
        MintSubMesh sm = mintload_MmeshSubMesh(&mesh, i);
        printf("    [%u] flags=0x%x vc=%u vo=%u vs=%u ic=%u io=%u  bb=[%.3f..%.3f,%.3f..%.3f,%.3f..%.3f]\n",
               i, sm.flags, sm.vertex_count, sm.vertex_offset, sm.vertex_stride,
               sm.index_count, sm.index_offset,
               sm.bounding_min[0], sm.bounding_max[0],
               sm.bounding_min[1], sm.bounding_max[1],
               sm.bounding_min[2], sm.bounding_max[2]);
    }
    if (sm_count > 5) printf("    ... (%u more)\n", sm_count - 5);

    mintload_MmeshUnload(&mesh);
}

static void load_and_print_material(const char* path) {
    MintMaterial mat;
    MintloadResult r = mintload_MmatLoad(path, &mat);
    if (r != MINTLOAD_SUCCESS) {
        printf("    [FAIL] mintload_MmatLoad: %d\n", r);
        return;
    }

    printf("    bc=[%.3f %.3f %.3f %.3f] metallic=%.3f roughness=%.3f emissive=[%.3f %.3f %.3f] cutoff=%.3f\n",
           mat.base_color[0], mat.base_color[1], mat.base_color[2], mat.base_color[3],
           mat.metallic, mat.roughness,
           mat.emissive[0], mat.emissive[1], mat.emissive[2],
           mat.alpha_cutoff);
    printf("    flags=0x%x tex_count=%u\n", mat.flags, mat.texture_count);

    for (uint32_t i = 0; i < mat.texture_count; i++) {
        printf("      [%u] %s -> %d.ktx2\n", i, tex_type_name(mat.textures[i].type), mat.textures[i].texture_index);
    }

    mintload_MmatUnload(&mat);
}

static void load_and_print_skeleton(const char* path, uint32_t index) {
    MintSkeleton skel;
    MintloadResult r = mintload_MskelLoad(path, &skel);
    if (r != MINTLOAD_SUCCESS) {
        printf("    [FAIL] mintload_MskelLoad: %d\n", r);
        return;
    }

    printf("    [skel %u] bones=%u\n", index, skel.bone_count);
    uint32_t bone_count = mintload_MskelBoneCount(&skel);
    for (uint32_t i = 0; i < bone_count && i < 8; i++) {
        MintBone b = mintload_MskelBone(&skel, i);
        printf("      [%u] \"%.*s\" parent=%d\n", i, (int)b.name_len, b.name, b.parent_index);
    }
    if (bone_count > 8) printf("      ... (%u more)\n", bone_count - 8);

    mintload_MskelUnload(&skel);
}

static void load_and_print_animation(const char* path, uint32_t index) {
    MintAnimation anim;
    MintloadResult r = mintload_ManimLoad(path, &anim);
    if (r != MINTLOAD_SUCCESS) {
        printf("    [FAIL] mintload_ManimLoad: %d\n", r);
        return;
    }

    printf("    [anim %u] \"%.*s\" duration=%.3f channels=%u\n",
           index, (int)anim.name_len, anim.name, anim.duration, anim.channel_count);

    uint32_t ch_count = mintload_ManimChannelCount(&anim);
    for (uint32_t i = 0; i < ch_count && i < 5; i++) {
        MintAnimChannel c = mintload_ManimChannel(&anim, i);
        printf("      [%u] node=%u %s %s comps=%u frames=%u\n",
               i, c.node_index,
               anim_prop_name(c.property), anim_interp_name(c.interpolation),
               c.components, c.frame_count);
    }
    if (ch_count > 5) printf("      ... (%u more)\n", ch_count - 5);

    mintload_ManimUnload(&anim);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <prop_path>\n", argv[0]);
        fprintf(stderr, "  e.g. %s models_minted/just_a_girl.mprop\n", argv[0]);
        return 1;
    }

    const char* prop_path = argv[1];

    char stem[128];
    char dir[256];
    parse_prop_path(prop_path, dir, sizeof(dir), stem, sizeof(stem));

    char mesh_path[256];
    char file_path[256];

    snprintf(mesh_path, sizeof(mesh_path), "%s%s.mmesh", dir, stem);

    MintProp prop;
    MintloadResult r = mintload_MpropLoad(prop_path, &prop);
    if (r != MINTLOAD_SUCCESS) {
        fprintf(stderr, "Failed to load mprop: %d\n", r);
        return 1;
    }

    printf("=== Prefab: %s ===\n", prop_path);
    printf("  entries=%u skeletons=%u animations=%u\n",
           prop.entry_count, prop.skeleton_count, prop.animation_count);

    uint32_t entry_count = mintload_MpropEntryCount(&prop);
    printf("  entries (%u):\n", entry_count);
    for (uint32_t i = 0; i < entry_count; i++) {
        MintPropEntry e = mintload_MpropEntry(&prop, i);
        printf("    [%u] sub_mesh=%d mat=%d skel=%d name=\"%.*s\"\n",
               i, e.sub_mesh_index, e.material_index, e.skeleton_index,
               e.name ? (int)e.name_len : 0, e.name ? e.name : "(null)");
    }

    printf("\n=== Mesh: %s ===\n", mesh_path);
    load_and_print_mesh(mesh_path);

    printf("\n=== Materials ===\n");
    for (uint32_t i = 0; i < entry_count; i++) {
        MintPropEntry e = mintload_MpropEntry(&prop, i);
        if (e.material_index < 0) continue;
        snprintf(file_path, sizeof(file_path), "%s%d.mmat", dir, e.material_index);
        printf("  %s:\n", file_path);
        load_and_print_material(file_path);
    }

    if (prop.skeleton_count > 0) {
        printf("\n=== Skeletons ===\n");
        for (uint32_t i = 0; i < prop.skeleton_count; i++) {
            snprintf(file_path, sizeof(file_path), "%s%d.mskel", dir, i);
            printf("  %s:\n", file_path);
            load_and_print_skeleton(file_path, i);
        }
    }

    if (prop.animation_count > 0) {
        printf("\n=== Animations ===\n");
        for (uint32_t i = 0; i < prop.animation_count; i++) {
            snprintf(file_path, sizeof(file_path), "%s%d.manim", dir, i);
            printf("  %s:\n", file_path);
            load_and_print_animation(file_path, i);
        }
    }

    mintload_MpropUnload(&prop);
    printf("\nDone.\n");
    return 0;
}
