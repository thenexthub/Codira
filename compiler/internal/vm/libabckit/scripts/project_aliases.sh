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

ark-find-buildroot() {
    unset ARK_BUILDROOT
    if [[ ! "$PWD" =~ "out/x64" ]]; then
        echo "Current directory is not a child of build root, go to ARK_REPO/out/x64.{release,debug}"
        unset ARK_BUILDROOT
        return 1
    fi
    ARK_BUILDROOT=$(echo $PWD | sed "s/\(out\/[[:alnum:]\.]*\).*/\1/")
}

alias ark-built-disasm='ark-find-buildroot && $ARK_BUILDROOT/arkcompiler/runtime_core/ark_disasm'
alias ark-built-es2abc='ark-find-buildroot && $ARK_BUILDROOT/arkcompiler/ets_frontend/es2abc'

alias ark-ld-library-prefix='ark-find-buildroot && LD_LIBRARY_PATH=$ARK_BUILDROOT/arkcompiler/runtime_core:$ARK_BUILDROOT/arkcompiler/ets_runtime:$ARK_BUILDROOT/thirdparty/icu:$ARK_BUILDROOT/thirdparty/zlib'

alias abckit-run-gtests='ark-ld-library-prefix $ARK_BUILDROOT/tests/unittest/arkcompiler/runtime_core/libabckit/abckit_gtests'
