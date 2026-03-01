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

//! CHECKER       AOT PGO: external inlining should not fold GetInstanceClass in EscapeAnalysis
//! SKIP_IF       @architecture == "arm32"
//! RUN_PGO_PROF  entry: "ext_inlining_inline_ic_check.ETSGLOBAL::main", options: "--compiler-regex='.*main.*'"
//! RUN_PGO_PAOC  options: "--compiler-inline-external-methods-aot=true --compiler-regex='.*__noinline__foo.*'"
//! METHOD        "ext_inlining_inline_ic_check.ETSGLOBAL::__noinline__foo"
//! PASS_AFTER    "Inline"
//! INST          /DeoptimizeIf INLINE_IC/
//! PASS_AFTER    "Peepholes"
//! INST_NOT      /LoadImmediate\(class: std\.core\.ArrayValuesIterator_T\)/
//! RUN           entry: "ext_inlining_inline_ic_check.ETSGLOBAL::main", options: "--compiler-regex='.*main.*'"
//! EVENT_NOT     /DeoptimizationReason,.*,INLINE_IC/

function __noinline__foo(x: int[]) {
    const y: int[] = [];
    for (let i = 0; i < 100; i++) {
        y.push(...x);
    }
    return y;
}

function main(): void {
    const res = __noinline__foo([1]);
    arktest.assertEQ(res.length, 100);
    for (let i = 0; i < res.length; i++) {
        arktest.assertEQ(res[i], 1);
    }
}
