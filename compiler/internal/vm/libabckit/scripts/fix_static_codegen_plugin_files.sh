#!/usr/bin/env bash
# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

OUTPUT_DIR="${1}"
shift

for FILE_PATH in "${@}"; do
    if [[ ! -f "${FILE_PATH}" ]]; then
        echo "File not found: ${FILE_PATH}"
        continue
    fi

    f="$(basename -- "$FILE_PATH")"
    echo "Fix plugin's file path in ${FILE_PATH}"

    sed -E 's/BytecodeGen/CodeGenStatic/g' "${FILE_PATH}" > "${OUTPUT_DIR}/$f"
    if [[ "$OSTYPE" == "darwin"* ]]; then
    sed -i '' 's/plugins\/ets\/bytecode_optimizer\/visitors\///g' "${OUTPUT_DIR}/$f"
    else
    sed -i 's/plugins\/ets\/bytecode_optimizer\/visitors\///g' "${OUTPUT_DIR}/$f"
    fi
done
