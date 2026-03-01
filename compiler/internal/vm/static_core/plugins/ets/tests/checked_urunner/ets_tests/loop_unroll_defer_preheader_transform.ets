/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

//! CHECKER         Check preheader's `Compare` isn't squashed by VN.
//! RUN             force_jit: true, options: "--compiler-enable-events --compiler-events-path=./events.csv --compiler-regex='.*::main'", entry: "loop_unroll_defer_preheader_transform.ETSGLOBAL::main"
//! METHOD          "loop_unroll_defer_preheader_transform.ETSGLOBAL::main"
//! PASS_AFTER      "LoopPeeling"
//! PASS_AFTER_NEXT "GVN"
//! INST            /Compare.* -> \(v[0-9]+\)/
//! INST_NEXT       /Compare.* -> \(v[0-9]+\)/
//! INST_NEXT       /Compare.* -> \(v[0-9]+\)/
//! INST_NEXT       /Compare.* -> \(v[0-9]+\)/
//! EVENT           /ETSGLOBAL::main,loop-unroll.*unroll_type:without branching/
//! EVENT           /ETSGLOBAL::main,loop-unroll.*unroll_type:without branching/

//! CHECKER         Check preheader's `Compare` is squashed if '--compiler-defer-preheader-transform' is disabled.
//! RUN             force_jit: true, options: "--compiler-defer-preheader-transform=false --compiler-regex='.*::main'", entry: "loop_unroll_defer_preheader_transform.ETSGLOBAL::main"
//! METHOD          "loop_unroll_defer_preheader_transform.ETSGLOBAL::main"
//! PASS_AFTER      "LoopPeeling"
//! PASS_AFTER_NEXT "GVN"
//! INST            /Compare.* -> \(v[0-9]+, v[0-9]+\)/

//! CHECKER         Check preheader's `Compare` is squashed if unroll is disabled.
//! RUN             force_jit: true, options: "--compiler-loop-unroll=false --compiler-regex='.*::main'", entry: "loop_unroll_defer_preheader_transform.ETSGLOBAL::main"
//! METHOD          "loop_unroll_defer_preheader_transform.ETSGLOBAL::main"
//! PASS_AFTER      "LoopPeeling"
//! PASS_AFTER_NEXT "GVN"
//! INST            /Compare.* -> \(v[0-9]+, v[0-9]+\)/


let RES = -6;

function main(): int {
    for (let i = 0; i < 3; i++) {
        RES += 1;
    }
    for (let i = 0; i < 3; i++) {
        RES += 1;
    }
    return RES;
}