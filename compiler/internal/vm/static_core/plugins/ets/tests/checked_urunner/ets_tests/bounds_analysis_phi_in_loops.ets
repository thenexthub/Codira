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

//! CHECKER      Check BoundsAnalysis work with phi functions in loop in AOT mode
//! SKIP_IF      @architecture == "arm32"
//! RUN_AOT      options: "--compiler-inlining=false", entry: "bounds_analysis_phi_in_loops.ETSGLOBAL::main"
//!
//! METHOD       "bounds_analysis_phi_in_loops.ETSGLOBAL::switch_case_loop"
//! PASS_BEFORE  "ChecksElimination"
//! INST         "ZeroCheck"
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     "ZeroCheck"
//!
//! METHOD       "bounds_analysis_phi_in_loops.ETSGLOBAL::simple_loop"
//! PASS_BEFORE  "ChecksElimination"
//! INST         "ZeroCheck"
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     "ZeroCheck"
//!
//! METHOD       "bounds_analysis_phi_in_loops.ETSGLOBAL::two_vars_loop"
//! PASS_BEFORE  "ChecksElimination"
//! INST         "ZeroCheck"
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     "ZeroCheck"
//!
//! METHOD       "bounds_analysis_phi_in_loops.ETSGLOBAL::nested_loops"
//! PASS_BEFORE  "ChecksElimination"
//! INST         "ZeroCheck"
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     "ZeroCheck"
//!
//! METHOD       "bounds_analysis_phi_in_loops.ETSGLOBAL::triple_nested_loops"
//! PASS_BEFORE  "ChecksElimination"
//! INST         "ZeroCheck"
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     "ZeroCheck"
//!
//! METHOD       "bounds_analysis_phi_in_loops.ETSGLOBAL::loop_init_without_update"
//! PASS_BEFORE  "ChecksElimination"
//! INST         "BoundsCheck"
//! PASS_AFTER   "ChecksElimination"
//! INST_COUNT   "BoundsCheck", 1
//!
//! METHOD       "bounds_analysis_phi_in_loops.ETSGLOBAL::different_operations"
//! PASS_BEFORE  "ChecksElimination"
//! INST_COUNT   "ZeroCheck", 2
//! PASS_AFTER   "ChecksElimination"
//! INST_COUNT   "ZeroCheck", 1
//!
//! METHOD       "bounds_analysis_phi_in_loops.ETSGLOBAL::operations_chain"
//! PASS_BEFORE  "ChecksElimination"
//! INST_COUNT   "ZeroCheck", 2
//! PASS_AFTER   "ChecksElimination"
//! INST_COUNT   "ZeroCheck", 1
//!
//! METHOD       "bounds_analysis_phi_in_loops.ETSGLOBAL::linked_vars"
//! PASS_BEFORE  "ChecksElimination"
//! INST_COUNT   "ZeroCheck", 3
//! PASS_AFTER   "ChecksElimination"
//! INST_COUNT   "ZeroCheck", 1
//!
//! METHOD       "bounds_analysis_phi_in_loops.ETSGLOBAL::not_countable_nested_loop"
//! PASS_BEFORE  "ChecksElimination"
//! INST_COUNT   "ZeroCheck", 1
//! PASS_AFTER   "ChecksElimination"
//! INST_COUNT   "ZeroCheck", 1


//! CHECKER      Check BoundsAnalysis work with phi functions in loop in JIT mode
//! SKIP_IF      @architecture == "arm32"
//! RUN          force_jit: true, options: "--compiler-inlining=false", entry: "bounds_analysis_phi_in_loops.ETSGLOBAL::main"
//!
//! METHOD       "bounds_analysis_phi_in_loops.ETSGLOBAL::switch_case_loop"
//! PASS_BEFORE  "ChecksElimination"
//! INST         "ZeroCheck"
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     "ZeroCheck"
//!
//! METHOD       "bounds_analysis_phi_in_loops.ETSGLOBAL::simple_loop"
//! PASS_BEFORE  "ChecksElimination"
//! INST         "ZeroCheck"
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     "ZeroCheck"
//!
//! METHOD       "bounds_analysis_phi_in_loops.ETSGLOBAL::two_vars_loop"
//! PASS_BEFORE  "ChecksElimination"
//! INST         "ZeroCheck"
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     "ZeroCheck"
//!
//! METHOD       "bounds_analysis_phi_in_loops.ETSGLOBAL::nested_loops"
//! PASS_BEFORE  "ChecksElimination"
//! INST         "ZeroCheck"
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     "ZeroCheck"
//!
//! METHOD       "bounds_analysis_phi_in_loops.ETSGLOBAL::triple_nested_loops"
//! PASS_BEFORE  "ChecksElimination"
//! INST         "ZeroCheck"
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     "ZeroCheck"
//!
//! METHOD       "bounds_analysis_phi_in_loops.ETSGLOBAL::loop_init_without_update"
//! PASS_BEFORE  "ChecksElimination"
//! INST         "BoundsCheck"
//! PASS_AFTER   "ChecksElimination"
//! INST_COUNT   "BoundsCheck", 1
//!
//! METHOD       "bounds_analysis_phi_in_loops.ETSGLOBAL::different_operations"
//! PASS_BEFORE  "ChecksElimination"
//! INST_COUNT   "ZeroCheck", 2
//! PASS_AFTER   "ChecksElimination"
//! INST_COUNT   "ZeroCheck", 1
//!
//! METHOD       "bounds_analysis_phi_in_loops.ETSGLOBAL::operations_chain"
//! PASS_BEFORE  "ChecksElimination"
//! INST_COUNT   "ZeroCheck", 2
//! PASS_AFTER   "ChecksElimination"
//! INST_COUNT   "ZeroCheck", 1
//!
//! METHOD       "bounds_analysis_phi_in_loops.ETSGLOBAL::linked_vars"
//! PASS_BEFORE  "ChecksElimination"
//! INST_COUNT   "ZeroCheck", 3
//! PASS_AFTER   "ChecksElimination"
//! INST_COUNT   "ZeroCheck", 1
//!
//! METHOD       "bounds_analysis_phi_in_loops.ETSGLOBAL::not_countable_nested_loop"
//! PASS_BEFORE  "ChecksElimination"
//! INST_COUNT   "ZeroCheck", 1
//! PASS_AFTER   "ChecksElimination"
//! INST_COUNT   "ZeroCheck", 1

function switch_case_loop() : number
{
    let arr : FixedArray<int> = [0, 1, 2, 3, 4, 4, 3, 2, 1, 0];
    let a : int = 1;
    for (let i = 0; i < arr.length; ++i) {
        switch (arr[i]) {
        case 0:
            a += 10;
            break;
        case 1:
            a += 5;
            break;
        case 2:
            a += 3;
            break;
        default:
            a += 1;
            break;
        }
    }
    return 41 / a; // no ZeroCheck
}

function simple_loop() : int
{
    let count : int = 10000000;
    let i3 : int = 1;
    let resources : FixedArray<int> = [12, 43, 56, 76, 89, 54, 45, 32, 35, 47, 46, 44, 21, 37, 84]
    let resourcesLength: int = resources.length;
  
    for (let i = 0; i < count; i++) {
        // no ZeroCheck
        if ((resources[i % i3 & (resourcesLength - 1)] & 1) == 0) {
            i3 += 1;
        } else {
            i3 += 2;
        }
    }
    return i3;
}

function two_vars_loop() : number
{
    let arr : FixedArray<int> = [0, 1, 2, 3, 4, 4, 3, 2, 1, 0];
    let a : int = 1;
    let b : int = 1;
    for (let i = 0; i < arr.length; ++i) {
        if (arr[i] <= 2) {
            a += 1;
            b += 2;
        } else {
            a += 2;
            b += 1;
        }
    }
    return 15 / a + 17 / b; // no ZeroCheck
}

function nested_loops() : number
{
    let matrix : FixedArray<FixedArray<int>> = [[0, 1, 2, 3],
                            [1, 2, 3, 4],
                            [2, 3, 4, 5],
                            [3, 4, 5, 6]];
    let val : int = 1;
    for (let i = 0; i < 4; ++i) {
        for (let j = 0; j < 4; ++j) {
            if (matrix[i][j] == 0) {
                val += 1;
            }
        }
    }
    return 2 / val; // no ZeroCheck
}

function triple_nested_loops() : number
{
    let matrix : FixedArray<FixedArray<int>> = [[0, 1, 2, 3],
                            [1, 2, 3, 4],
                            [2, 3, 4, 5],
                            [3, 4, 5, 6]];
    let global : int = 1;
    for (let i = 0; i < 4; ++i) {
        let parent : int = 2;
        if (i == 3) {
            global += 1;
        }
        for (let j = 0; j < 4; ++j) {
            let nested : int = parent;
            for (let k = 0; k < 4; ++k) {
                let local : int = 4;
                if (matrix[i][j] == 0) {
                    global += 1;
                    parent += 2;
                    nested += 3;
                    local  += 4;
                }
                console.log(local);
            }
            if (j == 2) {
                parent += 2;
            } else {
                parent += 1;
            }
            console.log(1 / nested); // no ZeroCheck
        }
        console.log(1 / parent); // no ZeroCheck
    }
    return 6 / global; // no ZeroCheck
}

function loop_init_without_update() : int
{
    let arr : FixedArray<int> = [0, 1, 2, 3, 4, 4, 3, 2, 1, 0];
    let a : int = 1;
    for (let i = 0; i < arr.length; ++i) {
        a += 2; // has no update phi -> cannot count boundaries
        try {
            console.log(arr[a]); // BoundsCheck cannot be deleted
        } catch (e) {
        }
    }
    return a;
}

function different_operations() : number
{
    let count : int = 10000000;
    let i3 : int = 1;
    let resources : FixedArray<int> = [12, 43, 56, 76, 89, 54, 45, 32, 35, 47, 46, 44, 21, 37, 84]
    let resourcesLength: int = resources.length;
  
    for (let i = 0; i < count; i++) {
        // ZeroCheck
        if ((resources[i % i3 & (resourcesLength - 1)] & 1) == 0) {
            i3 += 1; // Add
        } else {
            i3 /= 2; // Div -> analysis not applied
        }
    }
    return i3;
}

function operations_chain() : number
{
    let count : int = 10000000;
    let i3 : int = 1;
    let resources : FixedArray<int> = [12, 43, 56, 76, 89, 54, 45, 32, 35, 47, 46, 44, 21, 37, 84]
    let resourcesLength: int = resources.length;
  
    for (let i = 0; i < count; i++) {
        // ZeroCheck
        if ((resources[i % i3 & (resourcesLength - 1)] & 1) == 0) {
            i3 += 1; // Add
        } else {
            i3 = i3 / 2 + 1; // Div + Add -> analysis not applied
        }
    }
    return i3;
}

function linked_vars() : number
{
    let arr : FixedArray<int> = [0, 1, 2, 3, 4, 4, 3, 2, 1, 0];
    let a : int = 1;
    let b : int = 1;
    for (let i = 0; i < arr.length; ++i) {
        if (arr[i] <= 2) {
            a += b;
        } else {
            a += 1;
        }

        if (arr[i] % 2 == 0) {
            b -= 1;
        } else {
            b -= 2;
        }
    }
    return - 27 / a - 13 / b; // 1 ZeroCheck on a
}

function not_countable_nested_loop() : number
{
    let matrix : FixedArray<FixedArray<int>> = [[0, 1, 2, -1],
                            [1, 2, 3, -1],
                            [2, 3, 4, -1],
                            [3, 4, 5, -1]];
    let val : int = 1;
    let j : int = 0;
    for (let i = 0; i < 4; ++i) {
        while (true) {
            if (matrix[i][j] == 0) {
                val += 1;
            } else if (matrix[i][j] == -1) {
                break; // second backedge -> not countable loop
            }
            ++j;
        }
    }
    return 2 / val; // ZeroCheck
}

function main(): void {
    arktest.assertEQ(switch_case_loop(), 1);
    arktest.assertEQ(simple_loop(), 15000001);
    arktest.assertEQ(two_vars_loop(), 2);
    arktest.assertEQ(nested_loops(), 1);
    arktest.assertEQ(triple_nested_loops(), 1);
    arktest.assertEQ(loop_init_without_update(), 21);
    arktest.assertEQ(different_operations(), 9999997);
    arktest.assertEQ(operations_chain(), 9999998);
    arktest.assertEQ(linked_vars(), 2);
    arktest.assertEQ(not_countable_nested_loop(), 1);
}

