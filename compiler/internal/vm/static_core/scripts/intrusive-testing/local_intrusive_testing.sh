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

export BUILD_TOOL=ninja
export INTRUSIVE_TESTING=true
export CMAKE_BUILD_TYPE=Debug
export CMAKE_TOOLCHAIN_FILE=cmake/toolchain/host_clang_14.cmake
EXTERNAL_SYMBOLIZER_PATH=/usr/bin/llvm-symbolizer
export NPROC_PER_JOB=$(nproc)

CMAKE_OPTIONS="-DPANDA_ENABLE_THREAD_SANITIZER=true"
CMAKE_OPTIONS="${CMAKE_OPTIONS} -DPANDA_ENABLE_UNDEFINED_BEHAVIOR_SANITIZER=false"
CMAKE_OPTIONS="${CMAKE_OPTIONS} -DPANDA_ENABLE_ADDRESS_SANITIZER=false"
CMAKE_OPTIONS="${CMAKE_OPTIONS} -DPANDA_THREAD_SAFETY=false"
CMAKE_OPTIONS="${CMAKE_OPTIONS} -DINTRUSIVE_TESTING=true"
export CMAKE_OPTIONS

CLEAR_BUILD_DIRS=false
RUN_ONLY=false

function usage() {
    echo "Usage: $0 [-cr] <path_to_repo> <path_to_build_dir>"
    echo "   -c: clear existing build directory"
    echo "   -r: run tests only (instrumentation should have been already performed)"
    echo "Environment variable INTRUSIVE_TESTS_RELATIVE_DIR sets a directory with intrusive test set"
    echo "Environment variable INSTRUMENTATION_TARGETS sets a directory with a taget for intrusive tests"
    echo "Environment variable TSAN_SUPPRESSIONS sets a path to file with suppressions for thread sanitizer"
    exit 1
}

# The following environment variables might be defined: INTRUSIVE_TESTS_RELATIVE_DIR, INSTRUMENTATION_TARGETS, TSAN_SUPPRESSIONS.
if [[ -z "$INTRUSIVE_TESTS_RELATIVE_DIR" ]]; then
    export INTRUSIVE_TESTS_RELATIVE_DIR="runtime/tests/intrusive-tests"
fi
if [[ -z "$INSTRUMENTATION_TARGETS" ]]; then
    export INSTRUMENTATION_TARGETS="intrusive_test"
fi
if [[ -z "$BUILD_TARGETS" ]]; then
    export BUILD_TARGETS=${INSTRUMENTATION_TARGETS}_run
fi
tsan_options="external_symbolizer_path=$EXTERNAL_SYMBOLIZER_PATH"
if [[ -n "$TSAN_SUPPRESSIONS" ]]; then
    tsan_options="${tsan_options},suppressions=${TSAN_SUPPRESSIONS}"
fi

function run_tests() {
    echo "Run intrusive tests"
    # shellcheck disable=SC2086
    ${BUILD_TOOL} -k1 -j"${NPROC_PER_JOB}" ${BUILD_TARGETS}
}

function clear_dir() {
    TARGET_DIR=$1
    if [[ -d "$TARGET_DIR" ]]; then
        if [[ "$CLEAR_BUILD_DIRS" = true ]]; then
            rm -rf "$TARGET_DIR"
        else
            echo "Directory $TARGET_DIR is not empty. Clear it or use flag -c"
            exit 1
        fi
    fi
}

while getopts "h?cr" opt; do
    case "$opt" in
    h|\?)
        usage
        exit 0
        ;;
    c)  CLEAR_BUILD_DIRS=true
        ;;
    r)  RUN_ONLY=true
        ;;
    esac
done

shift $(($OPTIND - 1))

# Script takes two parameters one is the path to the repo
if [[ ! -d "$1" ]]; then
    echo "Directory with repository does not exist"
    usage
fi
# Second parameter is the path where to build
if [[ -z "$2" ]]; then
    echo "Path to build directory is not set"
    usage
fi

export ROOT_DIR=$(realpath "$1")
export ARTIFACTS_DIR=$(realpath "$2")

export TSAN_OPTIONS=${tsan_options}

BUILD_DIR=$ARTIFACTS_DIR/build
INTERMEDIATE_PANDA_DIR=$ARTIFACTS_DIR/instrumented
CLADE_DIR=$BUILD_DIR/clade
if ! mkdir -p "${BUILD_DIR}"; then
    echo "Cannot create build directory ${BUILD_DIR}"
    usage
fi

cd "$BUILD_DIR"

if [[ "$RUN_ONLY" = true ]]; then
    run_tests
    exit 0
fi

echo "Copy sources into intermediate directory for instrumentation"

clear_dir "$CLADE_DIR"
clear_dir "$INTERMEDIATE_PANDA_DIR"

cp -rL "$ROOT_DIR" "$INTERMEDIATE_PANDA_DIR"

echo "Configure build"
# shellcheck disable=SC2086
if ! cmake "$INTERMEDIATE_PANDA_DIR" -GNinja -DPANDA_C2ABC_UPGRADE_LEGACY=true -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} ${CMAKE_OPTIONS}; then
    echo "Cannot configure build"
    exit 1
fi

echo "Build ARK and tests to create a commands graph"
# shellcheck disable=SC2086
if ! clade -e CmdGraph --config '{"SrcGraph.requires":["CC","CXX"]}' ${BUILD_TOOL} -k1 -j"${NPROC_PER_JOB}" ${INSTRUMENTATION_TARGETS}; then
    echo "Cannot build ARK and tests to create a commands graph"
    exit 1
fi

echo "Run instrumentation"
INSTRUMENTATOR=$INTERMEDIATE_PANDA_DIR/scripts/intrusive-testing/intrusive_instrumentator.py
if ! python3 "$INSTRUMENTATOR" "$INTERMEDIATE_PANDA_DIR" "${CLADE_DIR}/cmds.txt" "${INTERMEDIATE_PANDA_DIR}/${INTRUSIVE_TESTS_RELATIVE_DIR}"; then
    echo "Cannot run instrumentation"
    exit 1
fi

run_tests
