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

set -eo pipefail

function print_test_help() {
    HELP_MESSAGE="
    This script builds Panda SDK.

    SYNOPSIS

    $0 [OPTIONS]

    OPTIONS

    --help, -h              Show this message and exit.

    --build_type=...        [Release/Debug/FastVerify] Set build type
    "

    echo "$HELP_MESSAGE"
}

SCRIPT_DIR="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"
PANDA_SDK_BUILD_TYPE="Release"

for i in "$@"; do
    ERROR_ARG=""
    case $i in
    -h|--help)
        print_test_help
        exit 0
        ;;
    --build_type=*)
        TYPE_ARG=${i//[-a-zA-Z0-9]*=/}
        if [[ "$TYPE_ARG" = "Release" ]]; then
            PANDA_SDK_BUILD_TYPE="Release"
        fi
        if [[ "$TYPE_ARG" = "Debug" ]]; then
            PANDA_SDK_BUILD_TYPE="Debug"
        fi
        if [[ "$TYPE_ARG" = "FastVerify" ]]; then
            PANDA_SDK_BUILD_TYPE="FastVerify"
        fi
        shift
        ;;
    *)
        ERROR_ARG="YES"
    esac

    if [[ -n "${ERROR_ARG}" ]]; then
        echo "Error: Unsupported flag $i" >&2
        exit 1
    fi
done

bash "$SCRIPT_DIR"/test_sdk --build_type="$PANDA_SDK_BUILD_TYPE"
