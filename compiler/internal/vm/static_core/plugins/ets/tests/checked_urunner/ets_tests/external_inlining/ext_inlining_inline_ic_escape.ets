/*
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

//! CHECKER       AOT PGO: EscapeAnalysis should not fold GetInstanceClass with external inlining
//! SKIP_IF       @architecture == "arm32"
//! RUN_PGO_PROF  entry: "ext_inlining_inline_ic_escape.ETSGLOBAL::main"
//! RUN_PGO_PAOC  options: "--compiler-inline-external-methods-aot=true --compiler-peepholes=false --compiler-regex='.*__noinline__sum_iter.*'"
//! METHOD        "ext_inlining_inline_ic_escape.ETSGLOBAL::__noinline__sum_iter"
//! PASS_AFTER    "Inline"
//! INST          /DeoptimizeIf INLINE_IC/
//! INST          /GetInstanceClass/
//! PASS_BEFORE   "EscapeAnalysis"
//! INST          /GetInstanceClass/
//! INST_NOT      /LoadImmediate\(class: std\.core\.ArrayValuesIterator_T\)/
//! PASS_AFTER    "EscapeAnalysis"
//! INST          /GetInstanceClass/
//! INST_NOT      /LoadImmediate\(class: std\.core\.ArrayValuesIterator_T\)/
//! RUN           entry: "ext_inlining_inline_ic_escape.ETSGLOBAL::main", options: "--compiler-regex='.*main.*'"
//! EVENT_NOT     /DeoptimizationReason,.*,INLINE_IC/

function __noinline__sum_iter(arr: int[]): int {
    let s = 0;
    for (let x of arr) {
        s += x;
    }
    return s;
}

function main(): void {
    let arr: int[] = [1, 2, 3, 4];
    let res = __noinline__sum_iter(arr);
    arktest.assertEQ(res, 10);
}
