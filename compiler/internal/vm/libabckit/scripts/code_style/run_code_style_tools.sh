#!/bin/bash
# Copyright (c) 2024 Huawei Device Co., Ltd.
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

# A very simple wrapper around clang-format:
# Compare the original file with what clang-format thinks it should
# look like and output corresponding diff. Exit with the same code as diff.
#
# USAGE:
# scripts/run-clang-format /path/to/clang-format /path/to/checked/file
#
# The only reason of this script's existence is to avoid messing with
# escaping special chatracters in CMake files.

set -exu

FILE_NAME="${1}"

CLANG_FORMAT_BIN="clang-format-14"
SCRIPT_DIR="$(dirname $(realpath "${0}"))"

if [ ! -f "${FILE_NAME}" ]; then
    echo "FATAL: Input file '${FILE_NAME}' is not found"
    exit 1
fi

# Here we rely on diff's exit code:
diff -u --color=auto \
    --label="${FILE_NAME} (original)" \
    --label="${FILE_NAME} (reformatted)" \
    "${FILE_NAME}" <(${CLANG_FORMAT_BIN} "${FILE_NAME}")

# Run additional checks
SCRIPTS_DIR="${SCRIPT_DIR}/../extras/code_style/"
SCRIPTS_TO_RUN=("run-check-concurrency-format.sh" "run_check_atomic_format.py")
for S in "${SCRIPTS_TO_RUN[@]}"; do
    if [ -f "${SCRIPTS_DIR}/${S}" ]; then
        "${SCRIPTS_DIR}/${S}" "${FILE_NAME}"
    fi
done
