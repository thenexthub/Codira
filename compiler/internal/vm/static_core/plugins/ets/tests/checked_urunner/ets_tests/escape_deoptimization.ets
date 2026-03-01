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

class A {
    a: int;
    b: int;
};

let arr: FixedArray<int> = new FixedArray<int>(4)
function foo(obj: A, cnt: int): int  {
    obj.b = 2;
    return arr[cnt]
}

//! CHECKER       Escape isn't applied
//! RUN           force_jit: true, options: "--compiler-regex='.*main.*'", entry: "escape_deoptimization.ETSGLOBAL::main"
//! METHOD        "escape_deoptimization.ETSGLOBAL::main"
//! PASS_BEFORE   "ChecksElimination"
//! INST_NOT      "DeoptimizeIf"
//! INST_COUNT    "BoundsCheck", 11
//! PASS_BEFORE   "Escape"
//! INST_COUNT    "DeoptimizeIf", 1
//! INST_NOT      "BoundsCheck"
//! INST          "NewObject"
//! INST_COUNT    "NewArray", 2
//! PASS_AFTER    "Escape"
//! INST          "NewObject"
//! INST_COUNT    "NewArray", 2

//! CHECKER       AOT Escape isn't applied
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*main.*'"
//! METHOD        "escape_deoptimization.ETSGLOBAL::main"
//! PASS_BEFORE   "ChecksElimination"
//! INST_NOT      "DeoptimizeIf"
//! INST_COUNT    "BoundsCheck", 11
//! PASS_BEFORE   "Escape"
//! INST_COUNT    "DeoptimizeIf", 1
//! INST_NOT      "BoundsCheck"
//! INST          "NewObject"
//! INST_COUNT    "NewArray", 2
//! PASS_AFTER    "Escape"
//! INST          "NewObject"
//! INST_COUNT    "NewArray", 2
//! RUN           options: "", entry: "escape_deoptimization.ETSGLOBAL::main"

function __noinline__abort() {
    arktest.assertTrue(false);
}

// remove dependence from assert in stdlib
function cassert(v: boolean, s?: string) { if (!v) { throw new AssertionError(s) } }

function main() {
    let cnt: int = 0
    let a: FixedArray<int> = [ 42 ]
    try {
       let obj = new A();
       obj.a = 1;
       a = [foo(obj, cnt++), foo(obj, cnt++), foo(obj, cnt++), foo(obj, cnt++), foo(obj, cnt++)];
    } catch (e) {
        cassert(e instanceof ArrayIndexOutOfBoundsError, "Throwed not ArrayIndexOutOfBoundsError");
        cassert(a.length == 1);
        cassert(a[0] == 42);
        return
    }
    __noinline__abort();
}
