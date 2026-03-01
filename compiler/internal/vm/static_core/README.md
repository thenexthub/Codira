# ArkVM Development guide

ArkVM and ArkTS and related projects are hosted at [Gitee](https://gitee.com) and [GitCode](https://gitcode.com).

## Table of content

Fast introduce:

1. [Quick setup](#quick-setup)
1. [Launch "Hello world!" on ArkVM](#launch-hello-world-on-arkvm)

Detailed info:

1. [How to use es2panda, ark, ark_aot, verifier](#how-to-use-es2panda-ark-ark_aot-verifier)
1. [Build modes](#build-modes)
1. [Toolchains](#toolchains)
1. [Enable CPU features](#enable-cpu-features)
1. [Plugins system](#plugins-system)
1. [Building with GN (still not all targets are supported)](#building-with-gn)
1. [Building with LLVM Backend](#building-with-llvm-backend)
1. [Launch cross-compiled ARM64/ARM32 tests with QEMU](#launch-cross-compiled-arm64arm32-tests-with-qemu)
1. [Install third party](#install-third-party)
1. [Testing your changes](#testing-your-changes)

## Quick setup

### Download project

```bash
mkdir arkcompiler
cd arkcompiler

BASELINE_BRANCH="OpenHarmony_feature_20241108"

git clone --branch ${BASELINE_BRANCH} https://gitee.com/openharmony/arkcompiler_runtime_core.git runtime_core

git clone --branch ${BASELINE_BRANCH} https://gitee.com/openharmony/arkcompiler_ets_frontend ets_frontend

# Set specific link for ets_frontend
ln -s ../../../ets_frontend/ets2panda runtime_core/static_core/tools/es2panda

# (optional) Install ecmascript plugin
git clone --branch ${BASELINE_BRANCH} https://gitcode.com/openharmony-sig/arkcompiler_ets_runtime.git runtime_core/static_core/plugins/ecmascript
```

### Install dependent programs

Ubuntu 22.04 is actual base system for development.

```bash
cd runtime_core/static_core/
sudo ./scripts/install-deps-ubuntu -i=dev -i=test -i=arm-all -i=llvm-prebuilts
# It is minimal pack of tools for start develop on Linux x86-64, you can see more options by pass "--help" to script
```

### Build project

Let's configure a common build in `Debug` mode using `Clang-14` (more information about the build configuration in [Build modes](#build-modes) and [Toolchains](#toolchains)):

```bash
mkdir build
cd build

cmake <PATH_TO_ARKCOMPILER>/runtime_core/static_core -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/host_clang_14.cmake
ninja ark ark_aot es2panda
```

**Congratulation, project and primary programs are built!**

### Launch "Hello world!" on ArkVM

```bash
echo 'console.log("Hello world!");' > hello.ets

BUILD=<PATH_TO_BUILD>
# Generate .abc files from source code
${BUILD}/bin/es2panda --extension=ets --output hello.abc hello.ets

# Launch Virtual Machine
${BUILD}/bin/ark --load-runtimes=ets --boot-panda-files=${BUILD}/plugins/ets/etsstdlib.abc hello.abc hello.ETSGLOBAL::main
```

**Congratulation, you launch the ArkTS code on ArkVM!**

## Detailed description

## How to use es2panda, ark, ark_aot, verifier

The most frequent used options for each program are described here.

### es2panda

`es2panda` is used to compile source code to .abc file.

| Option name | Type of option | Default value | Description |
| ---         | ---            | ---           | ---         |
| --extension | string         | "ets" | Parse the input as the given extension |
|--opt-level| number | 2 | Compiler optimization level |
|--output| string | filename with extension .abc | Path to output file |

Compile `ets` file:

```bash
./bin/es2panda --exttension=ets <file>.ets
```

You can see more options by pass `--help`.

---

### ark

`ark` is virtual machine, is used to run .abc files.

| Option name | Type of option | Default value | Description |
| ---         | ---            | ---           | ---         |
|--load-runtimes|string|"core"|Load specified class and intrinsic spaces and define runtime type|
|--boot-panda-files|string[]|[]|Boot panda files (.abc) separated by colon|
|--interpreter-type|string|irtoc|Interpreter implementation type|
|--compiler-enable-jit|bool|true|Enables/disables JIT compiler|
|--no-async-jit|bool|false|Perform compilation in the main thread or in parallel worker|
|--compiler-hotness-threshold|uint32_t|3000|Threshold for "hotness" counter of the method after that it will be compiled|
|--aot-files|string[]|[]|List of aot files (.an) to be loaded separated by colon|
|--enable-an|bool|false|Try to load ARK .an file base on abc file location|
|--enable-an:force|bool|false|Crash if there is no .an file for location based on .abc file|

Launch `ets` abc file:

```bash
./bin/ark --load-runtimes=ets --boot-panda-files=plugins/ets/etsstdlib.abc <file>.abc <file>.ETSGLOBAL::main
```

You can see more options by pass `--help`.

---

### ark_aot

`ark_aot` tool aims to compile input panda files into the single AOT file, that can be consumed by `ark`.

| Option name | Type of option | Default value | Description |
| ---         | ---            | ---           | ---         |
|--load-runtimes|string|"core"|Load specified class and intrinsic spaces and define runtime type|
|--paoc-panda-files|string[]|[]|List of input panda files to compiler separated by colon|
|--boot-panda-files|string[]|[]|Boot panda files (.abc) separated by colon|
|--paoc-output|string|out.an|Path to output file|

Compile an `ets` abc file to AOT file:

```bash
./bin/ark_aot --boot-panda-files=plugins/ets/etsstdlib.abc --load-runtimes=ets --paoc-panda-files=<file>.abc --paoc-output=<file>.an
```

> *NOTE*: `boot-panda-files` should be the same as it will be during launch `ark`, otherwise will be throw error during execution about mismach class hierarchy.

You can see more options by launch `ark_aot` without options.

---

### verifier

`verifier` tool aims to verify specified Panda files.

| Option name | Type of option | Default value | Description |
| ---         | ---            | ---           | ---         |
|--load-runtimes|string|"core"|Load specified class and intrinsic spaces and define runtime type|
|--boot-panda-files|string[]|[]|Boot panda files (.abc) separated by colon|

```bash
./bin/verifier --load-runtimes=ets --boot-panda-files=plugins/ets/etsstdlib.abc <file>.abc
```

> *NOTE*: `boot-panda-files` should be the same as it will be during launch `ark` or `ark_aot`.

You can see more options by pass `--help`.

## Build modes

Recommended way to build Panda is to set `CMAKE_BUILD_TYPE` variable explicitly during configurations. Supported values are:

| Mode         | Assertions | Optimizations         | Debug info           |
| ----         | ---------- | -------------         | ----------           |
| `Debug`     | Yes        | None (CMake default)  | `-g` (CMake default) |
| `Release`    | No         | `-O3` (CMake default) | None (CMake default) |
| `FastVerify` | Yes        | `-O2`                 | `-ggdb3`             |

*Notes*:

* Other common modes (`RelWithDebInfo`, `MinSizeRel`, `DebugDetailed`) should work but they are not tested in CI.
* Unlike `RelWithDebInfo`, `FastVerify` preserves assertions (and provides more verbose debug information).
  Use this build type for running heavy test suites when you want both fast-running code and debuggability.
* `DebugDetailed` gives more debug information than `Debug`, it can be useful for debugging unit tests for example.

## Toolchains

All types of toolchains are located by path: `runtime_core/static_core/cmake/toolchain`. Main toolchains which are used in development:

* `cmake/toolchain/host_clang_14.cmake` - build by clang-14 on Linux
* `cmake/toolchain/host_gcc_11.cmake` - build by gcc-11 on Linux
* `cmake/toolchain/cross-clang-14-qemu-aarch64.cmake` - cross build  by clang-14 for arm64 with launch on Linux x86-64
* `cmake/toolchain/cross-clang-14-qemu-arm-linux-gnueabi.cmake` - cross build for arm32 (gnueabi) with launch on Linux x86-64 by clang-14
* `cmake/toolchain/cross-clang-14-x86_64-w64-mingw32-static.cmake` - build by mingw clang-14 on Windows x86-64

You can already see how the toolchain name is built and what it is used for. By analogy, so for all the others.

For more details, see the [build system](cmake/README.md).

## Enable CPU features

To specify what CPU features will be supported by the target CPU, the `PANDA_TARGET_CPU_FEATURES` variable may be set.
This list of the features will be passed into numerous custom tools during the building of the VM itself, e.g., in
[irtoc](irtoc/README.md) and influence what instructions will be selected by these tools:

```
$ cmake -DPANDA_TARGET_CPU_FEATURES=jscvt,atomics
```

## Plugins system

ArkVM has a multilanguage virtual machine, each language attach as a plugin. All plugins located as a separate folder by path `runtime_core/static_core/plugins/`. By default, extension is plugged in configure if folder is exist.

> *Example*: there is folder `runtime_core/static_core/plugins/ets/`, so language `ets` will implicitly be added to configure for building.

You can explicitly enable/disable language by cmake option `-DPANDA_WITH_<LANG>`.

> *Example*: option for `ets` will be `--PANDA_WITH_ETS=ON/OFF`.

## Building with GN

### Using bootstrap

Build `arkts_bin`, `ark_aot`, `es2panda`, `verifier_bin` and `ets_interop_js_napi` targets.Launch cross-compiled ARM64/ARM32 tests with QEMU

```bash
cd runtime_core/static_core
./scripts/build-panda-with-gn
```

### Manually

1. Getting GN binary

```bash
$ git clone https://gn.googlesource.com/gn
$ cd gn
$ python build/gen.py
$ ninja -C out
```

2. Build panda using gn (`arkts_asm`, `arkts_disasm`, `ark_aot`, `ark_aotdump`, `arkts_bin`, `es2panda`, and `verifier_bin` targets are supported)

```bash
$ cd /path/to/panda/repository
$ /path/to/gn/repository/out/gn gen out
$ ninja -C out arkts_bin
```

When standard system, use

```bash
$ cd /path/to/panda/repository
$ /path/to/gn/repository/out/gn --args=is_standard_system=true gen out
$ ninja -C out <target name>
```

By default, gn build uses LLVM Backend, so one must provide `llvm_target_dir="/path/to/llvm-15-{type}-{arch}"` if it is not in default location. To build without llvm add the following arguments:

```bash
$ /path/to/gn/repository/out/gn out is_llvmbackend=false ...
```

Option `is_llvmbackend=true` in gn is the same scenarios as `-DPANDA_LLVM_BACKEND=true` option in cmake builds

## Building with LLVM Backend

If you want to build Ark with LLVM Backend you need to special modified LLVM 15 binaries, there are 2 way to get them:

1. Download prebuilds *(preferred way)*

    The prebuilds will be installed in the folder `/opt/`:

    ```bash
    cd runtime_core/static_core/
    sudo ./scripts/install-deps-ubuntu -i=llvm-prebuilts
    ```

2. Build binaries locally

    * Clone repository from [gitee.com](https://gitee.com/openharmony/third_party_llvm-project)
    * Check [README](scripts/llvm/README.md) file about build process
    * Use build [script](scripts/llvm/build_llvm.sh)

If modified LLVM available in `/opt`, the following two options are necessary
to build Ark with LLVM Backend functions.

```cmake
cmake -DPANDA_LLVM_BACKEND=true -DLLVM_TARGET_PATH=/opt/llvm-15-{type}-{arch} ...
```

The `PANDA_LLVM_BACKEND` enables:

1. LLVM Irtoc Interpreter. Use `-DPANDA_LLVM_INTERPRETER=OFF` to disable.
2. LLVM Fastpaths compilation. Use `-DPANDA_LLVM_FASTPATH=OFF` to disable.
3. LLVM Interpreter inlining. Use `-DPANDA_LLVM_INTERPRETER_INLINING=OFF` to disable.
4. LLVM AOT compiler. Use `-DPANDA_LLVM_AOT=OFF` to disable.

`PANDA_LLVM_INTERPRETER`, `PANDA_LLVM_FASTPATH`, and `PANDA_LLVM_AOT` are `ON` if `PANDA_LLVM_BACKEND` is turned on.

It is recommended to choose `clang` compiler using toolchain files: `-DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/host_clang_14.cmake`.
By default GNU compiler `c++` is used, but some features are not available in such `gcc` builds.

### Building with LLVM Backend for cross ARM64 build

For cross-build, when e.g. `-DCMAKE_TOOLCHAIN_FILE=cmake/toolchain/cross-clang-14-qemu-aarch64.cmake` is used,
two LLVM-paths options are required when all LLVM Backend functions are enabled.

* First one is target LLVM, like `-DLLVM_TARGET_PATH=/opt/llvm-15-debug-aarch64`
  * It is required for AOT, so, alternatively, you can use `-DPANDA_LLVM_AOT=OFF`.
* Second one is host LLVM, like `-DLLVM_HOST_PATH=/opt/llvm-15-debug-x86_64`
  * It is required for Irtoc compilation, so, alternatively, you can disable appropriate Interpreter and FastPath options (see above).

## Launch cross-compiled ARM64/ARM32 tests with QEMU

Recommended QEMU version for running tests is 6.2.0 (but 5.1+ should be ok, too). By default, it is downloaded and installed during environment bootstrap. Any system-installed package is left intact. If recommended QEMU version is not accessible via $PATH it can be specified during configuration time:

```bash
# If QEMU is available as /opt/qemu-6.2.0/bin/qemu-aarch64 or download manually
$ cmake -DQEMU_PREFIX=/opt/qemu-6.2.0 ...
```

## Install third party

Panda uses third party libraries. During firs configure of `cmake` third-party will be installed automatically. To manually download libraries run:

```bash
cd runtime_core/static_core
./scripts/install-third-party
```

Useful options:

* `--force-clone` - force download libraries, usually used when one of part third-party have updated
* `--repo-name=repo` - install specific repo
* `--node` - install node

You can see more options by pass `--help` to script

## Testing your changes

For testing, the following umbrella targets that guarantee building prior to running may be used:

* `ninja tests`, for running all testing suites.
* `ninja tests_full`, for running all testing suites and various code linters.

`clang-format` and `clang-tidy` checks are integrated into build system and can be called by target of build system:

* `ninja code-style-check` - clang-format
* `ninja clang-tidy-check` - clang-tidy

> *NOTE* `ninja` is launched from the build directory
