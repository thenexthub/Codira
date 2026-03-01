#!/bin/bash
# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e

readonly SOURCE_PATH="$1"
readonly WORK_PATH="$2"
readonly TSC_PATH="$3"
readonly TGZ_NAME="$4"

readonly TMP_DECLGEN_PATH="$WORK_PATH/declgen_tmp"
readonly NODE_MODULES_PATH="$TMP_DECLGEN_PATH/node_modules"
readonly TMP_TSC_PATH="$WORK_PATH/tsc_tmp"
readonly TSC_PACKAGE_PATH="$TMP_TSC_PATH/ohos-typescript-4.9.5-r4.tgz"

target_files=("lib" "bin" "package.json" "LICENSE" "README.md" "README.OpenSource" "SECURITY.md" "ThirdPartyNoticeText.txt")

# clean and create temporary directories
rm -rf "$TMP_DECLGEN_PATH" && cp -rP "$SOURCE_PATH/" "$TMP_DECLGEN_PATH/"
rm -rf "$TMP_TSC_PATH" && mkdir -p "$TMP_TSC_PATH"
for file in "${target_files[@]}"; do
    if [ -e "$TSC_PATH/$file" ]; then
        cp -rP "$TSC_PATH/$file" "$TMP_TSC_PATH/"
    fi
done

# pack ohos-typescript-4.9.5-r4.tgz in TMP_TSC_PATH
pushd "$TMP_TSC_PATH" > /dev/null
    npm pack
popd > /dev/null

# decompress ohos-typescript-4.9.5-r4.tgz to NODE_MODULES_PATH
tar -xzf "$TSC_PACKAGE_PATH" -C "$NODE_MODULES_PATH"
rm -rf "$NODE_MODULES_PATH/typescript" && mv "$NODE_MODULES_PATH/package" "$NODE_MODULES_PATH/typescript"

# build panda declgen
pushd "$TMP_DECLGEN_PATH" > /dev/null
    npm run build
popd > /dev/null

# pack panda declgen
pushd "$TMP_DECLGEN_PATH" > /dev/null
    npm pack
popd > /dev/null

cp "$TMP_DECLGEN_PATH/$TGZ_NAME" "$WORK_PATH/$TGZ_NAME"
