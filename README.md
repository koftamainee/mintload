# 📦 mintload — Runtime loader for mint formats

[![License: MIT](https://img.shields.io/badge/License-MIT-orange.svg)](LICENSE)

**mintload** is a `c11`/`c++17` **blazingly fast** and **zero-allocation** runtime loader for assets produced by [**mint**](https://github.com/koftamainee/mint).  
Written for usage in [**Mantle engine**](https://github.com/koftamainee/mantle). <br>
Reads `.mmesh`, `.mmat`, `.mprop`, `.mskel`, `.manim` directly from disk via `mmap`/`MapViewOfFile` — no heap, no mallocs, no JSON, no dependencies. **Cross-platform** (Linux + Windows).

---

## 🚀 Quick start

### Install

```bash
git clone https://github.com/koftamainee/mintload.git
cd mintload
cmake -B build -DMINTLOAD_ENABLE_CPP=ON
cmake --build build
```
---

## 📦 Minimal examples

### C

```c
#include <mintload/mintload.h>

#include <stdio.h>

int main(int argc, char** argv) {
    MintProp prop;
    mintload_MpropLoad(argv[1], &prop);

    printf("entries: %u\n", prop.entry_count);
    for (uint32_t i = 0; i < prop.entry_count; i++) {
        MintPropEntry e = mintload_MpropEntry(&prop, i);
        printf("  \"%.*s\" mesh=%d mat=%d\n",
               (int)e.name_len, e.name,
               e.sub_mesh_index, e.material_index);
    }

    mintload_MpropUnload(&prop);
}
```

### C++

```cpp
#include <mintload/mintload.hpp>

#include <iostream>
#include <string_view>

int main(int argc, char** argv) {
    auto Prefab = Mintload::Prefab::Load(argv[1]);
    if (!Prefab.IsValid()) return 1;

    std::cout << "entries: " << Prefab.EntryCount() << "\n";
    for (const auto& Entry : Prefab) {
        std::cout << "  \"" << std::string_view(Entry.name, Entry.name_len)
                  << "\" mesh=" << Entry.sub_mesh_index
                  << " mat=" << Entry.material_index << "\n";
    }
}
```

Both print the same output — the C++ version is a thin zero-overhead wrapper.

Detailed examples live in [`examples/`](examples/).

---

## 🧱 API overview

| Type        | Loader                          | Accessors |
|-------------|---------------------------------|-----------|
| `.mmesh`    | `mintload_MmeshLoad` / `Mesh::Load` | vertices, indices, sub-meshes, LODs |
| `.mmat`     | `mintload_MmatLoad` / `Material::Load` | PBR params, texture slots |
| `.mprop`    | `mintload_MpropLoad` / `Prop::Load` | entries, skeleton/animation counts |
| `.mskel`    | `mintload_MskelLoad` / `Skeleton::Load` | bones, hierarchy, inverse bind matrices |
| `.manim`    | `mintload_ManimLoad` / `Animation::Load` | channels, keyframes |
| Prefab      | `Prefab::Load(propPath)` | loads everything — mesh + materials + skeletons + animations in one call |

All C functions return `MintloadResult` (zero = success).  
All C++ types are move-only, no exceptions, no heap allocation.

---

## 🛠️ CMake options

| Option | Default | Description |
|--------|---------|-------------|
| `MINTLOAD_BUILD_EXAMPLES` | `OFF` | Build `mtl-c` (and `mtl-cpp` if C++ enabled) |
| `MINTLOAD_ENABLE_CPP`     | `OFF` | Include `mintload.hpp` in install, allow C++ examples |

---

## 🤝 Contributing

PRs welcome!

## 📄 License

MIT — do whatever you want, just keep the copyright notice.
