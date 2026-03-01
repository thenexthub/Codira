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
//! RUN_FRONTEND            options: "--ets-strings-concat=false --bco-compiler --compiler-simplify-string-builder=false"

//! CHECKER     AOT: Verify StringBuilder hoisting optimization for string concatenation in loop
//! SKIP_IF     @architecture == "arm32"
//! RUN_PAOC    options: "--compiler-regex='.*will_hoist[0-9]+'"
//!
//! METHOD      "ets_string_builder_hoist.ETSGLOBAL::will_hoist1"
//! PASS_AFTER  "SimplifyStringBuilder"
//! IN_BLOCK    "prehead"
//! INST        /Load.*std\.core\.StringBuilder/
//! INST_NEXT   /NewObject/
//! INST_NEXT   /CallStatic \d+ std\.core\.StringBuilder::<ctor>/
//! INST_NOT    /Intrinsic\.StdCoreSbAppendString/


//! CHECKER     JIT: Verify StringBuilder hoisting optimization for string concatenation in loop
//! RUN         force_jit: true, options: "--compiler-regex='.*will_hoist[0-9]+'", entry: "ets_string_builder_hoist.ETSGLOBAL::main"
//!
//! METHOD      "ets_string_builder_hoist.ETSGLOBAL::will_hoist1"
//! PASS_AFTER  "SimplifyStringBuilder"
//! IN_BLOCK    "prehead"
//! INST        /LoadImmediate/
//! INST_NEXT   /NewObject/
//! INST_NEXT   /CallStatic \d+ std\.core\.StringBuilder::<ctor>/
//! INST_NOT    /Intrinsic\.StdCoreSbAppendString/

function will_hoist1() {
    let ret: string = "test:"
    for (let i = 0; i < 10; i++) {
        ret += String(i)
    }
    return ret
}


//! CHECKER     AOT: Verify StringBuilder hoisting is not performed in some case
//! SKIP_IF     @architecture == "arm32"
//! RUN_PAOC    options: "--compiler-regex='.*will_not_hoist[0-9]+'"
//!
//! METHOD      "ets_string_builder_hoist.ETSGLOBAL::will_not_hoist1"
//! PASS_AFTER  "SimplifyStringBuilder"
//! IN_BLOCK    "prehead"
//! INST_NOT    /LoadAndInitClass 'std\.core\.StringBuilder'/
//!
//! METHOD      "ets_string_builder_hoist.ETSGLOBAL::will_not_hoist2"
//! PASS_AFTER  "SimplifyStringBuilder"
//! IN_BLOCK    "prehead"
//! INST_NOT    /LoadAndInitClass 'std\.core\.StringBuilder'/


//! CHECKER     JIT: Verify StringBuilder hoisting is not performed in some case
//! RUN         force_jit: true, options: "--compiler-regex='.*will_not_hoist[0-9]+'", entry: "ets_string_builder_hoist.ETSGLOBAL::main"
//!
//! METHOD      "ets_string_builder_hoist.ETSGLOBAL::will_not_hoist1"
//! PASS_AFTER  "SimplifyStringBuilder"
//! IN_BLOCK    "prehead"
//! INST_NOT    /LoadAndInitClass 'std\.core\.StringBuilder'/
//!
//! METHOD      "ets_string_builder_hoist.ETSGLOBAL::will_not_hoist2"
//! PASS_AFTER  "SimplifyStringBuilder"
//! IN_BLOCK    "prehead"
//! INST_NOT    /LoadAndInitClass 'std\.core\.StringBuilder'/

function will_not_hoist1() {
    let ret: string = ""
    let acc: string = "test:"
    for (let i = 0; i < 10; i++, acc += String(i)) {
        ret = acc
    }
    return ret
}

function will_not_hoist2() {
    let ret: string = ""
    let acc: string = "test:"
    for (let i = 0; i < 10; acc += String(i), i++) {
        ret = acc
    }
    return ret
}

function main() {
    arktest.assertEQ(will_hoist1(), "test:0123456789")
    arktest.assertEQ(will_not_hoist1(), "test:123456789")
    arktest.assertEQ(will_not_hoist2(), "test:012345678")
}

//! CHECKER     BCO: Verify StringBuilder hoisting optimizations
//! RUN_BCO     options: "--ets-strings-concat=false --bco-compiler --compiler-simplify-string-builder=true"
//!
//! METHOD      "ets_string_builder_hoist.ETSGLOBAL::will_hoist1"
//! PASS_AFTER  "SimplifyStringBuilder"
//! IN_BLOCK    "prehead"
//! INST        /Load.*std\.core\.StringBuilder/
//! INST_NEXT   /NewObject/
//! INST_NEXT   /CallStatic \d+ std\.core\.StringBuilder::<ctor>/
//! INST_NOT   /StringBuilder::append/
//!
//! METHOD      "ets_string_builder_hoist.ETSGLOBAL::will_not_hoist1"
//! PASS_AFTER  "SimplifyStringBuilder"
//! IN_BLOCK    "prehead"
//! INST_NOT    /LoadAndInitClass 'std\.core\.StringBuilder'/
//!
//! METHOD      "ets_string_builder_hoist.ETSGLOBAL::will_not_hoist2"
//! PASS_AFTER  "SimplifyStringBuilder"
//! IN_BLOCK    "prehead"
//! INST_NOT    /LoadAndInitClass 'std\.core\.StringBuilder'/
