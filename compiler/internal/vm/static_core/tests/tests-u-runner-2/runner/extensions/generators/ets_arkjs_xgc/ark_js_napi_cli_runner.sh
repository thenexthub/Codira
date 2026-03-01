#!/bin/bash
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
declare SCRIPT_WORK_DIR=""
declare SCRIPT_BUILD_DIR=""

while [[ -n "$1" ]]; do
    case "$1" in
        --work-dir)
            shift
            SCRIPT_WORK_DIR="$1"
            ;;
        --build-dir)
            shift
            SCRIPT_BUILD_DIR="$1"
            ;;
        *)
            break
            ;;
    esac
    shift
done

if [[ -z "$SCRIPT_WORK_DIR" || -z "$SCRIPT_BUILD_DIR" ]]; then
    exit 1
fi

CLI_PATH="$SCRIPT_BUILD_DIR/arkcompiler/runtime_core/ark_js_napi_cli $@"
cd "${SCRIPT_WORK_DIR}"
mkdir -p module
cp "$SCRIPT_BUILD_DIR/arkcompiler/runtime_core/libets_interop_js_napi.so" "$SCRIPT_WORK_DIR/module"
cp "$SCRIPT_BUILD_DIR/gen/arkcompiler/runtime_core/static_core/plugins/ets/etsstdlib.abc" "$SCRIPT_WORK_DIR"

$CLI_PATH
EXIT_CODE=$?

exit ${EXIT_CODE}
