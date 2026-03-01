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

# The script runs update_master_and_branch.sh for arkcompiler_runtime_core, 
# arkcompiler_runtime_core/plugins/ecmascript (fork https://gitee.com/openharmony-sig/arkcompiler_ets_runtime) and 
# arkcompiler_runtime_core/tools/es2panda (fork https://gitee.com/openharmony-sig/arkcompiler_ets_frontend).
# The script can be run from any folder but for this you need to add path to arkcompiler_runtime_core repo

set -euo pipefail

# Set args
REPO_DIR=${1:-$(pwd)}
SCRIPT_DIR=$REPO_DIR/scripts

echo "Script path: $SCRIPT_DIR"
echo "Repo path: $REPO_DIR"

# update arkcompiler_runtime_core fork
cd "${REPO_DIR}"
echo "update arkcompiler_runtime_core"

"${SCRIPT_DIR}/update_master_and_branch.sh"
echo "update arkcompiler_runtime_core succeeded"

# update arkcompiler_ets_runtime fork
ECMA_DIR=$REPO_DIR/plugins/ecmascript
cd "${ECMA_DIR}"
echo "update ecmascript"

"${SCRIPT_DIR}/update_master_and_branch.sh"
echo "update ecmascript succeeded"

# update arkcompiler_ets_frontend fork
ES2PANDA_DIR=${ECMA_DIR}/es2panda
cd "${ES2PANDA_DIR}"
echo "update es2panda"

"${SCRIPT_DIR}/update_master_and_branch.sh"
echo "update es2panda succeeded"
