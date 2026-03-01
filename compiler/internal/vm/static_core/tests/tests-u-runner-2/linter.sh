#!/bin/bash
# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e

SCRIPT_DIR=$(realpath "$(dirname "${0}")")
ROOT_DIR=${STATIC_ROOT_DIR:-"${SCRIPT_DIR}/../.."}


function save_exit_code() {
    return $(($1 + $2))
}

source "${ROOT_DIR}/scripts/python/venv-utils.sh"
activate_venv

set +e

EXIT_CODE=0
RUNNER_DIR=${ROOT_DIR}/tests/tests-u-runner-2

cd "${RUNNER_DIR}"

echo "Pylint /runner, main.py"
pylint --rcfile .pylintrc runner main.py --ignore=test262,config_test,reports,runner_file_based.py
save_exit_code ${EXIT_CODE} $?
EXIT_CODE=$?

pylint --rcfile .pylintrc runner/extensions/generators/test262/ runner/test/config_test runner/reports runner/runner_file_based.py --disable=duplicate-code
save_exit_code ${EXIT_CODE} $?
EXIT_CODE=$?

echo "MyPy main.py"
mypy main.py
save_exit_code ${EXIT_CODE} $?
EXIT_CODE=$?

echo "MyPy /runner"
mypy -p runner
save_exit_code ${EXIT_CODE} $?
EXIT_CODE=$?

echo "Ruff check"
ruff check .
save_exit_code ${EXIT_CODE} $?
EXIT_CODE=$?

echo "Check complexity with radon"
flake8 --radon-max-cc 15 --select=R701 .
save_exit_code ${EXIT_CODE} $?
EXIT_CODE=$?

echo "Yamllint check"
python ../test-linters/check_yaml_templates.py ./cfg
save_exit_code ${EXIT_CODE} $?
EXIT_CODE=$?

set -e

deactivate_venv
exit ${EXIT_CODE}
