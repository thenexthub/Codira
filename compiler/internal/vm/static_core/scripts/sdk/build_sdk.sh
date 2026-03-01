#!/usr/bin/env bash
#===----------------------------------------------------------------------===#
#   Copyright (c) NeXTHub Corporation. All Rights Reserved.
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Author: Tunjay Akbarli
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#   Middletown, DE 19709, New Castle County, USA.
#===----------------------------------------------------------------------===#

set -eo pipefail
set -x

export PANDA_SDK_BUILD_TYPE="Release"
export OHOS_ARM32="false"
export OHOS_ARM64="false"
export LINUX_ARM64_TOOLS="false"
export LINUX_TOOLS="false"
export WINDOWS_TOOLS="false"
export TS_LINTER="false"
export ETS_STD_LIB="false"
export ICU_DAT_FILE="false"
export PANDA_LLVM_BACKEND="ON"
export PANDA_BUILD_LLVM_BINARIES="OFF"
export PANDA_WITH_SAFEPOINT_CHECKER="OFF"
export PANDA_TRACK_INTERNAL_ALLOCATIONS=0
export LLVM_BIN_AARCH64_RELEASE="/opt/llvm-15-release-aarch64"
export LLVM_BIN_AARCH64_FASTVERIFY="/opt/llvm-15-release-aarch64-fastverify"
export LLVM_BIN_AARCH64_DEBUG="/opt/llvm-15-debug-aarch64"
export LLVM_BIN_X86_64_RELEASE="/opt/llvm-15-release-x86_64"
export LLVM_BIN_X86_64_DEBUG="/opt/llvm-15-debug-x86_64"
export LLVM_BIN_OHOS_RELEASE="/opt/llvm-15-release-ohos"
export LLVM_BIN_OHOS_FASTVERIFY="/opt/llvm-15-release-ohos-fastverify"

export SCRIPT_DIR="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"

export ARK_ROOT="$SCRIPT_DIR/../.."
export ETS_FRONTEND_ROOT="$ARK_ROOT/../../ets_frontend"

export BUILD_TYPE_RELEASE="Release"
BUILD_TYPE_FAST_VERIFY="FastVerify"
BUILD_TYPE_DEBUG="Debug"

BUILD_DIR=${BUILD_DIR:-"$SCRIPT_DIR"}

source "$SCRIPT_DIR"/build_sdk_lib

if  [ "$1" == "-h" ] || [ "$1" == "--help" ]; then
    print_help
    exit 0
fi

SDK_BUILD_ROOT="$BUILD_DIR/$1"
mkdir -p "$SDK_BUILD_ROOT"
shift

export OHOS_SDK_NATIVE
enable_ohos_sdk_native

parse_options "$@"

case "$PANDA_SDK_BUILD_TYPE" in
"$BUILD_TYPE_RELEASE" | "$BUILD_TYPE_FAST_VERIFY" | "$BUILD_TYPE_DEBUG") ;;
*)
    echo "Invalid build_type option!"
    usage
    ;;
esac

if [ ! -d "$OHOS_SDK_NATIVE" ]; then
    echo "Error: No such directory: $OHOS_SDK_NATIVE"
    usage
fi

if [ -z "$SDK_BUILD_ROOT" ]; then
    echo "Error: path to panda sdk destination is not provided"
    usage
fi

export PANDA_SDK_PATH="$SDK_BUILD_ROOT/sdk"
rm -rf "$PANDA_SDK_PATH"

build_sdk_targets

echo "> Packing NPM package..."
mkdir -p "$PANDA_SDK_PATH"
cp "$SCRIPT_DIR"/package.json "$PANDA_SDK_PATH"
cd "$PANDA_SDK_PATH"
npm pack --pack-destination ..
cd "$BUILD_DIR"
