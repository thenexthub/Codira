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

//! CHECKER       LSE optimize the summury of two elements from array
//! RUN           force_jit: true, options: "--compiler-regex='.*main1.*'", entry: "ets_sum_elem_array.ETSGLOBAL::main1", result: 5
//! EVENT         /Compilation,ets_sum_elem_array.ETSGLOBAL::main1,.*,COMPILED/
//! METHOD        "ets_sum_elem_array.ETSGLOBAL::main"
//! PASS_BEFORE   "LSE"
//! INST_COUNT    /LoadArray/, 2
//! INST_COUNT    /StoreArray/, 2
//! PASS_AFTER    "LSE"
//! INST_COUNT    /LoadArray/, 0
//! INST_COUNT    /StoreArray/, 2
//! INST_COUNT    /Cast i64/, 2

//! CHECKER       LSE optimize the summury of two elements from object
//! RUN           force_jit: true, options: "--compiler-regex='.*main2.*'", entry: "ets_sum_elem_array.ETSGLOBAL::main2", result: 3
//! EVENT         /Compilation,ets_sum_elem_array.ETSGLOBAL::main2,.*,COMPILED/
//! METHOD        "ets_sum_elem_array.ETSGLOBAL::main"
//! PASS_BEFORE   "LSE"
//! INST_COUNT    /LoadObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! PASS_AFTER    "LSE"
//! INST_COUNT    /LoadObject/, 0
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /Cast i64/, 2

function sum2(a: FixedArray<int>): int {
    return a[0] + a[1];
}

function main1(): int {
    let a: FixedArray<int> = new FixedArray<int>(2);
    a[0] = 2;
    a[1] = 3;
    return sum2(a);
}

class Base {
    public a: int
    public b: int
}

function sumObj(obj: Base): int {
    return obj.a + obj.b;
}

function main2(): int {
    let o = new Base();
    o.a = 1;
    o.b = 2;
    return sumObj(o);
}
