# AbcKit

`AbcKit` 是一个用于**检查和修改**方舟字节码的工具库。

文档索引：

- [如何构建与测试](doc/how_to_build_and_test_zh.md)
- [快速上手小册](doc/mini_cookbook_zh.md)
- [实现原理说明](doc/implementation_description_zh.md)

---

## 如何下载与构建

### 安装依赖项:

请确保已安装以下组件：

- `repo`
- `Clang++-14`
- `Clang-format-14`
- `Clang-tidy-14`
- `Doxygen`
- `Graphviz`
- `Git LFS`

### 下载代码:

```sh
repo init -u https://gitee.com/ark-standalone-build/manifest.git -b master
repo sync -c -j8
repo forall -c 'git lfs pull'
./prebuilts_download.sh
```

### 构建 AbcKit:

```sh
# Linux 平台构建
./ark.py x64.release abckit_packages --gn-args="is_standard_system=true abckit_enable=true enable_cmc_gc=false"
# Windows 平台构建
./ark.py mingw_x86_64.release abckit_packages --gn-args="is_standard_system=true abckit_enable=true enable_cmc_gc=false"
# Mac 平台构建
./ark.py mac_arm64.release abckit_packages --gn-args="is_standard_system=true abckit_enable=true enable_cmc_gc=false"
```

### 生成 Doxygen 文档:

```sh
cd out/x64.release
ninja abckit_documentation
```

生成的文档将保存在 `out/x64.release/gen/arkcompiler/runtime_core/libabckit/doxygen`
