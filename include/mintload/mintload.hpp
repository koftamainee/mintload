#ifndef MINTLOAD_MINTLOAD_HPP
#define MINTLOAD_MINTLOAD_HPP

#include "mintload/mintload.h"
#include <cstring>
#include <cstdio>
#include <cstddef>

namespace Mintload {

template<typename T>
struct Span {
    T*     data_ = nullptr;
    size_t size_ = 0;

    Span() = default;
    Span(T* data, size_t size) : data_(data), size_(size) {}

    T*     begin() const { return data_; }
    T*     end()   const { return data_ + size_; }
    T&     operator[](size_t i) const { return data_[i]; }
    size_t size()  const { return size_; }
    bool   empty() const { return size_ == 0; }
    T*     data()  const { return data_; }
};

template<typename T, typename C, T (*Getter)(const C*, uint32_t)>
struct CIter {
    const C* ctx;
    uint32_t idx;

    T        operator*()  const { return Getter(ctx, idx); }
    CIter&   operator++()       { ++idx; return *this; }
    bool     operator!=(const CIter& o) const { return idx != o.idx; }
};

struct Mesh {
    MintMesh m_ = {};

    static Mesh Load(const char* path) {
        Mesh m;
        mintload_MmeshLoad(path, &m.m_);
        return m;
    }

    ~Mesh() { mintload_MmeshUnload(&m_); }
    Mesh() = default;
    Mesh(Mesh&& o) noexcept : m_(o.m_) { o.m_ = {}; }
    Mesh& operator=(Mesh&& o) noexcept {
        if (this != &o) {
            mintload_MmeshUnload(&m_);
            m_ = o.m_;
            o.m_ = {};
        }
        return *this;
    }
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    bool     IsValid()      const { return m_._m.base != nullptr; }
    uint32_t VertexCount()  const { return m_.vertex_count; }
    uint32_t IndexCount()   const { return m_.index_count; }
    uint32_t SubMeshCount() const { return m_.sub_mesh_count; }
    MintSubMesh SubMeshAt(uint32_t i) const { return mintload_MmeshSubMesh(&m_, i); }
    uint32_t LodCount()     const { return m_.lod_count; }
    const uint32_t* LodFirstSubmesh() const { return m_.lod_first_submesh; }

    size_t TotalVertexBytes() const {
        size_t total = 0;
        for (uint32_t i = 0; i < m_.sub_mesh_count; i++) {
            auto sm = mintload_MmeshSubMesh(&m_, i);
            total += (size_t)sm.vertex_count * sm.vertex_stride;
        }
        return total;
    }

    size_t TotalIndexBytes() const {
        return (size_t)m_.index_count * 4;
    }

    Span<const uint8_t> RawVertexData() const { return {m_.vertex_data, TotalVertexBytes()}; }
    Span<const uint8_t> RawIndexData()  const { return {m_.index_data,  TotalIndexBytes()};  }

    const uint8_t* SubMeshVertexData(const MintSubMesh& sm) const {
        return m_.vertex_data + sm.vertex_offset;
    }

    const uint8_t* SubMeshIndexData(const MintSubMesh& sm) const {
        return m_.index_data + sm.index_offset;
    }

    auto begin() const { return CIter<MintSubMesh, MintMesh, mintload_MmeshSubMesh>{&m_, 0}; }
    auto end()   const { return CIter<MintSubMesh, MintMesh, mintload_MmeshSubMesh>{&m_, m_.sub_mesh_count}; }
};

struct Material {
    MintMaterial m_ = {};

    static Material Load(const char* path) {
        Material m;
        mintload_MmatLoad(path, &m.m_);
        return m;
    }

    ~Material() { mintload_MmatUnload(&m_); }
    Material() = default;
    Material(Material&& o) noexcept : m_(o.m_) { o.m_ = {}; }
    Material& operator=(Material&& o) noexcept {
        if (this != &o) {
            mintload_MmatUnload(&m_);
            m_ = o.m_;
            o.m_ = {};
        }
        return *this;
    }
    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;

    bool     IsValid()       const { return m_._m.base != nullptr; }
    uint32_t TextureCount()  const { return m_.texture_count; }
    float    NormalScale()   const { return m_.normal_scale; }
    float    EmissiveStrength() const { return m_.emissive_strength; }
    Span<const MintTextureSlot> Textures() const { return {m_.textures, m_.texture_count}; }
    uint32_t AlphaMode()     const { return (m_.flags >> 1) & 3; }
    float    AlphaCutoff()   const { return m_.alpha_cutoff; }
    uint32_t Flags()         const { return m_.flags; }
    float    BaseColorR()    const { return m_.base_color[0]; }
    float    BaseColorG()    const { return m_.base_color[1]; }
    float    BaseColorB()    const { return m_.base_color[2]; }
    float    BaseColorA()    const { return m_.base_color[3]; }
    float    Metallic()      const { return m_.metallic; }
    float    Roughness()     const { return m_.roughness; }
    float    EmissiveR()     const { return m_.emissive[0]; }
    float    EmissiveG()     const { return m_.emissive[1]; }
    float    EmissiveB()     const { return m_.emissive[2]; }
    const float* BaseColor()  const { return m_.base_color; }
    const float* Emissive()   const { return m_.emissive; }

    auto begin() const { return m_.textures; }
    auto end()   const { return m_.textures + m_.texture_count; }
};

struct Skeleton {
    MintSkeleton m_ = {};

    static Skeleton Load(const char* path) {
        Skeleton s;
        mintload_MskelLoad(path, &s.m_);
        return s;
    }

    ~Skeleton() { mintload_MskelUnload(&m_); }
    Skeleton() = default;
    Skeleton(Skeleton&& o) noexcept : m_(o.m_) { o.m_ = {}; }
    Skeleton& operator=(Skeleton&& o) noexcept {
        if (this != &o) {
            mintload_MskelUnload(&m_);
            m_ = o.m_;
            o.m_ = {};
        }
        return *this;
    }
    Skeleton(const Skeleton&) = delete;
    Skeleton& operator=(const Skeleton&) = delete;

    bool     IsValid()    const { return m_._m.base != nullptr; }
    uint32_t BoneCount()  const { return m_.bone_count; }
    MintBone BoneAt(uint32_t i) const { return mintload_MskelBone(&m_, i); }

    auto begin() const { return CIter<MintBone, MintSkeleton, mintload_MskelBone>{&m_, 0}; }
    auto end()   const { return CIter<MintBone, MintSkeleton, mintload_MskelBone>{&m_, m_.bone_count}; }
};

struct Animation {
    MintAnimation m_ = {};

    static Animation Load(const char* path) {
        Animation a;
        mintload_ManimLoad(path, &a.m_);
        return a;
    }

    ~Animation() { mintload_ManimUnload(&m_); }
    Animation() = default;
    Animation(Animation&& o) noexcept : m_(o.m_) { o.m_ = {}; }
    Animation& operator=(Animation&& o) noexcept {
        if (this != &o) {
            mintload_ManimUnload(&m_);
            m_ = o.m_;
            o.m_ = {};
        }
        return *this;
    }
    Animation(const Animation&) = delete;
    Animation& operator=(const Animation&) = delete;

    bool     IsValid()       const { return m_._m.base != nullptr; }
    uint32_t ChannelCount()  const { return m_.channel_count; }
    MintAnimChannel ChannelAt(uint32_t i) const { return mintload_ManimChannel(&m_, i); }
    double   Duration()      const { return m_.duration; }
    const char* Name()       const { return m_.name; }
    uint32_t NameLength()    const { return m_.name_len; }

    auto begin() const { return CIter<MintAnimChannel, MintAnimation, mintload_ManimChannel>{&m_, 0}; }
    auto end()   const { return CIter<MintAnimChannel, MintAnimation, mintload_ManimChannel>{&m_, m_.channel_count}; }
};

struct Prop {
    MintProp m_ = {};

    static Prop Load(const char* path) {
        Prop p;
        mintload_MpropLoad(path, &p.m_);
        return p;
    }

    ~Prop() { mintload_MpropUnload(&m_); }
    Prop() = default;
    Prop(Prop&& o) noexcept : m_(o.m_) { o.m_ = {}; }
    Prop& operator=(Prop&& o) noexcept {
        if (this != &o) {
            mintload_MpropUnload(&m_);
            m_ = o.m_;
            o.m_ = {};
        }
        return *this;
    }
    Prop(const Prop&) = delete;
    Prop& operator=(const Prop&) = delete;

    bool     IsValid()        const { return m_._m.base != nullptr; }
    uint32_t EntryCount()     const { return m_.entry_count; }
    MintPropEntry EntryAt(uint32_t i) const { return mintload_MpropEntry(&m_, i); }
    uint32_t SkeletonCount()  const { return m_.skeleton_count; }
    uint32_t AnimationCount() const { return m_.animation_count; }

    auto begin() const { return CIter<MintPropEntry, MintProp, mintload_MpropEntry>{&m_, 0}; }
    auto end()   const { return CIter<MintPropEntry, MintProp, mintload_MpropEntry>{&m_, m_.entry_count}; }
};

struct Prefab {
    static constexpr uint32_t kMaxMats  = 32;
    static constexpr uint32_t kMaxSkels = 8;
    static constexpr uint32_t kMaxAnims = 8;

    Prop       prop_;
    Mesh       mesh_;
    Material   materials_[kMaxMats];
    Skeleton   skeletons_[kMaxSkels];
    Animation  animations_[kMaxAnims];
    uint32_t   matCnt_  = 0;
    uint32_t   skelCnt_ = 0;
    uint32_t   animCnt_ = 0;
    bool       valid_   = false;

    static Prefab Load(const char* propPath,
                        uint32_t maxMats  = kMaxMats,
                        uint32_t maxSkels = kMaxSkels,
                        uint32_t maxAnims = kMaxAnims)
    {
        Prefab p;
        if (!propPath || !*propPath) return p;

        char buf[512];
        size_t len = strlen(propPath);
        if (len >= sizeof(buf) - 1) return p;
        memcpy(buf, propPath, len + 1);

        const char* ext = ".mprop";
        size_t extLen = 6;
        if (len >= extLen && memcmp(buf + len - extLen, ext, extLen) == 0)
            len -= extLen;
        else
            return p;

        if (len + 2 > sizeof(buf)) return p;
        buf[len] = '/';
        buf[len + 1] = '\0';

        return p.LoadAll(buf, maxMats, maxSkels, maxAnims);
    }

    static Prefab LoadAll(const char* assetDir,
                          uint32_t maxMats  = kMaxMats,
                          uint32_t maxSkels = kMaxSkels,
                          uint32_t maxAnims = kMaxAnims)
    {
        Prefab p;
        if (!assetDir || !*assetDir) return p;

        size_t len = strlen(assetDir);
        while (len > 0 && (assetDir[len-1] == '/' || assetDir[len-1] == '\\'))
            len--;
        if (len == 0) return p;

        const char* stem = assetDir + len - 1;
        while (stem > assetDir && stem[-1] != '/' && stem[-1] != '\\')
            stem--;
        size_t stemLen = (assetDir + len) - stem;

        char buf[512];
        size_t parentLen = stem - assetDir;

        if (parentLen + stemLen + 6 >= sizeof(buf)) return p;
        memcpy(buf, assetDir, parentLen);
        memcpy(buf + parentLen, stem, stemLen);
        memcpy(buf + parentLen + stemLen, ".mprop", 6);
        buf[parentLen + stemLen + 6] = '\0';

        p.prop_ = Prop::Load(buf);
        if (!p.prop_.IsValid()) return p;

        if (len + 1 + stemLen + 6 >= sizeof(buf)) return p;
        memcpy(buf, assetDir, len);
        buf[len] = '/';
        memcpy(buf + len + 1, stem, stemLen);
        memcpy(buf + len + 1 + stemLen, ".mmesh", 6);
        buf[len + 1 + stemLen + 6] = '\0';

        p.mesh_ = Mesh::Load(buf);
        if (!p.mesh_.IsValid()) return p;

        uint32_t maxMatIdx = 0;
        uint32_t ec = p.prop_.EntryCount();
        for (uint32_t i = 0; i < ec; i++) {
            auto e = p.prop_.EntryAt(i);
            if (e.material_index >= 0 && (uint32_t)e.material_index > maxMatIdx)
                maxMatIdx = (uint32_t)e.material_index;
        }
        uint32_t matCount = maxMatIdx + 1;
        if (matCount > maxMats) return p;

        size_t baseLen = len + 1;
        if (baseLen >= sizeof(buf)) return p;
        memcpy(buf, assetDir, len);
        buf[len] = '/';
        for (uint32_t i = 0; i < matCount; i++) {
            int n = snprintf(buf + baseLen, sizeof(buf) - baseLen, "%u.mmat", i);
            if (n < 0 || (size_t)(baseLen + n) >= sizeof(buf)) return p;
            buf[baseLen + n] = '\0';
            p.materials_[i] = Material::Load(buf);
        }
        p.matCnt_ = matCount;

        uint32_t sc = p.prop_.SkeletonCount();
        if (sc > maxSkels) return p;
        for (uint32_t i = 0; i < sc; i++) {
            int n = snprintf(buf + baseLen, sizeof(buf) - baseLen, "%u.mskel", i);
            if (n < 0 || (size_t)(baseLen + n) >= sizeof(buf)) return p;
            buf[baseLen + n] = '\0';
            p.skeletons_[i] = Skeleton::Load(buf);
        }
        p.skelCnt_ = sc;

        uint32_t ac = p.prop_.AnimationCount();
        if (ac > maxAnims) return p;
        for (uint32_t i = 0; i < ac; i++) {
            int n = snprintf(buf + baseLen, sizeof(buf) - baseLen, "%u.manim", i);
            if (n < 0 || (size_t)(baseLen + n) >= sizeof(buf)) return p;
            buf[baseLen + n] = '\0';
            p.animations_[i] = Animation::Load(buf);
        }
        p.animCnt_ = ac;

        p.valid_ = true;
        return p;
    }

    ~Prefab() = default;
    Prefab() = default;
    Prefab(Prefab&&) = default;
    Prefab& operator=(Prefab&&) = default;
    Prefab(const Prefab&) = delete;
    Prefab& operator=(const Prefab&) = delete;

    bool IsValid() const { return valid_; }

    uint32_t EntryCount()     const { return prop_.EntryCount(); }
    MintPropEntry EntryAt(uint32_t i) const { return prop_.EntryAt(i); }

    const Mesh&      GetMesh()      const { return mesh_; }
    const Material*  GetMaterial(uint32_t i) const {
        return i < matCnt_ && materials_[i].IsValid() ? &materials_[i] : nullptr;
    }
    const Skeleton*  GetSkeleton(uint32_t i) const {
        return i < skelCnt_ && skeletons_[i].IsValid() ? &skeletons_[i] : nullptr;
    }
    const Animation* GetAnimation(uint32_t i) const {
        return i < animCnt_ && animations_[i].IsValid() ? &animations_[i] : nullptr;
    }

    uint32_t MaterialCount()  const { return matCnt_; }
    uint32_t SkeletonCount()  const { return skelCnt_; }
    uint32_t AnimationCount() const { return animCnt_; }

    auto begin() const { return prop_.begin(); }
    auto end()   const { return prop_.end(); }
};

inline constexpr uint32_t AttrPosition  = MINTLOAD_ATTR_POSITION;
inline constexpr uint32_t AttrNormal    = MINTLOAD_ATTR_NORMAL;
inline constexpr uint32_t AttrTexcoord0 = MINTLOAD_ATTR_TEXCOORD0;
inline constexpr uint32_t AttrColor0    = MINTLOAD_ATTR_COLOR0;
inline constexpr uint32_t AttrJoints0   = MINTLOAD_ATTR_JOINTS0;
inline constexpr uint32_t AttrWeights0  = MINTLOAD_ATTR_WEIGHTS0;
inline constexpr uint32_t AttrTangent   = MINTLOAD_ATTR_TANGENT;

inline constexpr uint32_t AlphaOpaque = MINTLOAD_ALPHA_OPAQUE;
inline constexpr uint32_t AlphaMask   = MINTLOAD_ALPHA_MASK;
inline constexpr uint32_t AlphaBlend  = MINTLOAD_ALPHA_BLEND;

inline constexpr uint32_t TexBaseColor           = MINTLOAD_TEX_BASECOLOR;
inline constexpr uint32_t TexNormal              = MINTLOAD_TEX_NORMAL;
inline constexpr uint32_t TexMetallicRoughness   = MINTLOAD_TEX_METALLIC_ROUGHNESS;
inline constexpr uint32_t TexEmissive            = MINTLOAD_TEX_EMISSIVE;
inline constexpr uint32_t TexOcclusion           = MINTLOAD_TEX_OCCLUSION;

inline constexpr uint32_t AnimTranslation = MINTLOAD_ANIM_TRANSLATION;
inline constexpr uint32_t AnimRotation    = MINTLOAD_ANIM_ROTATION;
inline constexpr uint32_t AnimScale       = MINTLOAD_ANIM_SCALE;
inline constexpr uint32_t AnimWeights     = MINTLOAD_ANIM_WEIGHTS;

inline constexpr uint32_t AnimLinear = MINTLOAD_ANIM_LINEAR;
inline constexpr uint32_t AnimStep   = MINTLOAD_ANIM_STEP;
inline constexpr uint32_t AnimCubic  = MINTLOAD_ANIM_CUBIC;

}

#endif
