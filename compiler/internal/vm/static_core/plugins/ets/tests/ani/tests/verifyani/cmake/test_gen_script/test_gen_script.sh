#!/usr/bin/env bash
# Copyright (c) 2026 Huawei Device Co., Ltd.
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

set -e

DIR=$(cd $(dirname "$0") && pwd)
GENERATOR=${DIR}/test_gen_script.py
VENV_DIR=${VENV_DIR:-$(realpath ~/.venv-panda)}

if [ $# -ge 3 ]; then
    STATIC_ROOT_DIR=$1
    BUILD_DIR=$2
    shift 2

    source "${STATIC_ROOT_DIR}/scripts/python/venv-utils.sh"
    echo 2
    activate_venv
    echo "${GENERATOR}: generating test files"
    python3 "${GENERATOR}" "$@"
    deactivate_venv
    echo "${VENV_DIR} deactivate"

    "${STATIC_ROOT_DIR}"/scripts/code_style/code_style_check.py --reformat "${BUILD_DIR}">/dev/null
else
    echo "Usage $0 <static_root_dir> <panda_build_dir> <use arm32(true/false)>"
fi
