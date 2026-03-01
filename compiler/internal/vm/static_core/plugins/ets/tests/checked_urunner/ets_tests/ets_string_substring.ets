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

let original: String = new String("abcde");

//! CHECKER       Equal Substring JIT
//! RUN           force_jit: true, options: "--compiler-regex='.*main.*'", entry: "ets_string_substring.ETSGLOBAL::main"
//! METHOD        "ets_string_substring.ETSGLOBAL::main"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.StdCoreStringSubstring"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Intrinsic.StdCoreStringSubstring"


//! CHECKER       Equal Substring  AOT
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*main.*'"
//! METHOD        "ets_string_substring.ETSGLOBAL::main"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.StdCoreStringSubstring"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Intrinsic.StdCoreStringSubstring"
//! RUN           entry: "ets_string_substring.ETSGLOBAL::main"

//! CHECKER       Equal Substring INT
//! RUN           options: "--compiler-enable-jit=false", entry: "ets_string_substring.ETSGLOBAL::main"


function __noinline__SubString(str: String, begin: int, end: int): String {
    return str.substring(begin, end);
}
function main(): int {
    let subs: String = __noinline__SubString(original, 0, original.getLength());
    if(original !== subs) return 1;
    let subs1: String = __noinline__SubString(original, -1, 100);
    if(original !== subs1) return 2;
    let subs2: String = original.substring(0, original.getLength());
    if(original !== subs2) return 3;
    return 0
}
