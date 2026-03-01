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

set -o pipefail

for ARGUMENT in "$@"; do
case "$ARGUMENT" in
    --run-prefix=*)
    RUN_PREFIX="${ARGUMENT#*=}"
    ;;
    --ark-binary=*)
    ARK_BINARY="${ARGUMENT#*=}"
    ;;
    --boot-panda-files=*)
    BOOT_PANDA_FILES="${ARGUMENT}"
    ;;
    --non-existing-abc=*)
    NON_EXISTING_ABC="${ARGUMENT#*=}"
    ;;
    --entry-abc=*)
    ENTRY_ABC="${ARGUMENT#*=}"
    ;;
    --entrypoint=*)
    ENTRYPOINT="${ARGUMENT#*=}"
    ;;
    *)
    echo "Unexpected argument: '${ARGUMENT}'"
    exit 1
    ;;
esac
done

EXPECTED_ERROR_MESSAGE="Error: Open failed, file: ${NON_EXISTING_ABC}"

RUNTIME_ARGUMENTS="${BOOT_PANDA_FILES} --load-runtimes=ets --gc-type=g1-gc --panda-files=${NON_EXISTING_ABC} ${ENTRY_ABC} ${ENTRYPOINT}"

# shellcheck disable=SC2086
STDERR=$(${RUN_PREFIX} "${ARK_BINARY}" ${RUNTIME_ARGUMENTS} 2>&1 >/dev/null)
RETURN_CODE=$?

if [ ${RETURN_CODE} -eq 1 ] && [[ "${STDERR}" =~ "${EXPECTED_ERROR_MESSAGE}" ]]; then
    # Runtime failed with expected message in stderr - test has passed
    exit 0
fi

echo Expected runtime unhandled error with the following message: "${EXPECTED_ERROR_MESSAGE}"
echo Captured stderr: "${STDERR}"
echo Return code: "${RETURN_CODE}"
exit 1
