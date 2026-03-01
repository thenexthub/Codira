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

/*---
  files: [external_file_1.ets, external_file_2.ets, external_file_3.ets]
---*/

// Source file must be last in the compile list
//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            inputs: "external_file_1.ets", output: "inline_external_test_external_file_1.ets.abc"
//! RUN_FRONTEND            inputs: "external_file_2.ets", output: "inline_external_test_external_file_2.ets.abc"
//! RUN_FRONTEND            inputs: "external_file_3.ets", output: "inline_external_test_external_file_3.ets.abc"
//! RUN_FRONTEND            options: "--opt-level=2 --ets-strings-concat=false"


//! CHECKER      Check retrieving external method info during deoptimization
//! RUN_PAOC     options: "--panda-files=../inline_external_test_external_file_1.ets.abc:../inline_external_test_external_file_2.ets.abc:../inline_external_test_external_file_3.ets.abc --paoc-use-cha=true" # CC-OFF(G.FMT.02) special comment for testin
//! METHOD       "inline_external_test.ETSGLOBAL::main"
//! PASS_AFTER   "IrBuilder"
//! INST         /CallStatic.*B::array_test/
//! INST         /CallStatic.*C::array_test/
//! PASS_AFTER   "Inline"
//! INST         /CallStatic.Inlined.*B::array_test/
//! INST         /CallStatic.Inlined.*C::array_test/
//! RUN          options: "--panda-files=../inline_external_test_external_file_1.ets.abc:../inline_external_test_external_file_2.ets.abc:../inline_external_test_external_file_3.ets.abc", entry: "inline_external_test.ETSGLOBAL::main" # CC-OFF(G.FMT.02) special comment for testin
//! READ_FILE    "console.out"
//! INST         "RangeError: Out of bounds"

import { A } from './external_file_1';
import { B } from './external_file_2';
import { C } from './external_file_3';

function main(): int {
    let res : int = 0;
    let b : int[] = [1, 2];
    let c : int[] = [1, 2, 3, 4];
    try {
        res += B.array_test(b);
    } catch (e) {
        console.log(e);
    }
    try {
        res += C.array_test(c);
    } catch (e) {
        console.log(e);
    }
    return res;
}
