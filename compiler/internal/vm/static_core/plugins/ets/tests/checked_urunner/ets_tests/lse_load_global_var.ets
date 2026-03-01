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

//! CHECKER            Check that LSE and AA work correctly with global variables
//! SKIP_IF            @architecture == "arm32"
//! RUN_PAOC           options: "--compiler-regex='.*main.*'"
//!
//! METHOD             "lse_load_global_var.ETSGLOBAL::main"
//! PASS_AFTER         "IrBuilder"
//! INST_COUNT         /LoadStatic/, 2
//! INST_COUNT         /LoadStatic.*globalVar/, 2
//! PASS_BEFORE_NTH    "LSE", 1
//! INST_COUNT         /LoadStatic/, 2
//! PASS_AFTER_NTH     "LSE", 1
//! INST_COUNT         /LoadStatic/, 1
//!
//! RUN                entry: "lse_load_global_var.ETSGLOBAL::main"

let globalVar = '=';

function main(): int {
    let s = 'aa';
    // Here we expect single LoadStatic instruction, the LSE pass should have removed the second one.
    s = s + (globalVar + globalVar);
    arktest.assertEQ(s, 'aa==');
    return 0;
}
