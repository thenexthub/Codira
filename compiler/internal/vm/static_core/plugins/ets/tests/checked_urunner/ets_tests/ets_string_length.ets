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

//! CHECKER JIT IR Builder inlining String.getLength()
//! RUN     force_jit: true, options: "--compiler-regex='.*main.*'", entry: "ets_string_length.ETSGLOBAL::main"
//! METHOD  "ets_string_length.ETSGLOBAL::main"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /LenArray/
//! INST_NEXT  /Shr/
//! INST_NEXT  /Compare EQ i32/
//! INST_NEXT  /IfImm NE b/
//! PASS_AFTER "Codegen"
//! INST       /LenArray/
//! INST_NEXT  /ShrI/
//! INST_NEXT  /IfImm EQ i32/

//! CHECKER AOT IR Builder inlining String.getLength()
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC   options: "--compiler-regex='.*main.*'"
//! METHOD  "ets_string_length.ETSGLOBAL::main"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic/
//! INST       /LenArray/
//! INST_NEXT  /Shr/
//! INST_NEXT  /Compare EQ i32/
//! INST_NEXT  /IfImm NE b/
//! PASS_AFTER "Codegen"
//! INST       /LenArray/
//! INST_NEXT  /ShrI/
//! INST_NEXT  /IfImm EQ i32/
//! RUN        entry: "ets_string_length.ETSGLOBAL::main"

//! CHECKER String.length() with LLVMAOT
//! SKIP_IF    @architecture == "arm32"
//! RUN_LLVM   options: "--compiler-regex='.*main.*'"
//! RUN        entry: "ets_string_length.ETSGLOBAL::main"

function newString(): String {
    return new String("abcde");
}
function emptyString(): String {
    return new String();
}
function nullString(): String | null {
    return null;
}

function main(): int {
    let s = newString();
    if (s.getLength() != 5) {
        return 1;
    }
    let s1 = emptyString();
    if (s1.getLength() != 0) {
        return 2;
    }
    let s2 = nullString();
    try {
        // The program must fall into the catch block as s2.getLength() throws the NullPointerError.
        if (s2!.getLength() == 0) {
            return 3;
        }
        return 4;
    } catch (e) {
    }
    return 0;
}
