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

SCRIPT_DIR=$(realpath "$(dirname "${0}")")
ROOT_DIR=${STATIC_ROOT_DIR:-"${SCRIPT_DIR}/../.."}

source "${ROOT_DIR}/scripts/python/venv-utils.sh"
activate_venv

set +e

EXIT_CODE=0

echo "Yamllint check"
python ./check_yaml_templates.py ../../plugins/ets/tests/ets-templates/ || EXIT_CODE=1

set -e

deactivate_venv
exit ${EXIT_CODE}
