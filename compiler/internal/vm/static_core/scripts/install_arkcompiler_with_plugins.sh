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

# The script clones arkcompiler_runtime_core and run scripts/install_ecma_plugin.sh for the clone. 
# First parameter is folder for cloning. You can add Nikname as second parameters for cloning fork

set -euo pipefail

# Set args
REPO_DIR=${1:-""}
GITEE_USER=${2:-"openharmony-sig"}

# Check args
if [[ -z "$REPO_DIR" || -z "$GITEE_USER" ]]; then
    echo "Usage: $0 repo_dir [gitee_user (openharmony-sig by default)]"
    exit 1
fi

mkdir -p "$REPO_DIR"

# Update REPO_DIR to full path
REPO_DIR=$(readlink -f "$REPO_DIR")
FULL_SCRIPT_PATH=$(realpath "${BASH_SOURCE[0]}")

cd "${REPO_DIR}"

# clone arkcompiler_runtime_core repo
CLONE_URL=git@gitee.com:${GITEE_USER}/arkcompiler_runtime_core.git
echo 'CLONE_URL: ' "$CLONE_URL"
git clone "$CLONE_URL"

echo "arkcompiler_runtime_core cloned successed"

# set SCRIPT_DIR for running install_ecma_plugin.sh
SCRIPT_DIR=$(cd -- "$(dirname -- "${FULL_SCRIPT_PATH}")" &> /dev/null && pwd)
echo "Script path: $SCRIPT_DIR"

# install ecmascript plugin
cd arkcompiler_runtime_core
if [[ "${GITEE_USER}" != "openharmony-sig" ]]; then
    git remote add upstream git@gitee.com:openharmony-sig/arkcompiler_runtime_core.git
    echo "set upstream for arkcompiler_runtime_core to git@gitee.com:openharmony-sig/arkcompiler_runtime_core.git"
fi

"${SCRIPT_DIR}/install_ecma_plugin.sh" "$GITEE_USER"

echo "ecmascript plugin installed"
