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

//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            options: "--ets-strings-concat=false"

//! CHECKER       std.core Array push
//! RUN           force_jit: true, options: "--compiler-regex=.*::test_array_push_.*", entry: "escompat_array_push.ETSGLOBAL::main"
//!
//! METHOD        "escompat_array_push.ETSGLOBAL::test_array_push"
//! PASS_AFTER    "Codegen"
//! INST          /CallStatic.*std.core.Array::push/
//! INST_COUNT    /CallStatic.*std.core.Array::push/, 5
//!
//! METHOD        "escompat_array_push.ETSGLOBAL::test_array_push_unions"
//! PASS_AFTER    "Codegen"
//! INST          /CallStatic.*std.core.Array::push/
//! INST_COUNT    /CallStatic.*std.core.Array::push/, 5
//!
//! METHOD        "escompat_array_push.ETSGLOBAL::test_array_push_one"
//! PASS_AFTER    "Codegen"
//! INST          /CallStatic.*std.core.Array::pushOne/
//! INST_COUNT    /CallStatic.*std.core.Array::pushOne/, 5
//!
//! METHOD        "escompat_array_push.ETSGLOBAL::test_array_push_array"
//! PASS_AFTER    "Codegen"
//! INST          /CallStatic.*std.core.Array::pushArray/
//! INST_COUNT    /CallStatic.*std.core.Array::pushArray/, 5

function test_array_push() {
    let array_size: int = 3;
    let array1: Array<number> = new Array<number>(array_size);

    array1.push(-1);
    arktest.assertEQ(array1[array1.length - 1], -1);

    array1.push(0);
    arktest.assertEQ(array1[array1.length - 1], 0);

    array1.push(1);
    arktest.assertEQ(array1[array1.length - 1], 1);

    array1.push(2);
    arktest.assertEQ(array1[array1.length - 1], 2);

    array1.push(1, 2, 3);
    arktest.assertEQ(array1[array1.length - 1], 3);

    arktest.assertEQ(array1.length, 10);
}

//! CHECKER       std.core Array push2
//! RUN           force_jit: true, options: "--compiler-regex='.*ETSGLOBAL::test_array_push_managed_objects' --compiler-if-conversion=false", entry: "escompat_array_push.ETSGLOBAL::test_array_push_managed_objects"
//! METHOD        "escompat_array_push.ETSGLOBAL::test_array_push_managed_objects"
//! PASS_AFTER    "Codegen"
//! INST          /CallStatic.*std.core.Array::push/
//! INST_COUNT    /CallStatic.*std.core.Array::push/, 5
function test_array_push_managed_objects() {
    let array: Array<Object> = new Array<Object>();

    // test number
    let obj1 = 123;
    array.push(obj1);
    arktest.assertEQ(array[array.length - 1], obj1);

    // test array
    let arr = [1, 2, 3];
    array.push(arr);
    arktest.assertEQ(array[array.length - 1], arr);

    // test string
    let str = "hello";
    array.push(str);
    arktest.assertEQ(array[array.length - 1], str);
    arktest.assertEQ(array.length, 3);

    let array1: Array<null> = new Array<null>();
    // test null and undefined
    array1.push(null);
    arktest.assertEQ(array1[array1.length - 1], null);

    let array2: Array<undefined> = new Array<undefined>();
    array2.push(undefined);
    arktest.assertEQ(array2[array2.length - 1], undefined);
}

function test_array_push_unions() {
    let array: Array<Object | null | undefined | number | string> =
        new Array<Object | null | undefined | number | string>();
    // test number
    array.push(123);
    arktest.assertEQ(array[array.length - 1], 123);

    // test string
    array.push("abc");
    arktest.assertEQ(array[array.length - 1], "abc");

    // test object
    let obj: Object = new Object()
    array.push(obj);
    arktest.assertEQ(array[array.length - 1], obj);

    // test null
    array.push(null);
    arktest.assertEQ(array[array.length - 1], null);

    // test undefined
    array.push(undefined);
    arktest.assertEQ(array[array.length - 1], undefined);

    arktest.assertEQ(array.length, 5);
}

function test_array_push_one() {
    let array: Array<Object | null | undefined | number | string> =
        new Array<Object | null | undefined | number | string>();
    // test number
    array.pushOne(123);
    arktest.assertEQ(array[array.length - 1], 123);

    // test string
    array.pushOne("abc");
    arktest.assertEQ(array[array.length - 1], "abc");

    // test object
    let obj: Object = new Object()
    array.pushOne(obj);
    arktest.assertEQ(array[array.length - 1], obj);

    // test null
    array.pushOne(null);
    arktest.assertEQ(array[array.length - 1], null);

    // test undefined
    array.pushOne(undefined);
    arktest.assertEQ(array[array.length - 1], undefined);

    arktest.assertEQ(array.length, 5);
}

function test_array_push_array() {
    let array: Array<Object | null | undefined | number | string> =
        new Array<Object | null | undefined | number | string>();
    // test number
    array.pushArray(1, 2, 3);
    arktest.assertEQ(array[array.length - 1], 3);

    // test string
    array.pushArray("a", "b", "c");
    arktest.assertEQ(array[array.length - 1], "c");

    // test object
    let obj1: Object = new Object()
    let obj2: Object = new Object()
    let obj3: Object = new Object()
    array.pushArray(obj1, obj2, obj3);
    arktest.assertEQ(array[array.length - 1], obj3);

    // test null
    array.pushArray(null, null, null);
    arktest.assertEQ(array[array.length - 1], null);

    // test undefined
    array.pushArray(undefined, undefined, undefined);
    arktest.assertEQ(array[array.length - 1], undefined);

    arktest.assertEQ(array.length, 15);
}

function main(): void {
    test_array_push()
    test_array_push_unions()
    test_array_push_one()
    test_array_push_array()
}
