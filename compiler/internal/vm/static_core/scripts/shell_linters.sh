#!/usr/bin/env bash
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

set -e

if [[ -z "$1" ]]; then
    echo "Usage: $0 <sources>"
    echo "    <sources> path where the sources to be checked are located"
    echo
    echo "    The script launches shellcheck and bashate tools against all *sh"
    echo "    scripts found in the specified folder."
    echo "    For bashate, all rules are checked by default except E006 and E042"
    echo "    One can set BASHATE_IGNORE_RULES variable to skip oher rules."
    echo "    For shellcheck, a prefedined setof rules is checked (see comments in the script)."
    echo "    One can set SHELLCHECK_INCLLUDE_RULES variable to check custom set of rules"
    exit 1
fi

root_dir=$(realpath "$1")
# For shellcheck, we only enable certain rules
# - SC1068 - no spaces  round +=/=
# - SC1106 - correctness of string/arithmetic comparisons
# - SC1133 - special chars at the beginning of the line
# - SC2002 - useless cat
# - SC2003 - use of antique expr
# - SC2006 - for $(...) usage instead of obsolete ``
# - SC2010 - avoid 'ls | grep'
# - SC2024 - sudo redirection
# - SC2034 - unused variables
# - SC2041 - usage of strings instead of commands
# - SC2045 - iterating over ls
# - SC2064 - quotation in signal trap
# - SC2066 - stream merge order
# - SC2067 - find-exec usage
# - SC2081 - square/single brackets usage
# - SC2086 - Double quotes for vaiables
# - SC2088 - Tilde usage
# - SC2093 - exec usage
# - SC2115 - safe var usage in rm
# - SC2142 - aliass can't have parameters
# - SC2144 - '-e' and globs
# - SC2148 - requires shebang to be present
# - SC2152 - func return value between 0 and 255
# - SC2164 - Safe cd
# - SC2166 - avoid [ p -a q ]
# - SC2172 - do not trap by number
# - SC2173 - do not trap SIGKILL/SIGSTOP
# - SC2222 - exit code between 0 and 255
# - SC2253 - Safe chmod
SHELLCHECK_RULES=${SHELLCHECK_INCLLUDE_RULES:-"SC1068,SC1106,SC1133,\
SC2002,SC2003,SC2006,SC2010,SC2024,SC2034,SC2041,SC2045,SC2064,SC2066,\
SC2067,SC2081,SC2086,SC2088,SC2093,SC2115,SC2142,SC2144,SC2148,SC2152,\
SC2164,SC2166,SC2172,SC2173,SC2222,SC2253"}

# For bashate, we enable all rules and only exclude several ones
# - E006 is 'Line Too Long' which should be additionally adjusted for us
# - E042 is 'local definition hides errors'
BASHATE_RULES=${BASHATE_IGNORE_RULES:-"E006,E042"}

function save_exit_code() {
    return $(($1 + $2))
}

set +e

EXIT_CODE=0

skip_options="^${root_dir}/third_party/"
if [ ! -z "$SKIP_FOLDERS" ]; then
    for pt in $SKIP_FOLDERS; do
        skip_options="${skip_options}\|^${root_dir}/${pt}/"
    done
fi

while read file_to_check; do
    bashate-mod-ds -i "${BASHATE_RULES}" "${file_to_check}"
    save_exit_code ${EXIT_CODE} $?
    EXIT_CODE=$?
    shellcheck -i "${SHELLCHECK_RULES}" "${file_to_check}"
    save_exit_code ${EXIT_CODE} $?
    EXIT_CODE=$?
done <<<$(find "${root_dir}" -name "*.sh" -type f | grep -v "${skip_options}")

num_checked=$(find "${root_dir}" -name "*.sh" -type f | grep -c -v "${skip_options}")
echo "Checked ${num_checked} files"

exit ${EXIT_CODE}
