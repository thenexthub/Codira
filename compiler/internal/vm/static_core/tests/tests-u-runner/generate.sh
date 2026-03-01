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

if [[ -z "$1" ]]; then
    echo "Usage: generate.sh <generator options>"
    echo "    <generator options> options to main.py. To see full list use --help"

    exit 1
fi

SCRIPT_DIR=$(realpath "$(dirname "${0}")")
ROOT_DIR=${STATIC_ROOT_DIR:-"${SCRIPT_DIR}/../.."}

export PYTHONPATH=$PYTHONPATH:${ROOT_DIR}/tests/tests-u-runner
GENERATOR=${ROOT_DIR}/tests/tests-u-runner/runner/plugins/ets/preparation_step.py
GENERATOR_OPTIONS=$*

source "${ROOT_DIR}/scripts/python/venv-utils.sh"
activate_venv
set +e

# shellcheck disable=SC2086
python3 -B "${GENERATOR}" ${GENERATOR_OPTIONS}
EXIT_CODE=$?

set -e
deactivate_venv

exit ${EXIT_CODE}
