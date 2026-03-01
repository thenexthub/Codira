#!/usr/bin/env bash
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

set -e

SANDBOX_DIR=$1
IN_PATH=$2
OUT_PATH=$3
SOURCE_LANG=$4

IN_DIR="$(dirname $IN_PATH)"
IN_NAME=${IN_PATH##*/}
IN_STEM=${IN_NAME%.*}
IN_PATH_LOWERED=$(echo $IN_PATH | sed 's|[/\\ ]|_|g')

TMP_FILE="$SANDBOX_DIR/$IN_PATH_LOWERED.txt"

ACCEPTED_SOURCE_LANGS=("" "js" "ts" "ets")
# CC-OFFNXT(bc-50007) can not put quotes to match regex
if [[ ! "${ACCEPTED_SOURCE_LANGS[@]}" =~ $SOURCE_LANG ]]; then
    echo "Unknown source language:" $SOURCE_LANG >&2
    exit -1
fi

# CSV schema of merge file used by frontend:
# $fileName;$recordName;$scriptKind;$sourceFile;$pkgName;$IsSharedModule;$OriginSourceLang

echo "$IN_NAME;$IN_STEM;esm;$IN_NAME;entry;false;$SOURCE_LANG" >$TMP_FILE
MODULES_DIR="$IN_DIR/modules"
if [ -d "$MODULES_DIR" ]; then
    for file in $MODULES_DIR/*; do
        MOD_NAME="modules/${file##*/}"
        MOD_STEM=${MOD_NAME%.*}
        echo "$MOD_NAME;$MOD_STEM;esm;$MOD_NAME;entry;false;$SOURCE_LANG" >>$TMP_FILE
    done
fi

/usr/bin/python3 \
    "$(dirname $0)/../../../ets_runtime/test/quickfix/generate_merge_file.py" \
    --input "$TMP_FILE" --output "$OUT_PATH" --prefix "$IN_DIR/"
