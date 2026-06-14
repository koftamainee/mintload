#include "mintload/mintload.hpp"
#include <cstdio>
#include <cstring>

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

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <prop_path>\n", argv[0]);
        fprintf(stderr, "  e.g. %s models_minted/just_a_girl.mprop\n", argv[0]);
        return 1;
    }

    char stem[128];
    char dir[256];
    parse_prop_path(argv[1], dir, sizeof(dir), stem, sizeof(stem));

    char path[256];

    auto Prop = Mintload::Prop::Load(argv[1]);
    if (!Prop.IsValid()) {
        fprintf(stderr, "Failed to load prop from %s\n", argv[1]);
        return 1;
    }

    printf("=== Prop: %s ===\n", argv[1]);
    printf("  entries=%u skeletons=%u animations=%u\n",
           Prop.EntryCount(), Prop.SkeletonCount(), Prop.AnimationCount());

    printf("  entries (%u):\n", Prop.EntryCount());
    for (const auto& Entry : Prop) {
        printf("    [sm=%d mat=%d skel=%d] \"%.*s\"\n",
               Entry.sub_mesh_index, Entry.material_index, Entry.skeleton_index,
               (int)Entry.name_len, Entry.name ? Entry.name : "(null)");
    }

    snprintf(path, sizeof(path), "%s%s.mmesh", dir, stem);
    printf("\n=== Mesh ===\n");
    auto Mesh = Mintload::Mesh::Load(path);
    if (!Mesh.IsValid()) {
        fprintf(stderr, "  [FAIL] failed to load mesh\n");
    } else {
        printf("  vertices=%u indices=%u sub_meshes=%u lods=%u\n",
               Mesh.VertexCount(), Mesh.IndexCount(),
               Mesh.SubMeshCount(), Mesh.LodCount());

        printf("  sub-meshes (%u):\n", Mesh.SubMeshCount());
        uint32_t sm_i = 0;
        for (const auto& SM : Mesh) {
            if (sm_i >= 5) { printf("    ... (%u more)\n", Mesh.SubMeshCount() - 5); break; }
            printf("    [%u] flags=0x%x vc=%u ic=%u\n",
                   sm_i, SM.flags, SM.vertex_count, SM.index_count);
            sm_i++;
        }
    }

    printf("\n=== Materials ===\n");
    for (const auto& Entry : Prop) {
        if (Entry.material_index < 0) continue;
        snprintf(path, sizeof(path), "%s%d.mmat", dir, Entry.material_index);
        auto Mat = Mintload::Material::Load(path);
        if (!Mat.IsValid()) continue;
        printf("  [%d] bc=[%.3f %.3f %.3f %.3f] metallic=%.3f roughness=%.3f tex_count=%u\n",
               Entry.material_index,
               Mat.BaseColorR(), Mat.BaseColorG(), Mat.BaseColorB(), Mat.BaseColorA(),
               Mat.Metallic(), Mat.Roughness(), Mat.TextureCount());
        for (const auto& Tex : Mat) {
            printf("    %d.ktx2 (type=%u)\n", Tex.texture_index, Tex.type);
        }
    }

    for (uint32_t i = 0; i < Prop.SkeletonCount(); i++) {
        snprintf(path, sizeof(path), "%s%d.mskel", dir, i);
        auto Skel = Mintload::Skeleton::Load(path);
        if (!Skel.IsValid()) continue;
        printf("\n=== Skeleton [%u] ===\n", i);
        printf("  bones=%u\n", Skel.BoneCount());
        uint32_t b_i = 0;
        for (const auto& Bone : Skel) {
            if (b_i >= 8) { printf("    ... (%u more)\n", Skel.BoneCount() - 8); break; }
            printf("    [%u] \"%.*s\" parent=%d\n", b_i,
                   (int)Bone.name_len, Bone.name, Bone.parent_index);
            b_i++;
        }
    }

    for (uint32_t i = 0; i < Prop.AnimationCount(); i++) {
        snprintf(path, sizeof(path), "%s%d.manim", dir, i);
        auto Anim = Mintload::Animation::Load(path);
        if (!Anim.IsValid()) continue;
        printf("\n=== Animation [%u] ===\n", i);
        printf("  \"%.*s\" duration=%.3f channels=%u\n",
               (int)Anim.NameLength(), Anim.Name(), Anim.Duration(), Anim.ChannelCount());
        uint32_t c_i = 0;
        for (const auto& Ch : Anim) {
            if (c_i >= 5) { printf("    ... (%u more)\n", Anim.ChannelCount() - 5); break; }
            printf("    [%u] node=%u prop=%u frames=%u\n",
                   c_i, Ch.node_index, Ch.property, Ch.frame_count);
            c_i++;
        }
    }

    printf("\nDone.\n");
    return 0;
}
