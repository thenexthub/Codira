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

//! CHECKER       Hoist loop invariant BoundsCheck
//! RUN           force_jit: true, options: "--compiler-regex='.*test.*'", entry: "hoist_loop_inv_bounds_check.ETSGLOBAL::main"
//! METHOD       "hoist_loop_inv_bounds_check.ETSGLOBAL::test"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "BoundsCheck", 2
//! PASS_AFTER   "ChecksElimination"
//! INST_COUNT   "BoundsCheck", 1
//! PASS_BEFORE  "Codegen"
//! # Check that IR have only one instruction "BoundsCheck D", and no one other "BoundsCheck"
//! INST_COUNT   "BoundsCheck", 1
//! INST_COUNT   "BoundsCheck D", 1

function test(arr: FixedArray<int>, idx: int) : int {
    let sum: int = 0;
    for (let i:int = 0; i < arr.length; ++i) {
        sum += arr[i] * arr[idx];
    }
    return sum;
}

function main(): int {
    let arr: FixedArray<int> = [0, 1, 2, 3, 4, 5];
    return test(arr, 0);
}
