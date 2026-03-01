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

# The script clones https://gitee.com/openharmony-sig/arkcompiler_ets_runtime to plugins/ecmascripts and 
# clones https://gitee.com/openharmony-sig/arkcompiler_ets_frontend to plugins/ecmascripts/es2panda. 
# You can add Nikname as parameters for cloning fork. 
# !NOTE the scripts must be run from arkcompiler_runtime_core repo

set -euo pipefail

# Set args
GITEE_USER=${1:-"openharmony-sig"}

# Check args
if [[ "$GITEE_USER" == "-h" || "$GITEE_USER" == "--help" ]]; then
    echo "Usage: $0 [gitee_user (openharmony-sig by default)]"
    exit 1
fi

# Get links to repos
ETS_RUNTIME_URL=git@gitee.com:${GITEE_USER}/arkcompiler_ets_runtime.git
ETS_FRONTEND_URL=git@gitee.com:${GITEE_USER}/arkcompiler_ets_frontend.git

# clone ets_runtime to plugins/cmascript
cd plugins
echo "Clone: $ETS_RUNTIME_URL"
git clone "$ETS_RUNTIME_URL" ecmascript


# clone ets_frontend to es2panda
cd ecmascript

if [[ "${GITEE_USER}" != "openharmony-sig" ]]; then
    git remote add upstream git@gitee.com:openharmony-sig/arkcompiler_ets_runtime.git
    echo "set upstream for arkcompiler_ets_runtime to git@gitee.com:openharmony-sig/arkcompiler_ets_runtime.git"
fi

echo "Clone: $ETS_FRONTEND_URL"
git clone "$ETS_FRONTEND_URL" es2panda

if [[ "${GITEE_USER}" != "openharmony-sig" ]]; then
    cd es2panda
    git remote add upstream git@gitee.com:openharmony-sig/arkcompiler_ets_frontend.git
    echo "set upstream for arkcompiler_ets_frontend to git@gitee.com:openharmony-sigarkcompiler_ets_frontend.git"
fi
