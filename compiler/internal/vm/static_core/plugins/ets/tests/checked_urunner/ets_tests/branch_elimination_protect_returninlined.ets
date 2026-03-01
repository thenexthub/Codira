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

//! CHECKER          Block with ReturnInlined inst with an infinite loop is preserved
//! SKIP_IF          @architecture == "arm32"
//! RUN_PAOC         options: "--compiler-inline-external-methods-aot=true --compiler-regex=.*inlined_inf_loop"
//! METHOD           "branch_elimination_protect_returninlined.ETSGLOBAL::inlined_inf_loop"
//! PASS_AFTER       "Inline"
//! INST             /Call.*Inlined.*inf_loop/
//! INST             "ReturnInlined"
//! INST_COUNT       "ReturnInlined", 1
//! PASS_BEFORE_NTH  "BranchElimination", 3
//! INST             /Call.*Inlined.*inf_loop/
//! INST             "ReturnInlined"
//! INST_COUNT       "ReturnInlined", 1
//! PASS_AFTER_NTH   "BranchElimination", 3
//! INST             /Call.*Inlined.*inf_loop/
//! INST             "ReturnInlined"
//! INST_COUNT       "ReturnInlined", 1
//! PASS_AFTER       "Codegen"
//! INST             /Call.*Inlined.*inf_loop/
//! INST             "ReturnInlined"
//! INST_COUNT       "ReturnInlined", 1

function inf_loop(): int {
    for (let i = 0; i < 5; i = i + 1) {
        break
    }
    let x = 0
    while (x < 5) {
        x = x + 1
        continue
    }
    a: do {
        'app.string.delete_show'
    } while (x > 0)

    switch (x) {
        case 1:
            break
    }
    return 1
}

// Check that a function with an infinite loop gets inlined
// and the ReturnInlined block is not removed by BranchElimination.
function inlined_inf_loop(): void {
    inf_loop()
}

//! CHECKER          Block with ReturnInlined inst with some processing after infinite loop
//! SKIP_IF          @architecture == "arm32"
//! RUN_PAOC         options: "--compiler-regex=.*complex_processing_after_inf_call"
//! METHOD           "branch_elimination_protect_returninlined.ETSGLOBAL::complex_processing_after_inf_call"
//! PASS_AFTER       "Inline"
//! INST             /Call.*Inlined.*inf_loop/
//! INST             "ReturnInlined"
//! INST_COUNT       "ReturnInlined", 1
//! PASS_BEFORE_NTH  "BranchElimination", 3
//! INST             /Call.*Inlined.*inf_loop/
//! INST             "ReturnInlined"
//! INST_COUNT       "ReturnInlined", 1
//! PASS_AFTER_NTH   "BranchElimination", 3
//! INST             /Call.*Inlined.*inf_loop/
//! INST             "ReturnInlined"
//! INST_COUNT       "ReturnInlined", 1
//! PASS_AFTER       "Codegen"
//! INST             /Call.*Inlined.*inf_loop/
//! INST             "ReturnInlined"
//! INST_COUNT       "ReturnInlined", 1

function complex_processing_after_inf_call(): int {
    inf_loop()
    let x = 0;
    while (true) {
        x = x + 1;
        if (x > 1000) break;
        if (x % 7 == 0) continue;
        if (x == 123) return x;
    }
    return -1;
}

//! CHECKER          Block with ReturnInlined inst with an complex infinite loop is preserved
//! SKIP_IF          @architecture == "arm32"
//! RUN_PAOC         options: "--compiler-regex=.*complex_inlined_inf_loop"
//! METHOD           "branch_elimination_protect_returninlined.ETSGLOBAL::complex_inlined_inf_loop"
//! PASS_AFTER       "Inline"
//! INST             /Call.*Inlined.*complex_inf_loop/
//! INST             "ReturnInlined"
//! INST_COUNT       "ReturnInlined", 1
//! PASS_BEFORE_NTH  "BranchElimination", 3
//! INST             /Call.*Inlined.*complex_inf_loop/
//! INST             "ReturnInlined"
//! INST_COUNT       "ReturnInlined", 1
//! PASS_AFTER_NTH   "BranchElimination", 3
//! INST             /Call.*Inlined.*complex_inf_loop/
//! INST             "ReturnInlined"
//! INST_COUNT       "ReturnInlined", 1
//! PASS_AFTER       "Codegen"
//! INST             /Call.*Inlined.*complex_inf_loop/
//! INST             "ReturnInlined"
//! INST_COUNT       "ReturnInlined", 1

function complex_inf_loop(): int {
    for (let i = 0; i < 5; i = i + 1) {
        break
    }
    let x = 0
    while (x < 5) {
        x = x + 1
        continue
    }
    a: do {
        'app.string.delete_show'
    } while (x > 0)

    switch (x % 3) {
        case 0:
            x = 3;
            break;
        case 1:
            x = x * 2;
            break;
        default:
            x = 7;
            break;
    }
    return x
}

function complex_inlined_inf_loop(): int {
    let result = complex_inf_loop()
    if (result > 0) {
        for (let k = result; k > 0; k = k - 1) {
            if (k % 10 == 0) {
                result = result * 2;
            } else if (k % 3 == 0) {
                continue;
            } else {
                break;
            }
        }
    } else {
        do {
            result = result + 1;
        } while (result < 0);
    }
    return result
}

//! CHECKER          Check proper elimination of block with Throw and ReturnInlined instructions
//! SKIP_IF          @architecture == "arm32"
//! RUN_PAOC         options: "--compiler-inline-external-methods-aot=true --compiler-regex=.*test_eliminate_throw"
//! METHOD           "branch_elimination_protect_returninlined.ETSGLOBAL::test_eliminate_throw"
//! PASS_AFTER       "Inline"
//! INST             /Call.*Inlined.*eliminate_throw/
//! INST_COUNT       "CallStatic.Inlined", 7
//! INST_COUNT       "ReturnInlined", 10
//! INST_COUNT       "Throw", 2
//! PASS_BEFORE_NTH  "BranchElimination", 2
//! INST             /Call.*Inlined.*eliminate_throw/
//! INST             /LoadClass.*escompat.Error/
//! INST_NEXT        "ReturnInlined"
//! INST_NEXT        "Throw"
//! INST_NEXT        "Intrinsic.StdCoreRuntimeFailedTypeCastExclUndefinedStub"
//! INST_NEXT        "ReturnInlined"
//! INST_NEXT        "Throw"
//! INST_COUNT       "Throw", 2
//! INST_COUNT       "ReturnInlined", 10
//! PASS_AFTER_NTH   "BranchElimination", 2
//! INST             /Call.*Inlined.*eliminate_throw/
//! INST_NEXT        "ReturnInlined"
//! INST_NEXT        "ReturnVoid"
//! INST_COUNT       "Throw", 0
//! INST_COUNT       "ReturnInlined", 1

function always_false(): boolean {
    return false;
}

function eliminate_throw(): int {
    // A condition that the compiler can definitively evaluate.
    // Statically known to be false.
    if (always_false()) {
        // This block SHOULD be removed by BE:
        const error = (): Error => {
            // Throw.
            // ReturnInlined.
            return new Error("This will be eliminated");
        }();
        throw error;
    }
    return 42
}

// Checking that the unreachable block with ReturnInlined and Throw instructions
// (as determined by BranchElimination) is properly eliminated.
function test_eliminate_throw(): void {
    eliminate_throw()
}
