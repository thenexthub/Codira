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

//! CHECKER     Test StringGet inlining in compiler
//! RUN         force_jit: true, options: "--compiler-regex='.*main.*'", entry: "ets_string_get.ETSGLOBAL::main"
//! METHOD      "ets_string_get.ETSGLOBAL::main"
//! PASS_AFTER  "IrBuilder"
//! INST        "Intrinsic.StdCoreStringGet"

//! CHECKER     AOT IR Builder StringGet get inlining.
//! RUN_PAOC    options: "--compiler-regex='.*main.*'"
//! METHOD      "ets_string_get.ETSGLOBAL::main"
//! PASS_AFTER  "IrBuilder"
//! INST        /Intrinsic.StdCoreStringGet/

function main(): void {
    let str = "abcdefg";
    arktest.assertEQ(str[0], "a", "Wrong str.get(0)");
    arktest.assertEQ(str[6], "g", "Wrong str.get(6)");

    let errorFlag = 0;
    try {
        console.log(str[-1]);
    } catch (e) {
        if (e instanceof StringIndexOutOfBoundsError) {
            errorFlag = 1;
        } else {
            errorFlag = 2;
        }
    }
    arktest.assertEQ(1, errorFlag);

    errorFlag = 0;
    try {
        console.log(str[7]);
    } catch (e) {
        if (e instanceof StringIndexOutOfBoundsError) {
            errorFlag = 1;
        } else {
            errorFlag = 2;
        }
    }
    arktest.assertEQ(1, errorFlag);
}
