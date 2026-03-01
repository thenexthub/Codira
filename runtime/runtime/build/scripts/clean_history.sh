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

set -e

CURRENT_PATH="$(readlink -f "$0")"
PROJECT_PATH="$(dirname $CURRENT_PATH)/../../"
TEST_PATH="$PROJECT_PATH/test_tools/tests/codethread_test"

echo "-----------------------------------------------------------------"
echo "clean codethread project begin..."
# clean all construct file

if [ -d "${PROJECT_PATH}/output" ]; then
    rm -rf ${PROJECT_PATH}/output
fi

if [ -d "${PROJECT_PATH}/build/codethread_build" ]; then
    rm -rf ${PROJECT_PATH}/build/codethread_build
fi

if [ -d "${TEST_PATH}/codethread_sdv/build" ]; then
    rm -rf ${TEST_PATH}/codethread_sdv/build
fi

if [ -d "${TEST_PATH}/codethread_sdv/bin" ]; then
    rm -rf ${TEST_PATH}/codethread_sdv/bin
fi

if [ -d "${TEST_PATH}/dtest/build" ]; then
    rm -rf ${TEST_PATH}/dtest/build
fi

if [ -d "${TEST_PATH}/dtest/lib" ]; then
    rm -rf ${TEST_PATH}/dtest/lib
fi

rm -f ${PROJECT_PATH}/Test_Report_*
rm -f ${PROJECT_PATH}/result.xml
rm -f ${PROJECT_PATH}/test_syscall.txt

echo "clean codethread project end..."
echo "-----------------------------------------------------------------"


