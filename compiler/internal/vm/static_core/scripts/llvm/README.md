# Building modified LLVM library from source

## Sources

Sources are available at [gitee](https://gitee.com/openharmony/third_party_llvm-project)

## Build script

The [script](build_llvm.sh) does not take command line arguments, instead build parameters should be set using environment variables.

Some variables are mandatory, others are optional.

### Environment variables list

```bash
### Required variables
BUILD_DIR=<directory where building process takes place>
LLVM_SOURCES=<directory with sources>/llvm
VERSION=<version string, which is included in build/install directory names>(default: "main")
PACKAGE_VERSION=<kit version>(default: $VERSION) # must match REQUIRED_LLVM_VERSION in libllvmbackend/CMakeLists.txt

### Select targets to build, at least one must be set to "true"
BUILD_X86_DEBUG=<debug version for x86_64 architecure>(default: false)
BUILD_X86_RELEASE=<release version for x86_64 architecure>(default: false)
BUILD_AARCH64_DEBUG=<debug version for arm64 architecure>(default: false)
BUILD_AARCH64_RELEASE=<release version for arm64 architecure>(default: false)
BUILD_OHOS_RELEASE=<release version for OHOS platform (OHOS SDK build)>(default: false)
BUILD_OHOS_RELEASE_GN=<release version for OHOS GN platform (GN build)>(default: false)

### Optional variables
INSTALL_DIR=<directory for installation, empty means "do not install"> (default: "")

DO_STRIPPING=<when install, strip libraries before installation>(default: true)
DO_TAR=<when install, also pack llvm-$VERSION-*-*.tar.xz archives for corresponding folders>(default: true)

OHOS_SDK=<path to native OHOS SDK>, required for any OHOS build.

OHOS_PREBUILTS=<path to OHOS pre-build directory>, required for OHOS GN build.

### Build tools
CC=<C compiler> (default: "/usr/bin/clang-14")
CXX=<C++ compiler> (default: "/usr/bin/clang++-14")
STRIP=<strip utility> (default: "/usr/bin/llvm-strip-14")
OPTIMIZE_DEBUG=<compile debug version with -O2>(default: true)
```

**Note! `PACKAGE_VERSION` must be properly set during build to match string
constant `REQUIRED_LLVM_VERSION` from `libllvmbackend/CMakeLists.txt`.**

## Example for local user build

```bash
cd /home/user/src
git clone https://gitee.com/openharmony/third_party_llvm-project.git

INSTALL_DIR="/home/user/inst" \
BUILD_DIR="/home/user/build" \
LLVM_SOURCES="/home/user/src/llvm-for-ark/llvm" \
VERSION="15.0.4-ark99-beta9" \
PACKAGE_VERSION="15.0.4-ark99" \
OPTIMIZE_DEBUG=false \
BUILD_X86_DEBUG=true \
BUILD_AARCH64_DEBUG=true \
bash -x ./build_llvm.sh
```

In this example, only `x86_64` and `arm64` debug versions are built. Then, they can be specified for Ark build, like:
* host build: `-DLLVM_TARGET_PATH=/home/user/build/llvm-15.0.4-ark99-beta9-debug-x86_64`
* cross-arm64 build:
   * `-DLLVM_TARGET_PATH=/home/user/build/llvm-15.0.4-ark99-beta9-debug-aarch64`
   * `-DLLVM_HOST_PATH=/home/user/build/llvm-15.0.4-ark99-beta9-debug-x86_64`

## Example with packaging all necessary versions

```bash
INSTALL_DIR="/mnt/scratch/install" \
BUILD_DIR="/mnt/scratch/build" \
LLVM_SOURCES="/mnt/scratch/src/llvm-for-ark/llvm" \
VERSION="15.0.4-ark99-beta9" \
PACKAGE_VERSION="15.0.4-ark99" \
OHOS_SDK="/opt/ohos-sdk/native" \
OHOS_PREBUILTS="/home/user/ohos/prebuilts" \
BUILD_X86_DEBUG=true \
BUILD_X86_RELEASE=true \
BUILD_AARCH64_DEBUG=true \
BUILD_AARCH64_RELEASE=true \
BUILD_OHOS_RELEASE=true \
BUILD_OHOS_RELEASE_GN=true \
bash -x ./build_llvm.sh
```
