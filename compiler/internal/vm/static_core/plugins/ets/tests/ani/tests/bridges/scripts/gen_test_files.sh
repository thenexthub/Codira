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

set -e

DIR=$(cd $(dirname "$0") && pwd)
GENERATOR=${DIR}/gen_test_files.py
VENV_DIR=${VENV_DIR:-$(realpath ~/.venv-panda)}

if [ $# -ge 3 ]; then
    STATIC_ROOT_DIR=$1
    BUILD_DIR=$2
    ARM32=$3

    source "${STATIC_ROOT_DIR}/scripts/python/venv-utils.sh"
    activate_venv
    echo "${GENERATOR}: generating test files"
    python3 -B "${GENERATOR}" "${BUILD_DIR}" "${DIR}/../j2" "${ARM32}"
    deactivate_venv
    echo "${VENV_DIR} deactivate"

    "${STATIC_ROOT_DIR}"/scripts/code_style/code_style_check.py --reformat "${BUILD_DIR}">/dev/null
else
    echo "Usage $0 <static_root_dir> <panda_build_dir> <use arm32(true/false)>"
fi
