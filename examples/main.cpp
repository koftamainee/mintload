#include "mintload/mintload.hpp"
#include <cstdio>
#include <cstring>

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <prop_path>\n", argv[0]);
        fprintf(stderr, "  e.g. %s models_minted/just_a_girl.mprop\n", argv[0]);
        return 1;
    }

    auto Prefab = Mintload::Prefab::Load(argv[1]);
    if (!Prefab.IsValid()) {
        fprintf(stderr, "Failed to load prefab from %s\n", argv[1]);
        return 1;
    }

    printf("=== Prefab: %s ===\n", argv[1]);
    printf("  entries=%u materials=%u skeletons=%u animations=%u\n",
           Prefab.EntryCount(), Prefab.MaterialCount(),
           Prefab.SkeletonCount(), Prefab.AnimationCount());

    printf("  entries (%u):\n", Prefab.EntryCount());
    for (const auto& Entry : Prefab) {
        printf("    [sm=%d mat=%d skel=%d] \"%.*s\"\n",
               Entry.sub_mesh_index, Entry.material_index, Entry.skeleton_index,
               (int)Entry.name_len, Entry.name ? Entry.name : "(null)");
    }

    printf("\n=== Mesh ===\n");
    const auto& Mesh = Prefab.GetMesh();
    printf("  vertices=%u indices=%u sub_meshes=%u lods=%u\n",
           Mesh.VertexCount(), Mesh.IndexCount(),
           Mesh.SubMeshCount(), Mesh.LodCount());

    printf("  sub-meshes (%u):\n", Mesh.SubMeshCount());
    uint32_t sm_i = 0;
    for (const auto& SM : Mesh) {
        if (sm_i >= 5) { printf("    ... (%u more)\n", Mesh.SubMeshCount() - 5); break; }
        printf("    [%u] flags=0x%x vc=%u ic=%u mat=%d\n",
               sm_i, SM.flags, SM.vertex_count, SM.index_count, SM.mat_index);
        sm_i++;
    }

    printf("\n=== Materials ===\n");
    for (uint32_t i = 0; i < Prefab.MaterialCount(); i++) {
        const auto* Mat = Prefab.GetMaterial(i);
        if (!Mat) continue;
        printf("  [%u] bc=[%.3f %.3f %.3f %.3f] metallic=%.3f roughness=%.3f tex_count=%u\n",
               i, Mat->BaseColorR(), Mat->BaseColorG(), Mat->BaseColorB(), Mat->BaseColorA(),
               Mat->Metallic(), Mat->Roughness(), Mat->TextureCount());
        for (const auto& Tex : *Mat) {
            printf("    %d.ktx2 (type=%u)\n", Tex.texture_index, Tex.type);
        }
    }

    for (uint32_t i = 0; i < Prefab.SkeletonCount(); i++) {
        const auto* Skel = Prefab.GetSkeleton(i);
        if (!Skel) continue;
        printf("\n=== Skeleton [%u] ===\n", i);
        printf("  bones=%u\n", Skel->BoneCount());
        uint32_t b_i = 0;
        for (const auto& Bone : *Skel) {
            if (b_i >= 8) { printf("    ... (%u more)\n", Skel->BoneCount() - 8); break; }
            printf("    [%u] \"%.*s\" parent=%d\n", b_i,
                   (int)Bone.name_len, Bone.name, Bone.parent_index);
            b_i++;
        }
    }

    for (uint32_t i = 0; i < Prefab.AnimationCount(); i++) {
        const auto* Anim = Prefab.GetAnimation(i);
        if (!Anim) continue;
        printf("\n=== Animation [%u] ===\n", i);
        printf("  \"%.*s\" duration=%.3f channels=%u\n",
               (int)Anim->NameLength(), Anim->Name(), Anim->Duration(), Anim->ChannelCount());
        uint32_t c_i = 0;
        for (const auto& Ch : *Anim) {
            if (c_i >= 5) { printf("    ... (%u more)\n", Anim->ChannelCount() - 5); break; }
            printf("    [%u] node=%u prop=%u frames=%u\n",
                   c_i, Ch.node_index, Ch.property, Ch.frame_count);
            c_i++;
        }
    }

    printf("\nDone.\n");
    return 0;
}
