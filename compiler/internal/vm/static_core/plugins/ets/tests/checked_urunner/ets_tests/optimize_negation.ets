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

//! CHECKER         Test check optimize bool Negation
//! RUN             force_jit: true, options: "--compiler-regex='.*func.*' --compiler-inlining=false", entry: "optimize_negation.ETSGLOBAL::main"
//! EVENT           /Compilation,optimize_negation.ETSGLOBAL::func,.*,COMPILED/
//! METHOD          "optimize_negation.ETSGLOBAL::func"
//!
//! PASS_BEFORE     "Peepholes"
//! INST            "Xor "
//! INST_NOT        "Compare"
//!
//! PASS_AFTER      "Peepholes"
//! INST            "Xor "
//! INST            "Compare"
//!
//! PASS_BEFORE     "Lowering"
//! INST            "Compare"
//! INST_NOT        "Xor "
//! INST_NOT        "XorI"
//!
//! PASS_AFTER      "Lowering"
//! INST            "Xor "
//! INST            "Compare"
//! INST            "XorI"
//!
//! PASS_AFTER_NEXT "Cleanup"
//! INST            "XorI"
//! INST_NOT        "Xor "
//! INST_NOT        "Compare"

function get_bool() : boolean {
    return false
}

function func() : boolean {
    let a : boolean = get_bool()
    return !a
}

function main() : void {
    func()
}
