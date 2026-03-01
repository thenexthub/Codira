
/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            options: "--ets-strings-concat=false --opt-level=1"

// Checker below based on plugins/ets/sdk/api/@ohos.buffer.ets
// Expected successful compilation

//! CHECKER       AOT IR Builder, Bugfix 30601
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*testStringify.*'"
//!
//! METHOD      "ets_stringbuilder_loop_branch.ETSGLOBAL::testStringify"
//! PASS_AFTER  "BranchElimination"
//! INST_COUNT  /NewObject/,3
//! INST_COUNT  /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /NewObject/,2
//! INST_COUNT  /Intrinsic.StdCoreSbToString/,1
//!
//! RUN         entry: "ets_stringbuilder_loop_branch.ETSGLOBAL::main", result: 0

function testStringify(): string {
    let base64 = 'abc';
    for (let i = 0; i < 2; i += 1) {
        if (1) {
            base64 += 'x';
        } else {
            base64 += 'y'.toString() + 'z'.toString();
        }
    }
    return base64;
}

function main() : int {
    arktest.assertEQ(testStringify(), "abcxx", 'Wrong result at testStringify()');
    return 0
}
