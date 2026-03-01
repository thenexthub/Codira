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
#
# shellcheck disable=SC2086

set -eo pipefail

PAOC_OUTPUT="etsstdlib.an"
for ARGUMENT in "$@"; do
case "$ARGUMENT" in
    --binary-dir=*)
    PANDA_BINARY_ROOT="${ARGUMENT#*=}"
    ;;
    --target-arch=*)
    TARGET_ARCH="--compiler-cross-arch=${ARGUMENT#*=}"
    ;;
    --paoc-mode=*)
    PAOC_MODE="--paoc-mode=${ARGUMENT#*=}"
    ;;
    --prefix=*)
    PANDA_RUN_PREFIX="${ARGUMENT#*=}"
    ;;
    -compiler-options=*)
    OPTIONS="${ARGUMENT#*=}"
    ;;
    -paoc-output=*)
    PAOC_OUTPUT="${ARGUMENT#*=}"
    ;;
    -paoc-regex=*)
    PAOC_REGEX="--compiler-regex=${ARGUMENT#*=}"
    ;;
    *)
    echo "Unexpected argument: '${ARGUMENT}'"
    exit 1
    ;;
esac
done

if [ -z "$PANDA_BINARY_ROOT" ]; then
    echo "Please set PANDA_BINARY_ROOT"
    exit 1
fi

${PANDA_RUN_PREFIX} ${PANDA_BINARY_ROOT}/bin/ark_aot --boot-panda-files=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc \
                    --paoc-panda-files=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc  \
                    --compiler-ignore-failures=false $PAOC_MODE $PAOC_REGEX --load-runtimes="ets" \
                    $TARGET_ARCH ${OPTIONS}  --paoc-output=${PAOC_OUTPUT}
