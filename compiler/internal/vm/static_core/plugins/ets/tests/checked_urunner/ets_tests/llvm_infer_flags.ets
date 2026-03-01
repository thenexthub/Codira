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

// InferFlags should set nsw flag to "i0 += ?" addition, and as a consequence eliminate branch with ArithmeticError
//! CHECKER        Infer nsw flag for i0 phi
//! RUN_LLVM       options: "--llvm-options=--print-after=infer-flags --compiler-regex='.*simple'"
//! READ_FILE      "console.out"
//! LLVM_METHOD    /define protected.*simple/
//! INST_NEXT      /%v19 = add nsw i32 %v\d+, %v19.v/

//! CHECKER          No ArithmeticError
//! RUN_LLVM         options: "--llvm-dump-after --compiler-regex='.*simple'"
//! READ_FILE        "console.out"
//! LLVM_METHOD      /.*simple.*/
//! INST_NEXT_NOT    /.*ArithmeticError.*/

function condition(i: int, i0: int): boolean {
    return i % i0 == 0
}

function simple(): int {
    let i0 = 1
    for (let i = 0; i < 1000000; i++) {
        if (condition(i, i0)) {
            i0 += 2
        } else {
            i0 += 1
        }
    }
    return i0
}

//! CHECKER        Will throw
//! RUN_LLVM       options: "--llvm-options=--print-after=runtime-calls-lowering --compiler-regex='.*willThrow.*'"
//! READ_FILE      "console.out"
//! LLVM_METHOD    /.*willThrow.*/
//! RUN            entry: "llvm_infer_flags.ETSGLOBAL::willThrow"

function willThrow(): int {
    let i0 = 1
    const i1 = 0
    try {
	    let result : int = i0 / i1
    } catch(e) {
        if (e instanceof ArithmeticError) {
            return 0
        } else {
            throw e
        }
    }
    return i0
}

//! CHECKER        Overflows
//! RUN_LLVM       options: "--llvm-options=--print-after=infer-flags --compiler-regex='.*overflows.*'"
//! READ_FILE      "console.out"
//! LLVM_METHOD    /.*overflows.*/
//! RUN            entry: "llvm_infer_flags.ETSGLOBAL::overflows"
function overflows(): int {
    try {
        let i0: int = 2147483647
        for (let i: int = 0; i < 1000; i++) {
            if (condition(i, i0)) {
                i0 += 2147483647
            } else {
                i0 += 2
            }
        }
    } catch(e) {
        if (e instanceof ArithmeticError) {
            return 0
        } else {
            throw e
        }
    }
    return 1
}
