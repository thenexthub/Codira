#!/usr/bin/env bash
# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

set -e -o pipefail

readonly SCRIPT_DIR="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"
readonly GENPATH="${SCRIPT_DIR}/../../stdlib"
readonly GEN_ESCOMPAT_PATH="${1:-${GENPATH}}/escompat"
readonly GEN_STDCORE_PATH="${1:-${GENPATH}}/std/core"
readonly VENV_DIR=${VENV_DIR:-$(realpath ~/.venv-panda)}
readonly GEN_STDINTEROP_PATH="${1:-${GENPATH}}/std/interop/js"
readonly ARR="${GEN_STDCORE_PATH}/Array.ets"
readonly BLT_ARR="${GEN_STDCORE_PATH}/BuiltinArray.ets"
readonly BLT_ARR_SORT="${GEN_STDCORE_PATH}/BuiltinArraySort.ets"
readonly BLT_ARR_ARG="${GEN_STDCORE_PATH}/BuiltinArrayAlgorithms.ets"
readonly DATAVIEW="${GEN_STDCORE_PATH}/DataView.ets"
readonly TYPED_ARR="${GEN_ESCOMPAT_PATH}/TypedArrays.ets"
readonly TYPED_UARR="${GEN_ESCOMPAT_PATH}/TypedUArrays.ets"
readonly FUNC="${GEN_STDCORE_PATH}/Function.ets"
readonly TUP="${GEN_STDCORE_PATH}/Tuple.ets"
readonly INTEROP_TRANSFER_HELPER="${GEN_STDINTEROP_PATH}/InteropTransferHelper.ets"

cd "$SCRIPT_DIR"

JINJA_PATH="${VENV_DIR}/bin/jinja2"
if ! [[ -f "${JINJA_PATH}" ]]; then
    JINJA_PATH="jinja2"
fi

function format_file() {
    sed -e 's/\b \s\+\b/ /g' | sed -e 's/\s\+$//g' | sed -e 's+/\*\s*\*/\s*++g' | cat -s
}

mkdir -p "${GEN_ESCOMPAT_PATH}"
mkdir -p "${GEN_STDCORE_PATH}"
mkdir -p "${GEN_STDINTEROP_PATH}"

# Generate Array
echo "Generating ${ARR}"
erb Array_escompat.erb | format_file > "${ARR}"

echo "Generating ${BLT_ARR}"
erb Array_builtin.erb | format_file > "${BLT_ARR}"

echo "Generating ${BLT_ARR_SORT}"
"${JINJA_PATH}" Array_builtin_sort.ets.j2 | format_file > "${BLT_ARR_SORT}"

echo "Generating ${BLT_ARR_ARG}"
"${JINJA_PATH}" Array_builtin_algorithms.ets.j2 | format_file > "${BLT_ARR_ARG}"

# Generate TypedArrays
echo "Generating ${DATAVIEW}"
"${JINJA_PATH}" "${SCRIPT_DIR}/DataView.ets.j2" -o "${DATAVIEW}"

echo "Generating ${TYPED_ARR}"
"${JINJA_PATH}" "${SCRIPT_DIR}/typedArray.ets.j2" -o "${TYPED_ARR}"

echo "Generating ${TYPED_UARR}"
"${JINJA_PATH}" "${SCRIPT_DIR}/typedUArray.ets.j2" -o "${TYPED_UARR}"

# Generate Functions
echo "Generating ${FUNC}"
"${JINJA_PATH}" "${SCRIPT_DIR}/Function.ets.j2" -o "${FUNC}"

# Generate Tuples
echo "Generating ${TUP}"
"${JINJA_PATH}" "${SCRIPT_DIR}/Tuple.ets.j2" -o "${TUP}"

# Generate InteropTransferHelper
echo "Generating ${INTEROP_TRANSFER_HELPER}"
"${JINJA_PATH}" "${SCRIPT_DIR}/InteropTransferHelper.ets.j2" -o "${INTEROP_TRANSFER_HELPER}"

exit 0
