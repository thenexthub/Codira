/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
//! RUN_FRONTEND            options: "--ets-strings-concat=false"

//! CHECKER      Test charAt intrinsic
//! RUN          force_jit: true, options: "--compiler-regex=charAt.*", entry: "charAt.ETSGLOBAL::main"
//! METHOD       "charAt.ETSGLOBAL::main"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "Call.*std.core.String::charAt"
//! INST         "Intrinsic.StdCoreStringCharAt"

function main(): void {
    let str: String = new String("aBCd");
    arktest.assertEQ(str.charAt(0), c'a', "Wrong str.charAt(0)");
    arktest.assertEQ(str.charAt(1), c'B', "Wrong str.charAt(1)");
    arktest.assertEQ(str.charAt(2), c'C', "Wrong str.charAt(2)");
    arktest.assertEQ(str.charAt(3), c'd', "Wrong str.charAt(3)");
}

//! CHECKER      Test charAt intrinsic with tree string
//! RUN          force_jit: true, options: "--use-all-strings=true --compiler-regex=charAt.*", entry: "charAt.ETSGLOBAL::testTreeStringCharAt"
//! METHOD       "charAt.ETSGLOBAL::testTreeStringCharAt"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "Call.*std.core.String::charAt"
//! INST_COUNT   "Intrinsic.StdCoreStringCharAt", 4
//! PASS_AFTER   "StringFlatCheck"
//! INST_COUNT   "StringFlatCheck", 1
function testTreeStringCharAt(): void {
    let str = createTreeString("123456789", "abcdefghi")
    arktest.assertEQ(str.charAt(0), c'1', "Wrong str.charAt(0)")
    arktest.assertEQ(str.charAt(8), c'9', "Wrong str.charAt(8)")
    arktest.assertEQ(str.charAt(9), c'a', "Wrong str.charAt(9)")
    arktest.assertEQ(str.charAt(17), c'i', "Wrong str.charAt(17)")
}

function createTreeString(s1: string, s2: string): string {
    return s1 + s2
}

//! CHECKER      Test charAt intrinsic with sliced string
//! RUN          force_jit: true, options: "--use-all-strings=true --compiler-regex=charAt.*", entry: "charAt.ETSGLOBAL::testSlicedStringCharAt"
//! METHOD       "charAt.ETSGLOBAL::testSlicedStringCharAt"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "Call.*std.core.String::charAt"
//! INST_COUNT   "Intrinsic.StdCoreStringCharAt", 2
//! PASS_AFTER   "StringFlatCheck"
//! INST_COUNT   "StringFlatCheck", 1
function testSlicedStringCharAt(): void {
    let s0 = "abcdefghi"
    let str = __noinline__createSlicedString(s0, s0.length * 100 + 3, 256 * 1024)
    arktest.assertEQ(str.charAt(0), c'd', "Wrong str.charAt(0)")
    arktest.assertEQ(str.charAt(5), c'i', "Wrong str.charAt(str.length - 1)")
}

function __noinline__createSlicedString(s: string, start: int, minLength: int): string {
    // allocate long string to go to the slowpath since currently fastpath does not create sliced string
    while (s.length < minLength) {
        s += s
    }
    return s.substring(start)
}
