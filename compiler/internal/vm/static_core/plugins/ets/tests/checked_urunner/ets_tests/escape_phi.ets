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
    valA_: int;

    constructor() {
        this.valA_ = 0
    }

    constructor(valA: int) {
        this.valA_ = valA
    }
}

class B extends A {
    valB_: int;

    constructor(valA: int, valB: int) {
        super(valA);
        this.valB_ = valB;
    }
}

function __noinline__modify(obj: A) {
    obj.valA_ = 100500
}

function __noinline__modify(arr: int[]) {
    arr[0] = 100500
}

function __noinline__getLength(flag: boolean): int {
    return 3 + (flag ? 1 : 0);
}

//! CHECKER       AOT Escape eliminate phi with objects
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-inline-external-methods-aot=true --compiler-regex='.*test.*'"
//! METHOD        "ETSGLOBAL::test_simple_phi1"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! PASS_AFTER    "Escape"
//! INST_NOT      /NewObject/
//! INST_NOT      /StoreObject/
//! INST_NOT      /LoadObject/
//!
//! METHOD        "ETSGLOBAL::test_simple_phi2"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 1
//! INST_COUNT    /StoreObject/, 1
//! INST_COUNT    /LoadObject/, 1
//!
//! METHOD        "ETSGLOBAL::test_with_slowpath"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 2
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 1
//! INST_COUNT    /StoreObject/, 1
//! INST_COUNT    /LoadObject/, 1
//!
//! METHOD        "ETSGLOBAL::test_different_classes"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 3
//! INST_COUNT    /LoadObject/, 1
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 3
//! INST_COUNT    /LoadObject/, 1
//!
//! METHOD        "ETSGLOBAL::test_loop"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 7
//! INST_COUNT    /StoreObject/, 7
//! INST_COUNT    /LoadObject/, 0
//! INST_COUNT    /ref.*Phi/, 0
//! INST_COUNT    /i32.*Phi/, 3
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 1
//! INST_COUNT    /StoreObject/, 1
//! INST_COUNT    /LoadObject/, 0
//! INST_COUNT    /ref.*Phi/, 0
//! INST_COUNT    /i32.*Phi/, 3
//!
//! METHOD        "ETSGLOBAL::test_dominate_objects"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//!
//! METHOD        "ETSGLOBAL::test_dominate_objects_materialize"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//!
//! METHOD        "ETSGLOBAL::test_dominate_objects_materialize_in_slowpath"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 2
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 2
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//!
//! METHOD        "ETSGLOBAL::test_dominate_objects_separate_obj_usage"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//!
//! METHOD        "ETSGLOBAL::test_temp_arrays"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 9
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 9
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//!
//! METHOD        "ETSGLOBAL::test_temp_arrays_diff_length1"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 14
//! INST_COUNT    /LoadArray/, 11
//! INST_COUNT    /ref.*Phi/, 1
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 14
//! INST_COUNT    /LoadArray/, 11
//! INST_COUNT    /ref.*Phi/, 1
//!
//! METHOD        "ETSGLOBAL::test_temp_arrays_diff_length2"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewArray/, 2
//! INST_COUNT    /StoreArray/, 20
//! INST_COUNT    /LoadArray/, 4
//! INST_COUNT    /ref.*Phi/, 5
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewArray/, 2
//! INST_COUNT    /StoreArray/, 20
//! INST_COUNT    /LoadArray/, 4
//! INST_COUNT    /ref.*Phi/, 5
//!
//! METHOD        "ETSGLOBAL::test_arrays_materialize"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 9
//! INST_COUNT    /ref.*Phi/, 1
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 9
//! INST_COUNT    /ref.*Phi/, 1
//!
//! METHOD        "ETSGLOBAL::test_arrays_materialize_in_slowpath"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 12
//! INST_COUNT    /ref.*Phi/, 1
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 12
//! INST_COUNT    /ref.*Phi/, 1

//! CHECKER       JIT Escape eliminate phi with objects
//! RUN           force_jit: true, options: "--compiler-regex='.*test.*'", entry: "escape_phi.ETSGLOBAL::main"
//! METHOD        "ETSGLOBAL::test_simple_phi1"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! PASS_AFTER    "Escape"
//! INST_NOT      /NewObject/
//! INST_NOT      /StoreObject/
//! INST_NOT      /LoadObject/
//!
//! METHOD        "ETSGLOBAL::test_simple_phi2"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 1
//! INST_COUNT    /StoreObject/, 1
//! INST_COUNT    /LoadObject/, 1
//!
//! METHOD        "ETSGLOBAL::test_with_slowpath"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 2
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 1
//! INST_COUNT    /StoreObject/, 1
//! INST_COUNT    /LoadObject/, 1
//!
//! METHOD        "ETSGLOBAL::test_different_classes"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 3
//! INST_COUNT    /LoadObject/, 1
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 3
//! INST_COUNT    /LoadObject/, 1
//!
//! METHOD        "ETSGLOBAL::test_loop"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 7
//! INST_COUNT    /StoreObject/, 7
//! INST_COUNT    /LoadObject/, 0
//! INST_COUNT    /ref.*Phi/, 0
//! INST_COUNT    /i32.*Phi/, 3
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 1
//! INST_COUNT    /StoreObject/, 1
//! INST_COUNT    /LoadObject/, 0
//! INST_COUNT    /ref.*Phi/, 0
//! INST_COUNT    /i32.*Phi/, 3
//!
//! METHOD        "ETSGLOBAL::test_dominate_objects"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//!
//! METHOD        "ETSGLOBAL::test_dominate_objects_materialize"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//!
//! METHOD        "ETSGLOBAL::test_dominate_objects_materialize_in_slowpath"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 2
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 2
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//!
//! METHOD        "ETSGLOBAL::test_dominate_objects_separate_obj_usage"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewObject/, 2
//! INST_COUNT    /StoreObject/, 2
//! INST_COUNT    /LoadObject/, 1
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//!
//! METHOD        "ETSGLOBAL::test_temp_arrays"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 9
//! INST_COUNT    /ref.*Phi/, 1
//! INST_NOT      /i32.*Phi/
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 9
//! INST_COUNT    /ref.*Phi/, 1
//! INST_COUNT    /i32.*Phi/, 0
//!
//! METHOD        "ETSGLOBAL::test_temp_arrays_diff_length1"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 14
//! INST_COUNT    /LoadArray/, 11
//! INST_COUNT    /ref.*Phi/, 1
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 14
//! INST_COUNT    /LoadArray/, 11
//! INST_COUNT    /ref.*Phi/, 1
//!
//! METHOD        "ETSGLOBAL::test_temp_arrays_diff_length2"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewArray/, 2
//! INST_COUNT    /StoreArray/, 24
//! INST_COUNT    /LoadArray/, 4
//! INST_COUNT    /ref.*Phi/, 5
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewArray/, 2
//! INST_COUNT    /StoreArray/, 24
//! INST_COUNT    /LoadArray/, 4
//! INST_COUNT    /ref.*Phi/, 5
//!
//! METHOD        "ETSGLOBAL::test_arrays_materialize"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 9
//! INST_COUNT    /ref.*Phi/, 1
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 9
//! INST_COUNT    /ref.*Phi/, 1
//!
//! METHOD        "ETSGLOBAL::test_arrays_materialize_in_slowpath"
//! PASS_BEFORE   "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 12
//! INST_COUNT    /ref.*Phi/, 1
//! PASS_AFTER    "Escape"
//! INST_COUNT    /NewArray/, 4
//! INST_COUNT    /StoreArray/, 12
//! INST_COUNT    /LoadArray/, 12
//! INST_COUNT    /ref.*Phi/, 1


// apply, objects are temporary
function test_simple_phi1(a: int, b: int): int {
    let obj: A;
    if (a > b) {
        obj = new A(a);
    } else {
        obj = new A(b);
    }
    return obj.valA_;
}

// no apply, object escapes
function test_simple_phi2(a: int, b: int): int {
    let obj: A;
    if (a > b) {
        obj = new A(a);
    } else {
        obj = new A(b);
    }
    __noinline__modify(obj);
    return obj.valA_;
}

// apply, materialize into slowpath
function test_with_slowpath(a: int, b: int, flag: boolean): int {
    let obj: A;
    if (a > b) {
        obj = new A(a);
    } else {
        obj = new A(b);
    }
    if (flag) {
        __noinline__modify(obj);
        return obj.valA_;
    }
    return obj.valA_;
}

// no apply, different classes
function test_different_classes(a: int, b: int): int {
    let obj: A;
    if (a > b) {
        obj = new A(a);
    } else {
        obj = new B(b, a);
    }
    return obj.valA_;
}

// apply, objects are temporary
function test_loop(N: int) : int {
    let a = new A(0);
    while (a.valA_ != N) {
        a = new A(a.valA_ + 1);
    }
    return a.valA_;
}

// no apply
function test_dominate_objects(flag: boolean): int {
    let a1 = new A(0);
    let a2 = new A(1);
    let a: A;
    if (flag) {
        a = a1;
    } else {
        a = a2;
    }
    return a.valA_;
}

// no apply, objects are temporary, 
function test_dominate_objects_separate_obj_usage(flag: boolean): int {
    let a1 = new A(0);
    let a2 = new A(1);
    let a: A;
    if (flag) {
        a = a1;
    } else {
        a = a2;
    }
    a2.valA_ = 3;
    return a.valA_;
}

// no apply
function test_dominate_objects_materialize(flag: boolean): int {
    let a1 = new A(0);
    let a2 = new A(1);
    let a: A;
    if (flag) {
        a = a1;
    } else {
        a = a2;
    }
    __noinline__modify(a);
    return a.valA_;
}

// no apply
function test_dominate_objects_materialize_in_slowpath(flag: boolean, slowpath: boolean): int {
    let a1 = new A(0);
    let a2 = new A(1);
    let a: A;
    if (flag) {
        a = a1;
    } else {
        a = a2;
    }
    if (slowpath) {
        __noinline__modify(a);
        return a.valA_;
    }
    return a.valA_;
}

// apply
function test_temp_arrays(flag: boolean): int {
    let arr: int[];
    if (flag) {
        arr = [0, 1, 2];
    } else {
        arr = [3, 4, 5];
    }
    return arr[0] + arr[1] + arr[2];
}

// no apply, different length
function test_temp_arrays_diff_length1(flag: boolean): int {
    let arr: int[];
    if (flag) {
        arr = [0, 1, 2, 4];
    } else {
        arr = [3, 4, 5];
    }
    return arr[0] + arr[1] + arr[2] + arr[3];
}

// no apply, different length
function test_temp_arrays_diff_length2(flag: boolean): int {
    let length = __noinline__getLength(flag);
    let arr: int[];
    if (flag) {
        arr = new int[length];
        for (let i: int = 0; i < length; ++i) {
            arr[i] = i +(flag ? 1 : 0);
        }
    } else {
        arr = new int[length];
        for (let i: int = 0; i < length; ++i) {
            arr[i] = i;
        }
    }
    return arr[0] + arr[1] + arr[2] + arr[3];
}

// no apply
function test_arrays_materialize(flag: boolean): int {
    let arr: int[];
    if (flag) {
        arr = [0, 1, 2];
    } else {
        arr = [3, 4, 5];
    }
    __noinline__modify(arr);
    return arr[0] + arr[1] + arr[2];
}

// apply
function test_arrays_materialize_in_slowpath(flag1: boolean, flag2: boolean): int {
    let arr: int[];
    if (flag1) {
        arr = [0, 1, 2];
    } else {
        arr = [3, 4, 5];
    }
    if (flag2) {
        __noinline__modify(arr);
        return arr[0] + arr[1] + arr[2];
    }
    return arr[0] + arr[1] + arr[2];
}

function main(): int {
    if (test_simple_phi1(1, 2) != 2) return 1;
    if (test_simple_phi1(2, 1) != 2) return 2;
    if (test_simple_phi2(1, 2) != 100500) return 3;
    if (test_simple_phi2(2, 1) != 100500) return 4;
    if (test_with_slowpath(1, 2, false) != 2) return 5;
    if (test_with_slowpath(2, 1, false) != 2) return 6;
    if (test_with_slowpath(2, 1, true) != 100500) return 7;
    if (test_different_classes(1, 2) != 2) return 8;
    if (test_different_classes(2, 1) != 2) return 9;
    if (test_loop(10) != 10) return 10;
    if (test_dominate_objects(true) != 0) return 11;
    if (test_dominate_objects(false) != 1) return 12;
    if (test_dominate_objects_materialize(true) != 100500) return 13;
    if (test_dominate_objects_materialize(false) != 100500) return 14;
    if (test_dominate_objects_materialize_in_slowpath(false, false) != 1) return 15;
    if (test_dominate_objects_materialize_in_slowpath(true, false) != 0) return 16;
    if (test_dominate_objects_materialize_in_slowpath(false, true) != 100500) return 17;
    if (test_dominate_objects_materialize_in_slowpath(true, true) != 100500) return 18;
    if (test_temp_arrays(true) != 3) return 19;
    if (test_temp_arrays(false) != 12) return 20;
    if (test_temp_arrays_diff_length1(true) != 7) return 21;
    try {
        test_temp_arrays_diff_length1(false);
        return 22;
    } catch (e) {}
    if (test_temp_arrays_diff_length2(true) != 10) return 23;
    try {
        test_temp_arrays_diff_length2(false);
        return 24;
    } catch (e) {}
    if (test_arrays_materialize(true) != 100503) return 25;
    if (test_arrays_materialize(false) != 100509) return 26;
    if (test_arrays_materialize_in_slowpath(false, false) != 12) return 27;
    if (test_arrays_materialize_in_slowpath(true, false) != 3) return 28;
    if (test_arrays_materialize_in_slowpath(false, true) != 100509) return 29;
    if (test_arrays_materialize_in_slowpath(true, true) != 100503) return 30;
    if (test_dominate_objects_separate_obj_usage(true) != 0) return 31;
    if (test_dominate_objects_separate_obj_usage(false) != 3) return 32;
    return 0;
}
