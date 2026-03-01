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

function usage() {
    echo "Usage: run_test.sh [--env NAME=VALUE]... --dir <working directory> [--module <native module>]... --etsstdlib <ets stdlib abc file> <COMMAND>"
    exit 1
}

WORKING_DIR=""
declare -a MODULES
ETSSTDLIB=""
CMD=""
declare TEST_LOG_FILE="log.txt"

ARG=$1
while [ ! -z "${ARG}" ]; do
    case "${ARG}" in
    "--env")
        shift 1
        if [ -z "$1" ]; then
            echo "Error: expect NAME=VALUE"
            exit 1;
        fi
        export "$1"
        ;;
    "--dir")
        shift 1
        WORKING_DIR="$1"
        ;;
    "--module")
        shift 1
        MODULES+=("$1")
        ;;
    "--etsstdlib")
        shift 1
        ETSSTDLIB=$1
        ;;
    *)
        CMD=$1
        ;;
    esac
    shift 1
    ARG=$1
done

if [ -z "${WORKING_DIR}" ]; then
    echo "Error: working directory is not specified"
    usage
fi
if [ -z "${ETSSTDLIB}" ]; then
    echo "Error: etsstdlib file is not specified"
    usage
fi
if [ -z "${CMD}" ]; then
    echo "Error: command is not specified"
    usage
fi

mkdir -p "${WORKING_DIR}/module"
for m in ${MODULES[@]}; do
    ln -sf "${m}" "${WORKING_DIR}/module/"
done
ln -sf "${ETSSTDLIB}" "${WORKING_DIR}"

cd "${WORKING_DIR}"
if ! ${CMD} &> "${TEST_LOG_FILE}" ; then
    cat "${TEST_LOG_FILE}"
    echo "To reproduce test do the following ..."
    echo "cd $(pwd)"
    echo "LD_LIBRARY_PATH=${LD_LIBRARY_PATH} ${CMD}"
    exit 1
fi
