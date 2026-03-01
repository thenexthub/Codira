#!/bin/bash
# Copyright (c) 2024 Huawei Device Co., Ltd.
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

set -u
set -e
set -o pipefail

ENV_VALUE=""
SCRIPT_ARGS_VALUE=""
RUNNER_VALUE=""
RUNNER_ARGS_VALUE=""
RET_CODE_VALUE=0

SCRIPT_ARGUMENT="--script"
SCRIPT_ARGS_OPTION="--script-args"
RUNNER_OPTION="--runner"
RUNNER_ARGS_OPTION="--runner-args"
ENV_OPTION="--env"
RET_CODE_OPTION="--ret-code"
HELP_OPTION="--help"
DEBUG_OPTION="--debug"

declare -i TRUE=1
declare -i FALSE=0

declare -i HELP_VALUE=$FALSE
declare -i DEBUG_VALUE=$FALSE

print_help() {
    echo "Usage: run_script.sh ${SCRIPT_ARGUMENT}=                                                        "
    echo "      [${SCRIPT_ARGS_OPTION}=]+ [${RUNNER_ARGS_OPTION}=]+ [${ENV_OPTION}=]+                     "
    echo "      [${RUNNER_OPTION}=] [${RET_CODE_OPTION}=] [${HELP_OPTION}] [${DEBUG_OPTION}]              "
    echo "Arguments:                                                                                      "
    echo "      ${SCRIPT_ARGUMENT}      Path for the script to be executed.                               "
    echo "Options:                                                                                        "
    echo "      ${SCRIPT_ARGS_OPTION}   Arguments for the script.                                         "
    echo "      ${RUNNER_OPTION}        Specify command that shall execute the script.                    "
    echo "      ${RUNNER_ARGS_OPTION}   Specify arguments for the script runner.                          "
    echo "      ${ENV_OPTION}           Environment for the script execution.                             "
    echo "      ${RET_CODE_OPTION}      Specify expected return code of the command. Set to 0 by default. "
    echo "      ${HELP_OPTION}          Print this message.                                               "
    echo "      ${DEBUG_OPTION}         Print debug information.                                          "
}

report_error() {
    echo "[ERROR]/: " "$@"
    exit 1
}

debug_message() {
    if [ ${DEBUG_VALUE} -eq ${TRUE} ]; then
        echo "[DEBUG]/: " "$@"
    fi
}

parse_args() {
    for i in "$@"; do
        case "$i" in
        "${SCRIPT_ARGUMENT}"=*)
            SCRIPT_VALUE="${i#*=}"
            shift
            ;;
        "${SCRIPT_ARGS_OPTION}"=*)
            # CC-OFFNXT(bc-50008) false positive
            SCRIPT_ARGS_VALUE="${SCRIPT_ARGS_VALUE} ${i#*=}"
            shift
            ;;
        "${RUNNER_OPTION}"=*)
            # CC-OFFNXT(bc-50008) false positive
            RUNNER_VALUE="${i#*=}"
            shift
            ;;
        "${RET_CODE_OPTION}"=*)
            # CC-OFFNXT(bc-50008) false positive
            RET_CODE_VALUE="${i#*=}"
            shift
            ;;
        "${RUNNER_ARGS_OPTION}"=*)
            # CC-OFFNXT(bc-50008) false positive
            RUNNER_ARGS_VALUE="${RUNNER_ARGS_VALUE} ${i#*=}"
            shift
            ;;
        "${ENV_OPTION}"=*)
            # CC-OFFNXT(bc-50008) false positive
            ENV_VALUE="${ENV_VALUE} ${i#*=}"
            shift
            ;;
        "${HELP_OPTION}")
            # CC-OFFNXT(bc-50008) false positive
            HELP_VALUE=$TRUE
            shift
            ;;
        "${DEBUG_OPTION}")
            # CC-OFFNXT(bc-50008) false positive
            DEBUG_VALUE=$TRUE
            shift
            ;;
        *)
            print_help
            report_error "Recieved unexpected argument: ${i}"
            ;;
        esac
    done
}

check_args() {
    if [ -z "$SCRIPT_VALUE" ]; then
        report_error "${SCRIPT_ARGUMENT} argument can not be empty."
    fi

    if [[ -z "${RUNNER_VALUE}" && -n "${RUNNER_ARGS_VALUE}" ]]; then
        report_error "You must specify ${RUNNER_OPTION} for ${RUNNER_ARGS_OPTION} to be passed to."
    fi
}

print_parsed_args() {
    debug_message "Parsed arguments:"
    debug_message "${SCRIPT_ARGUMENT}=${SCRIPT_VALUE}"
    debug_message "${SCRIPT_ARGS_OPTION}=${SCRIPT_ARGS_VALUE}"
    debug_message "${RUNNER_OPTION}=${RUNNER_VALUE}"
    debug_message "${RUNNER_ARGS_OPTION}=${RUNNER_ARGS_VALUE}"
    debug_message "${ENV_OPTION}=${ENV_VALUE}"
    debug_message "${RET_CODE_OPTION}=${RET_CODE_VALUE}"
    debug_message "${HELP_OPTION}=${HELP_VALUE}"
}

main() {
    parse_args "$@"
    print_parsed_args

    if [ ${HELP_VALUE} -eq ${TRUE} ]; then
        print_help
        exit 0
    fi

    check_args

    local -i ret_code=0
    if [ -n "${ENV_VALUE}" ]; then
        export ${ENV_VALUE?}
    fi
    ${RUNNER_VALUE} ${RUNNER_ARGS_VALUE} ${SCRIPT_VALUE} ${SCRIPT_ARGS_VALUE} || ret_code=$?

    if [ ! ${ret_code} -eq ${RET_CODE_VALUE} ]; then
        report_error "Unexpected error code returned! Expected: ${RET_CODE_VALUE} vs actual: ${ret_code}"
    fi
}

main "$@"
