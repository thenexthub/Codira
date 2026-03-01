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

# Huawei Technologies Co.,Ltd.
set -e

for ARGUMENT in "$@"; do
case "$ARGUMENT" in
    --binary-dir=*)
    PANDA_BINARY_ROOT="${ARGUMENT#*=}"
    ;;
    --root-dir=*)
    PANDA_ROOT="${ARGUMENT#*=}"
    ;;
esac
done

exec 2>/dev/null

"${PANDA_BINARY_ROOT}/bin-gtests/bytecodeopt_unit_tests"
if test -f "${PANDA_BINARY_ROOT}/bin-gtests/bytecodeopt_unit_tests_java"; then
    "${PANDA_BINARY_ROOT}/bin-gtests/bytecodeopt_unit_tests_java"
fi
if test -f "${PANDA_BINARY_ROOT}/bin-gtests/bytecodeopt_unit_tests_ecma"; then
    "${PANDA_BINARY_ROOT}/bin-gtests/bytecodeopt_unit_tests_ecma"
fi

gcov "${PANDA_BINARY_ROOT}/bytecode_optimizer/CMakeFiles/arkbytecodeopt.dir"
if [ -x "$(command -v gcovr)" ]; then
    echo "gcovr found"
    gcovr --version
    gcovr -v -r "${PANDA_ROOT}/bytecode_optimizer" -e "${PANDA_ROOT}/bytecode_optimizer/tests" --object-directory="$PANDA_BINARY_ROOT" --html-details --html -o report.html
else
    echo "gcovr not found"
    exit 1
fi

exit 0
