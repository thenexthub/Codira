#!/bin/bash
# Copyright (c) 2023-2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -euo pipefail

# Required variables
BUILD_DIR=${BUILD_DIR:-""}
LLVM_SOURCES=${LLVM_SOURCES:-""}
LLVM_EXTRA_CMAKE_FLAGS=${LLVM_EXTRA_CMAKE_FLAGS:-""}
# Optional variables
INSTALL_DIR=${INSTALL_DIR:-""}         # empty -- do not install
VERSION=${VERSION:-"main"}             # Specifies build and install directory names:
PACKAGE_VERSION=${PACKAGE_VERSION:-${VERSION}}
BUILD_SUFFIX=${BUILD_SUFFIX:-""}       # llvm-<VERSION>-{debug,release}-{aarch64,x86}<BUILD_SUFFIX>
OPTIMIZE_DEBUG=${OPTIMIZE_DEBUG:-true} # Compile debug versions with -O2
DO_STRIPPING=${DO_STRIPPING:-true}     # checked only if install
DO_TARS=${DO_TARS:-true}               # checked only if install
# Select a target to build
BUILD_X86_DEBUG=${BUILD_X86_DEBUG:-false}
BUILD_X86_RELEASE=${BUILD_X86_RELEASE:-false}
BUILD_AARCH64_DEBUG=${BUILD_AARCH64_DEBUG:-false}
BUILD_AARCH64_RELEASE=${BUILD_AARCH64_RELEASE:-false}
BUILD_OHOS_RELEASE=${BUILD_OHOS_RELEASE:-false}
BUILD_OHOS_RELEASE_GN=${BUILD_OHOS_RELEASE_GN:-false}
LLVM_COMPONENTS="cmake-exports;llvm-headers;LLVM"
LLVM_COMPONENTS_PLUS_LINK="${LLVM_COMPONENTS};llvm-link"

# Release version with asserts and debug methods included
FASTVERIFY_RELEASE=${FASTVERIFY_RELEASE:-false}
ENABLE_LLVM_DUMP_FOR_RELEASE=OFF
ENABLE_LLVM_ASSERTIONS_FOR_RELEASE=OFF
RELEASE_POSTFIX=""

if [[ "x${FASTVERIFY_RELEASE}" == "xtrue" ]]; then
    if [[ "x${BUILD_OHOS_RELEASE}" == "xtrue" || "x${BUILD_AARCH64_RELEASE}" == "xtrue" ]]; then
        ENABLE_LLVM_DUMP_FOR_RELEASE=ON
        ENABLE_LLVM_ASSERTIONS_FOR_RELEASE=ON
        RELEASE_POSTFIX="-fastverify"
    else
        echo "ERROR: FASTVERIFY_RELEASE should be used only with" \
             "BUILD_OHOS_RELEASE or BUILD_AARCH64_RELEASE"
        exit 1
    fi
fi

# OHOS SDK and compiler
OHOS_SDK=${OHOS_SDK:-""}
OHOS_PREBUILTS=${OHOS_PREBUILTS:-""}

# Build tools
CC=${CC:-"/usr/bin/clang-14"}
CXX=${CXX:-"/usr/bin/clang++-14"}
STRIP=${STRIP:-"/usr/bin/llvm-strip-14"}
LLVM_TABLEGEN=${LLVM_TABLEGEN:-""}
# Etc.
NPROC=$(nproc)

if [[ -z "${BUILD_DIR}" ]]; then
    echo "Please, specify build directory with BUILD_DIR variable"
    exit 1
fi

if [[ -z "${LLVM_SOURCES}" ]]; then
    echo "Please, specify llvm sources directory with LLVM_SOURCES variable"
    exit 1
fi

if [[ "x${BUILD_AARCH64_DEBUG}" == "xtrue" || "x${BUILD_AARCH64_RELEASE}" == "xtrue" || "x${BUILD_OHOS_RELEASE_GN}" == "xtrue"  || "x${BUILD_OHOS_RELEASE}" == "xtrue" ]]; then
    if [[ -n "${LLVM_TABLEGEN}" && ! -f "${LLVM_TABLEGEN}" ]]; then
        echo "Provided LLVM_TABLEGEN '${LLVM_TABLEGEN}' does not exist"
        exit 1
    fi

    if [[ -z "${LLVM_TABLEGEN}" ]]; then
        qemu=$(which update-binfmts &> /dev/null && \
                (update-binfmts --display || true) | grep -q "qemu-aarch64" && \
                echo "true" || echo "false")

        if [[ "x${qemu}" == "xfalse" && "x${BUILD_X86_DEBUG}" == "xfalse" && \
                "x${BUILD_X86_RELEASE}" == "xfalse" ]]; then
            echo "Cannot launch AArch64 builds without host llvm-tblgen."
            echo "Please do something from listed below:"
            echo "1) set LLVM_TABLEGEN variable to installed llvm-tblgen"
            echo "2) install qemu-binfmt to transparently run AArch64 binaries"
            echo "3) request one of X86 build using BUILD_X86_DEBUG or BUILD_X86_RELEASE."
            echo "   llvm-tblgen will be used from build directory automatically."
            exit 1
        fi
    fi
fi

if [[ -z "${OHOS_SDK}" && "x${BUILD_OHOS_RELEASE}" == "xtrue" ]]; then
    echo "OHOS_SDK variable must be provided for OHOS build"
    exit 1
fi

if [[ -z "${OHOS_SDK}" && "x${BUILD_OHOS_RELEASE_GN}" == "xtrue" ]]; then
    echo "OHOS_SDK variable must be provided for OHOS GN build"
    exit 1
fi

if [[ -z "${OHOS_PREBUILTS}" && "x${BUILD_OHOS_RELEASE_GN}" == "xtrue" ]]; then
    echo "OHOS_PREBUILTS variable must be provided for OHOS GN build"
    exit 1
fi

DEBUG_OPT_FLAG=""
if [[ "x${OPTIMIZE_DEBUG}" == "xtrue" ]]; then
    DEBUG_OPT_FLAG="-O2"
fi

function install() {
    local install_dir="$1"
    local strip="$2"
    local archive="$3"
    if [[ "x${strip}" == "xtrue" ]]; then
        ninja install-distribution-stripped
    else
        ninja install-distribution
    fi
    if [[ "x${archive}" == "xtrue" ]]; then
        tar cJf "${install_dir}.tar.xz" -C "${install_dir}" .
        du -h "${install_dir}.tar.xz"
    fi
}

AUTO_LLVM_TABLEGEN=""
if [[ "x${BUILD_X86_DEBUG}" == "xtrue" ]]; then
    TARGET="x86_64${BUILD_SUFFIX}"
    INSTALL_DIR_NAME="llvm-${VERSION}-debug-${TARGET}"
    INSTALL_PREFIX="${INSTALL_DIR}/${INSTALL_DIR_NAME}"
    BUILD_PREFIX="${BUILD_DIR}/${INSTALL_DIR_NAME}"
    mkdir -p "${BUILD_PREFIX}"
    cd "${BUILD_PREFIX}"
    cmake -G Ninja \
        -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
        -DPACKAGE_VERSION="${PACKAGE_VERSION}" \
        -DCMAKE_BUILD_TYPE=Debug \
        \
        -DCMAKE_C_FLAGS="${DEBUG_OPT_FLAG}" -DCMAKE_CXX_FLAGS="${DEBUG_OPT_FLAG}" \
        \
        -DCMAKE_C_COMPILER="${CC}" \
        -DCMAKE_CXX_COMPILER="${CXX}" \
        -DCMAKE_STRIP="${STRIP}" \
        \
        -DLLVM_ENABLE_FFI=OFF \
        -DLLVM_ENABLE_TERMINFO=OFF \
        \
        -DLLVM_INCLUDE_BENCHMARKS=OFF \
        -DLLVM_INCLUDE_EXAMPLES=OFF \
        -DLLVM_INCLUDE_TESTS=OFF \
        \
        -DLLVM_BUILD_TOOLS=ON \
        -DLLVM_DISTRIBUTION_COMPONENTS="${LLVM_COMPONENTS_PLUS_LINK}" \
        -DLLVM_BUILD_LLVM_DYLIB=ON \
        -DLLVM_ENABLE_ZLIB=OFF \
        -DLLVM_TARGETS_TO_BUILD="X86;AArch64" \
        -DLLVM_PARALLEL_COMPILE_JOBS="${NPROC}" \
        -DLLVM_PARALLEL_LINK_JOBS=1 \
        "${LLVM_EXTRA_CMAKE_FLAGS}" \
        "${LLVM_SOURCES}"

    ninja distribution
    if [[ -n "${INSTALL_DIR}" ]]; then
        install "${INSTALL_PREFIX}" "${DO_STRIPPING}" "${DO_TARS}"
    fi

    AUTO_LLVM_TABLEGEN="${BUILD_PREFIX}/bin/llvm-tblgen"
    if [[ ! -f "${AUTO_LLVM_TABLEGEN}" ]]; then
        echo "${AUTO_LLVM_TABLEGEN} must be present. Something went wrong..."
    fi
fi

if [[ "x${BUILD_X86_RELEASE}" == "xtrue" ]]; then
    TARGET="x86_64${BUILD_SUFFIX}"
    INSTALL_DIR_NAME="llvm-${VERSION}-release-${TARGET}"
    INSTALL_PREFIX="${INSTALL_DIR}/${INSTALL_DIR_NAME}"
    BUILD_PREFIX="${BUILD_DIR}/${INSTALL_DIR_NAME}"
    mkdir -p "${BUILD_PREFIX}"
    cd "${BUILD_PREFIX}"
    cmake -G Ninja \
        -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
        -DPACKAGE_VERSION="${PACKAGE_VERSION}" \
        -DCMAKE_BUILD_TYPE=Release \
        \
        -DCMAKE_C_COMPILER="${CC}" \
        -DCMAKE_CXX_COMPILER="${CXX}" \
        -DCMAKE_STRIP="${STRIP}" \
        \
        -DLLVM_ENABLE_FFI=OFF \
        -DLLVM_ENABLE_TERMINFO=OFF \
        \
        -DLLVM_INCLUDE_BENCHMARKS=OFF \
        -DLLVM_INCLUDE_EXAMPLES=OFF \
        -DLLVM_INCLUDE_TESTS=OFF \
        \
        -DLLVM_BUILD_TOOLS=ON \
        -DLLVM_DISTRIBUTION_COMPONENTS="${LLVM_COMPONENTS_PLUS_LINK}" \
        -DLLVM_BUILD_LLVM_DYLIB=ON \
        -DLLVM_ENABLE_ZLIB=OFF \
        -DLLVM_TARGETS_TO_BUILD="X86;AArch64" \
        -DLLVM_PARALLEL_COMPILE_JOBS="${NPROC}" \
        -DLLVM_PARALLEL_LINK_JOBS=1 \
        "${LLVM_EXTRA_CMAKE_FLAGS}" \
        "${LLVM_SOURCES}"

    ninja distribution
    if [[ -n "${INSTALL_DIR}" ]]; then
        install "${INSTALL_PREFIX}" "${DO_STRIPPING}" "${DO_TARS}"
    fi

    AUTO_LLVM_TABLEGEN="${BUILD_PREFIX}/bin/llvm-tblgen"
    if [[ ! -f "${AUTO_LLVM_TABLEGEN}" ]]; then
        echo "${AUTO_LLVM_TABLEGEN} must be present. Something went wrong..."
    fi
fi

LLVM_TABLEGEN=${LLVM_TABLEGEN:-${AUTO_LLVM_TABLEGEN}}
LLVM_TABLEGEN=${LLVM_TABLEGEN:+-DLLVM_TABLEGEN=${LLVM_TABLEGEN}}

if [[ "x${BUILD_AARCH64_DEBUG}" == "xtrue" ]]; then
    TARGET="aarch64${BUILD_SUFFIX}"
    INSTALL_DIR_NAME="llvm-${VERSION}-debug-${TARGET}"
    INSTALL_PREFIX="${INSTALL_DIR}/${INSTALL_DIR_NAME}"
    BUILD_PREFIX="${BUILD_DIR}/${INSTALL_DIR_NAME}"
    mkdir -p "${BUILD_PREFIX}"
    cd "${BUILD_PREFIX}"
    cmake -G Ninja \
        -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
        -DPACKAGE_VERSION="${PACKAGE_VERSION}" \
        -DCMAKE_BUILD_TYPE=Debug \
        \
        -DCMAKE_CROSSCOMPILING=ON \
        -DLLVM_TARGET_ARCH=AArch64 \
        -DLLVM_DEFAULT_TARGET_TRIPLE=aarch64-linux-gnu \
        -DCMAKE_C_FLAGS="--target=aarch64-linux-gnu -I/usr/aarch64-linux-gnu/include/ ${DEBUG_OPT_FLAG}" \
        -DCMAKE_CXX_FLAGS="--target=aarch64-linux-gnu -I/usr/aarch64-linux-gnu/include/c++/8/aarch64-linux-gnu -I/usr/aarch64-linux-gnu/include/ ${DEBUG_OPT_FLAG}" \
        \
        -DCMAKE_C_COMPILER="${CC}" \
        -DCMAKE_CXX_COMPILER="${CXX}" \
        -DCMAKE_STRIP="${STRIP}" \
        \
        -DLLVM_ENABLE_FFI=OFF \
        -DLLVM_ENABLE_TERMINFO=OFF \
        \
        -DLLVM_INCLUDE_BENCHMARKS=OFF \
        -DLLVM_INCLUDE_EXAMPLES=OFF \
        -DLLVM_INCLUDE_TESTS=OFF \
        \
        -DLLVM_BUILD_TOOLS=ON \
        -DLLVM_DISTRIBUTION_COMPONENTS="${LLVM_COMPONENTS_PLUS_LINK}" \
        -DLLVM_BUILD_LLVM_DYLIB=ON \
        -DLLVM_ENABLE_ZLIB=OFF \
        -DLLVM_TARGETS_TO_BUILD=AArch64 \
        -DLLVM_PARALLEL_COMPILE_JOBS="${NPROC}" \
        -DLLVM_PARALLEL_LINK_JOBS=1 \
        "${LLVM_TABLEGEN}" \
        "${LLVM_EXTRA_CMAKE_FLAGS}" \
        "${LLVM_SOURCES}"

    ninja distribution
    if [[ -n "${INSTALL_DIR}" ]]; then
        install "${INSTALL_PREFIX}" "${DO_STRIPPING}" "${DO_TARS}"
    fi
fi

if [[ "x${BUILD_AARCH64_RELEASE}" == "xtrue" ]]; then
    TARGET="aarch64${BUILD_SUFFIX}"
    INSTALL_DIR_NAME="llvm-${VERSION}-release-${TARGET}${RELEASE_POSTFIX}"
    INSTALL_PREFIX="${INSTALL_DIR}/${INSTALL_DIR_NAME}"
    BUILD_PREFIX="${BUILD_DIR}/${INSTALL_DIR_NAME}"
    mkdir -p "${BUILD_PREFIX}"
    cd "${BUILD_PREFIX}"
    cmake -G Ninja \
        -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
        -DPACKAGE_VERSION="${PACKAGE_VERSION}" \
        -DCMAKE_BUILD_TYPE=Release \
        \
        -DCMAKE_CROSSCOMPILING=ON \
        -DLLVM_TARGET_ARCH=AArch64 \
        -DLLVM_DEFAULT_TARGET_TRIPLE=aarch64-linux-gnu \
        -DCMAKE_C_FLAGS="--target=aarch64-linux-gnu -I/usr/aarch64-linux-gnu/include/" \
        -DCMAKE_CXX_FLAGS="--target=aarch64-linux-gnu -I/usr/aarch64-linux-gnu/include/c++/8/aarch64-linux-gnu -I/usr/aarch64-linux-gnu/include/" \
        \
        -DCMAKE_C_COMPILER="${CC}" \
        -DCMAKE_CXX_COMPILER="${CXX}" \
        -DCMAKE_STRIP="${STRIP}" \
        \
        -DLLVM_ENABLE_DUMP=${ENABLE_LLVM_DUMP_FOR_RELEASE} \
        -DLLVM_ENABLE_ASSERTIONS=${ENABLE_LLVM_ASSERTIONS_FOR_RELEASE} \
        -DLLVM_ENABLE_FFI=OFF \
        -DLLVM_ENABLE_TERMINFO=OFF \
        \
        -DLLVM_INCLUDE_BENCHMARKS=OFF \
        -DLLVM_INCLUDE_EXAMPLES=OFF \
        -DLLVM_INCLUDE_TESTS=OFF \
        \
        -DLLVM_BUILD_TOOLS=ON \
        -DLLVM_DISTRIBUTION_COMPONENTS="${LLVM_COMPONENTS_PLUS_LINK}" \
        -DLLVM_BUILD_LLVM_DYLIB=ON \
        -DLLVM_ENABLE_ZLIB=OFF \
        -DLLVM_TARGETS_TO_BUILD=AArch64 \
        -DLLVM_PARALLEL_COMPILE_JOBS="${NPROC}" \
        -DLLVM_PARALLEL_LINK_JOBS=1 \
        "${LLVM_TABLEGEN}" \
        "${LLVM_EXTRA_CMAKE_FLAGS}" \
        "${LLVM_SOURCES}"

    ninja distribution
    if [[ -n "${INSTALL_DIR}" ]]; then
        install "${INSTALL_PREFIX}" "${DO_STRIPPING}" "${DO_TARS}"
    fi
fi

if [[ "x${BUILD_OHOS_RELEASE}" == "xtrue" ]]; then
    TARGET="ohos${BUILD_SUFFIX}"
    INSTALL_DIR_NAME="llvm-${VERSION}-release-${TARGET}${RELEASE_POSTFIX}"
    INSTALL_PREFIX="${INSTALL_DIR}/${INSTALL_DIR_NAME}"
    BUILD_PREFIX="${BUILD_DIR}/${INSTALL_DIR_NAME}"
    mkdir -p "${BUILD_PREFIX}"
    cd "${BUILD_PREFIX}"
    "${OHOS_SDK}/build-tools/cmake/bin/cmake" -G Ninja \
        -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
        -DPACKAGE_VERSION="${PACKAGE_VERSION}" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE="${OHOS_SDK}/build/cmake/ohos.toolchain.cmake" \
        \
        -DOHOS_ALLOW_UNDEFINED_SYMBOLS=ON \
        \
        -DLLVM_DEFAULT_TARGET_TRIPLE=aarch64-linux-ohos \
        -DLLVM_TARGETS_TO_BUILD=AArch64 \
        -DLLVM_ENABLE_DUMP=${ENABLE_LLVM_DUMP_FOR_RELEASE} \
        -DLLVM_ENABLE_ASSERTIONS=${ENABLE_LLVM_ASSERTIONS_FOR_RELEASE} \
        \
        -DLLVM_ENABLE_FFI=OFF \
        -DLLVM_ENABLE_TERMINFO=OFF \
        \
        -DLLVM_INCLUDE_BENCHMARKS=OFF \
        -DLLVM_INCLUDE_EXAMPLES=OFF \
        -DLLVM_INCLUDE_TESTS=OFF \
        \
        -DLLVM_BUILD_TOOLS=ON \
        -DLLVM_DISTRIBUTION_COMPONENTS="${LLVM_COMPONENTS}" \
        -DLLVM_BUILD_LLVM_DYLIB=ON \
        -DLLVM_ENABLE_ZLIB=OFF \
        -DLLVM_PARALLEL_COMPILE_JOBS="${NPROC}" \
        -DLLVM_PARALLEL_LINK_JOBS=1 \
        "${LLVM_EXTRA_CMAKE_FLAGS}" \
        "${LLVM_SOURCES}"

    ninja distribution
    if [[ -n "${INSTALL_DIR}" ]]; then
        install "${INSTALL_PREFIX}" "${DO_STRIPPING}" "${DO_TARS}"
    fi
fi

if [[ "x${BUILD_OHOS_RELEASE_GN}" == "xtrue" ]]; then
    TARGET="ohos-gn${BUILD_SUFFIX}"
    INSTALL_DIR_NAME="llvm-${VERSION}-release-${TARGET}"
    INSTALL_PREFIX="${INSTALL_DIR}/${INSTALL_DIR_NAME}"
    BUILD_PREFIX="${BUILD_DIR}/${INSTALL_DIR_NAME}"
    mkdir -p "${BUILD_PREFIX}"
    cd "${BUILD_PREFIX}"
    "${OHOS_SDK}/build-tools/cmake/bin/cmake" -G Ninja \
        -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
        -DPACKAGE_VERSION="${PACKAGE_VERSION}" \
        -DCMAKE_BUILD_TYPE=Release \
        \
        -DCMAKE_CROSSCOMPILING=ON \
        -DLLVM_TARGET_ARCH=AArch64 \
        -DCMAKE_SYSROOT="${OHOS_SDK}/sysroot" \
        -DLLVM_HOST_TRIPLE=aarch64-linux-ohos \
        -DLLVM_DEFAULT_TARGET_TRIPLE=aarch64-linux-ohos \
        -DCMAKE_C_FLAGS="-fstack-protector-strong --target=aarch64-linux-ohos -ffunction-sections -fdata-sections -v -funwind-tables -no-canonical-prefixes -D__MUSL__" \
        -DCMAKE_CXX_FLAGS="-fstack-protector-strong --target=aarch64-linux-ohos -ffunction-sections -fdata-sections -v -funwind-tables -no-canonical-prefixes -D__MUSL__ -stdlib=libc++" \
        -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld -Wl,--gc-sections -Wl,--build-id=sha1 --rtlib=compiler-rt -stdlib=libc++ -Wl,-z,relro,-z,now -pie -lunwind -Wl,-rpath,'\$ORIGIN/../lib' -static-libstdc++" \
        \
        -DCMAKE_C_COMPILER="${OHOS_PREBUILTS}/clang/ohos/linux-x86_64/llvm/bin/clang" \
        -DCMAKE_CXX_COMPILER="${OHOS_PREBUILTS}/clang/ohos/linux-x86_64/llvm/bin/clang++" \
        -DCMAKE_STRIP="${STRIP}" \
        \
        -DLLVM_ENABLE_FFI=OFF \
        -DLLVM_ENABLE_TERMINFO=OFF \
        \
        -DLLVM_INCLUDE_BENCHMARKS=OFF \
        -DLLVM_INCLUDE_EXAMPLES=OFF \
        -DLLVM_INCLUDE_TESTS=OFF \
        \
        -DLLVM_BUILD_TOOLS=ON \
        -DLLVM_DISTRIBUTION_COMPONENTS="${LLVM_COMPONENTS}" \
        -DLLVM_BUILD_LLVM_DYLIB=ON \
        -DLLVM_ENABLE_ZLIB=OFF \
        -DLLVM_TARGETS_TO_BUILD=AArch64 \
        -DLLVM_PARALLEL_COMPILE_JOBS="${NPROC}" \
        -DLLVM_PARALLEL_LINK_JOBS=1 \
        "${LLVM_TABLEGEN}" \
        "${LLVM_EXTRA_CMAKE_FLAGS}" \
        "${LLVM_SOURCES}"

    ninja distribution
    if [[ -n "${INSTALL_DIR}" ]]; then
        install "${INSTALL_PREFIX}" "${DO_STRIPPING}" "${DO_TARS}"
    fi
fi
