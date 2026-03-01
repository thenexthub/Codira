#!/bin/bash

# ===----------------------------------------------------------------------===
#
#  Copyright (c) NeXTHub Corporation. All Rights Reserved.
#  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#  Author: Tunjay Akbarli
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at:
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#  Middletown, DE 19709, New Castle County, USA.
#
# ===----------------------------------------------------------------------===

# CI shell script for calling build.py
set -e

MACHINE="$(uname -m)"

if [ "${MACHINE}" = "x86_64" ]; then
    platform="linux_x86_64"
elif [ "${MACHINE}" = "aarch64" ]; then
    platform="linux_aarch64"
fi

script_abs="$(readlink -f "$0")"
export PROJECT_PATH="$(dirname $script_abs)/../"
export CODETHREAD_PATH="${PROJECT_PATH}/src/CODEThread"
export BUILD_PATH="${PROJECT_PATH}/build/codethread_build"

# Example
# 1. make static lib
# sh build/build_codethread.sh -p linux_x86_64 Debug SHARED
# sh build/build_codethread.sh -p linux_x86_64 Debug SHARED asan
# sh build/build_codethread.sh -p linux_x86_64_gcov Debug SHARED
# 2. make test
# sh build/build_codethread.sh -t codethread_sdv -s src -a linux_x86_64 -m Debug

if [ "$1" = "clean" ];then
    cd "${PROJECT_PATH}"
    /bin/bash ./build/scripts/clean_history.sh
elif [ "$1" = "-t" ];then
    cd "${PROJECT_PATH}"/test_tools/tests/codethread_test/codethread_sdv/src
    /bin/bash build_test.sh "$@"
elif [ "$1" = "lcov" ];then
    cd "${PROJECT_PATH}"/test_tools/tests/codethread_test/codethread_sdv/src
    /bin/bash build_lcov.sh "$@"
elif [ "$1" = "-p" ];then
    if [ -d "${BUILD_PATH}" ]; then
      rm -r ${BUILD_PATH}
    fi
    if [ ! -d "${BUILD_PATH}" ]; then
      mkdir ${BUILD_PATH}
    fi

    # DO NOT remove install prefix directory ($7)

    if [ ! -d "${PROJECT_PATH}/output" ]; then
      mkdir -p ${PROJECT_PATH}/output/temp/lib
      mkdir -p ${PROJECT_PATH}/output/temp/include
    fi

    cd "${BUILD_PATH}"
    echo "CODETHREAD BUILDING: target:$2, build type: $3, libtype: $4, building stage: $5, other definitions: $6, path: ${CODETHREAD_PATH}"
    if [ -n "$9" ]; then
      cmake -DTARGET="$2" -DCMAKE_BUILD_TYPE="$3" -DLIBTYPE="$4" -DBUILDING_STAGE="$5" $6 -DCMAKE_INSTALL_PREFIX="$7" -DTARGET_ARCH="$8" -DBUILD_APPLE_STATIC="$9" ${CODETHREAD_PATH}
    else
      cmake -DTARGET="$2" -DCMAKE_BUILD_TYPE="$3" -DLIBTYPE="$4" -DBUILDING_STAGE="$5" $6 -DCMAKE_INSTALL_PREFIX="$7" -DTARGET_ARCH="$8" ${CODETHREAD_PATH}
    fi
    make -j32 && make install
fi

exit 0
