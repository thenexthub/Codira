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

//! CHECKER                 ets.istrue JIT
//! RUN                     force_jit: true, options: "--compiler-regex='.*check_istrue.*'", entry: "ets_istrue.ETSGLOBAL::main"
//! CHECK_GROUP             CheckGroup1

//! CHECKER                 ets.istrue AOT
//! RUN_PAOC                options: "--compiler-regex='.*check_istrue.*'"
//! CHECK_GROUP             CheckGroup1
//! RUN                     entry: "ets_istrue.ETSGLOBAL::main"

//! DISABLED_CHECKER        ets.istrue LLVM AOT
//! RUN_LLVM                options: "--compiler-regex='.*check_istrue.*'"
//! RUN                     entry: "ets_istrue.ETSGLOBAL::main"

//! CHECK_GROUP             CheckGroup1
//! METHOD                  "ets_istrue.ETSGLOBAL::check_istrue"
//! PASS_AFTER              "Codegen"
//! INST_COUNT              /Intrinsic.CompilerEtsIstrue/, 1

function check_istrue(v: Any) { return !!v }

function erase<T>(v: T): Any {
    return v;
}

function main(): int {
    if (check_istrue(erase(new Byte(0 as byte)))) return 1;
    if (!check_istrue(erase(new Byte(1 as byte)))) return 2;
    if (check_istrue(erase(new Short(0 as short)))) return 3;
    if (!check_istrue(erase(new Short(1 as short)))) return 4;
    if (check_istrue(erase(new Int(0)))) return 5;
    if (!check_istrue(erase(new Int(1)))) return 6;
    if (check_istrue(erase(new Long(0)))) return 7;
    if (!check_istrue(erase(new Long(1)))) return 8;
    if (check_istrue(erase(new Float(0.0)))) return 9;
    if (!check_istrue(erase(new Float(0.5)))) return 10;
    if (check_istrue(erase(new Double(0.0)))) return 11;
    if (!check_istrue(erase(new Double(0.5)))) return 12;
    
    return 0;
}
