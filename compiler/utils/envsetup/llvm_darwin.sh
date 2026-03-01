#!/bin/bash

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

# This script needs to be placed in the output directory of Codira compiler.
# ** NOTE: Please use `source' command to execute this script. **

# Get current shell name.
shell_name=$(basename -- $(ps -o comm= $$))

# Get the absolute path of this script according to different shells.
case "${shell_name}" in
    "zsh" | "-zsh")
        source_dir="${(%):-%N}"
        ;;
    "sh" | "-sh" | "bash" | "-bash")
        source_dir="${BASH_SOURCE[0]}"
        ;;
    *)
        echo "[ERROR] Unsupported shell: ${shell_name}, please switch to bash, sh or zsh."
        return 1
        ;;
esac

if [ -L "${source_dir}" ]; then
    if command -v realpath 2>&1 >/dev/null; then
        source_dir=$(realpath "${source_dir}")
    else
        echo '`realpath` is not found, setup may not process properly.'
    fi
fi
script_dir=$(cd "$(dirname "${source_dir}")"; pwd)

export CODIRA_HOME=${script_dir}

hw_arch=$(uname -m)
if [ "$hw_arch" = "" ]; then
    hw_arch="x86_64"
elif [ "$hw_arch" = "arm64" ]; then
    hw_arch="aarch64"
fi
export PATH=${CODIRA_HOME}/bin:${CODIRA_HOME}/tools/bin:$PATH:${HOME}/.codepm/bin
export DYLD_LIBRARY_PATH=${CODIRA_HOME}/runtime/lib/darwin_${hw_arch}_codenative:${CODIRA_HOME}/tools/lib:${DYLD_LIBRARY_PATH}
unset hw_arch

if [ -z ${SDKROOT+x} ]; then
    export SDKROOT=$(xcrun --sdk macosx --show-sdk-path)
fi

xattr -dr com.apple.quarantine ${script_dir}/* &> /dev/null || true
codesign -s - -f --preserve-metadata=entitlements,requirements,flags,runtime ${script_dir}/third_party/llvm/bin/debugserver &> /dev/null || true
