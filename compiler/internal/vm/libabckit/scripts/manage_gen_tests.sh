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

cur_path="$(pwd)"
cur_dir="$(basename $cur_path)"

if [ "$cur_dir" != "libabckit" ]; then
    echo "please rerun from runtime_core/libabckit"
    exit 1
fi

function clear_tests() {
    rm -rf tests/null_args_tests/null_args_tests_*
    rm -rf tests/wrong_ctx_tests/wrong_ctx_tests_*
    rm -rf tests/wrong_mode_tests/wrong_mode_tests_*
    rm -rf tests/wrong_imm_tests/wrong_imm_tests_*
}

function generate_tests() {
    ruby scripts/gen_wrong_ctx_tests.rb
    ruby scripts/gen_wrong_mode_tests.rb
    ruby scripts/gen_wrong_imm_tests.rb
    ruby scripts/gen_null_arg_tests.rb
}

if [[ "${1}" == "clear" ]]; then
    clear_tests
    if [[ "${2}" == "gen" ]]; then
        generate_tests
    fi
fi

if [[ "$1" == "gen" ]]; then
    generate_tests
    if [[ "$2" == "clear" ]]; then
        clear_tests
    fi
fi
