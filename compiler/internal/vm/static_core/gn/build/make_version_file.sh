#!/usr/bin/env bash
# Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

# arg1 - path to panda_root
function git_hash() {
    if git --version &> /dev/null ; then
        if git -C "$1" rev-parse &> /dev/null ; then
            echo $(git -C "$1" rev-parse HEAD)
        else
            echo ""
        fi
    else
        echo ""
    fi
}

if [ $# -ne 3 ]; then
    echo "Unvalid arguments"
    echo "Usage: make_version_file.sh <git root dir> <input file> <output file>"
    exit 1
fi

ROOT_DIR=$1
INPUT=$2
OUTPUT=$3
HASH=$(git_hash "$ROOT_DIR")

if [ -z "$HASH" ]; then
    python3 "$ROOT_DIR/gn/build/cmake_configure_file.py" "$INPUT" "$OUTPUT"
else
    python3 "$ROOT_DIR/gn/build/cmake_configure_file.py" "$INPUT" "$OUTPUT" "ARK_VERSION_GIT_HASH=$HASH"
fi
