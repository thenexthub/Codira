/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

//! CHECKER       JIT IR Builder, check String.indexOf(char) function
//! SKIP_IF       @architecture == "arm32" || @architecture == "amd64"
//! RUN           force_jit: true, options: "--compiler-regex='.*main.*'", entry: "ets_string_indexof.ETSGLOBAL::main"
//! METHOD        "ets_string_indexof.ETSGLOBAL::main"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /Intrinsic\.StdCoreStringIndexOf\s/,1
//! INST_COUNT    /Intrinsic\.StdCoreStringIndexOfAfter/,1
//! ASM_METHOD     "ets_string_indexof.ETSGLOBAL::main"
//! ASM_INST      /Intrinsic\.StdCoreStringIndexOf/
//! ASM_INST      /Intrinsic\.StdCoreStringIndexOfAfter/

let u8_line = new string('abcdefghijklmnopqrstuvwxyz');

function main() {
    arktest.assertEQ(u8_line.indexOf(c'a'), 0);
    arktest.assertEQ(u8_line.indexOf(c'a', 5), -1);
}
