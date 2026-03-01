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

//! CHECKER       Escape is applied, temporary array must be removed.
//! RUN           force_jit: true, options: "--compiler-regex='.*test.*'", entry: "ets_escape_array.ETSGLOBAL::main"
//! METHOD        "ets_escape_array.ETSGLOBAL::test1"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    "NewArray", 1
//! PASS_AFTER    "Escape"
//! INST_COUNT    "NewArray", 1
//! EVENT         /Inline,ets_escape_array.ETSGLOBAL::test1,ets_escape_array.ETSGLOBAL::sum,.*,STATIC,SUCCESS/
//! METHOD        "ets_escape_array.ETSGLOBAL::test2"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    "NewArray", 1
//! INST_COUNT    "StoreArray", 4
//! INST_COUNT    "LoadArray", 3
//! PASS_AFTER    "Escape"
//! INST_COUNT    "NewArray", 1
//! INST_COUNT    "StoreArray", 4
//! INST_COUNT    "LoadArray", 3
//! METHOD        "ets_escape_array.ETSGLOBAL::test3"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    "NewArray", 1
//! PASS_AFTER    "Escape"
//! INST_COUNT    "NewArray", 1
//! METHOD        "ets_escape_array.ETSGLOBAL::test4"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    "NewArray", 2
//! PASS_AFTER    "Escape"
//! INST_COUNT    "NewArray", 1


function sum(...values: number[]): number {
    if (values.length == 0) {
        return 0;
    }
    let sum = values[0];
    for (let i = 1; i < values.length; i++) {
        sum += values[i];
    }
    return sum;
}

// Temporary must be removed, after inlining sum
function test1(): number {
    return sum(6, 5, 4, 3, 2, 1, 0);
}

let gValues: number[]; 

// Tmp array creation must be moved to if branch.
function test2(flag: boolean): number {
    let values = [1, 2, 3, 4];
    let sum = values[0];
    for (let i = 1; i < values.length; i++) {
        sum += values[i];
    }
    if (flag) {
        gValues = values;
    }
    return sum;
}

// Escape isn't apply, because array is used in catch block
function test3(): number {
    let arr = new number[1];
    try {
        arr[0] = 1;
        arr[1] = 2;
    } catch (e) {
        arktest.assertTrue(e instanceof ArrayIndexOutOfBoundsError, "Throwed not ArrayIndexOutOfBoundsError");
        arktest.assertEQ(arr[0], 1);
        return 0;
    }
    return 1;

}

function test4(): String {
    return "".concat("a", "B", "c", "D", "a", "B", "c", "D");
}

function test5(a: String, b: String, c: String, d: String, e: String, f: String, g: String, h: String): String {
    return "".concat(a, b, c, d, e, f, g, h);
}

function main() {
    arktest.assertEQ(test1(), 21, "test1 failed");
    arktest.assertEQ(test2(true), 10, "test2 failed");
    arktest.assertEQ(test3(), 0, "test3 failed");
    arktest.assertEQ(test4(), "aBcDaBcD", "test4 failed");
    arktest.assertEQ(test5("a", "B", "c", "D", "a", "B", "c", "D"), "aBcDaBcD", "test5 failed");
}
