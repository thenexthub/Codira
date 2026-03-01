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

//! CHECKER      Test check initobj for string in compiler
//! RUN          force_jit: true, options: "--compiler-regex='.*ETSGLOBAL::main'", entry: "string_test.ETSGLOBAL::main"
//! EVENT        /Compilation,string_test.ETSGLOBAL::main,.*,COMPILED/
//! METHOD       "string_test.ETSGLOBAL::main"
//! PASS_AFTER   "IrBuilder"
//! INST         "InitEmptyString"
//! INST         "InitString FromCharArray"
//! INST_NOT     "InitString FromString"

function main(): void {
    let str1: String = new String();
    arktest.assertEQ(str1.getLength(), 0, "String is not empty");
    arktest.assertTrue(str1.isEmpty(), "String is not empty");

    let chars: FixedArray<char> = [c'a', c'B', c'C', c'd', c'e', c'g', c'X', c'Y', c'z'];
    let str2: String = new String(chars);  // "aBCdegXYz"
    arktest.assertEQ(str2.charAt(0), c'a', "Wrong str2.charAt(0)");
    arktest.assertEQ(str2.charAt(1), c'B', "Wrong str2.charAt(1)");
    arktest.assertEQ(str2.charAt(2), c'C', "Wrong str2.charAt(2)");
    arktest.assertEQ(str2.charAt(8), c'z', "Wrong str2.charAt(8)");
    arktest.assertNE(str2.charAt(8), c'Z');

    let str3: String = new String(str2); // "aBCdegXYz"
    arktest.assertEQ(str3.charAt(0), c'a', "Wrong str3.charAt(0)");
    arktest.assertEQ(str3.charAt(1), c'B', "Wrong str3.charAt(1)");
    arktest.assertEQ(str3.charAt(2), c'C', "Wrong str3.charAt(2)");
    arktest.assertEQ(str3.charAt(8), c'z', "Wrong str3.charAt(8)");
    arktest.assertNE(str3.charAt(8), c'Z');
}
