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

//! CHECKER       std.core Array fill intrinsic
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--heap-size-limit=12345678 --compiler-hotness-threshold=0 --compiler-regex='.*escompat_array_fill.ETSGLOBAL::test_array.*'", entry: "escompat_array_fill.ETSGLOBAL::main"
//! METHOD        "escompat_array_fill.ETSGLOBAL::test_array_fill"
//! PASS_AFTER    "Codegen"
//! INST          /CallVirtual.*std.core.Array::fill/
//! INST_COUNT    /CallVirtual.*std.core.Array::fill/, 8
//! INST          /CallStatic.*std.core.Array::fill/
//! INST_COUNT    /CallStatic.*std.core.Array::fill/, 5
//!
//! METHOD        "escompat_array_fill.ETSGLOBAL::test_array_call_virtual_fill"
//! PASS_AFTER    "Codegen"
//! INST          /escompat_array_fill.MyArray::fill/
//! INST_COUNT    /escompat_array_fill.MyArray::fill/, 8

class TestClass {
    num: int;
    public constructor(num: int) {
        this.num = num;
    }
    public get(): int {
        return this.num;
    }
}

let array_size: int = 10;
let array1: Array<int> = new Array<int>(array_size);

function check_array(arr: Array<int>, start: int, end: int, expected: int) {
    for (let i = start; i < end; i++) {
        arktest.assertEQ(arr[i], expected);
    }
}

function fill_big_array() {
    // if have memory leak, will cause oom, then test failed
    let array2: Array<number> = new Array<number>(1024 * 768);
    array2.fill(1);
}

function test_array_fill() {
    array1.fill(99, 0, array_size);
    check_array(array1, 0, array_size, 99);

    array1.fill(11, array_size / 2 - array_size, array_size);
    check_array(array1, 0, array_size / 2, 99);
    check_array(array1, array_size / 2, array_size, 11);

    array1.fill(12, -(array_size + 2), array_size);
    check_array(array1, 0, array_size, 12);

    array1.fill(13, array_size / 2 - array_size, array_size + 3);
    check_array(array1, 0, array_size / 2, 12);
    check_array(array1, array_size / 2, array_size, 13);

    array1.fill(14, 0, array_size + 2);
    check_array(array1, 0, array_size, 14);

    arktest.assertEQ(array1.fill(15, 0, 1)[0], 15);
    arktest.assertEQ(array1.fill(15, 0, 1)[1], 14);
    arktest.assertEQ(array1.fill(16, 1, 2)[1], 16);

    let arrNull = new Array<null>(3);
    arrNull.fill(null, 0, 3);
    for (let i = 0; i < 3; i++) {
        arktest.assertEQ(arrNull[i], null);
    }
    let arrUndef = new Array<undefined>(5);
    arrUndef.fill(undefined, 0, 5)
    for (let i = 0; i < 5; i++) {
        arktest.assertEQ(arrUndef[i], undefined);
    }
    let arrUnion = new Array<TestClass | undefined>(5);
    arrUnion.fill(undefined, 0, 5)
    arrUnion.fill(new TestClass(2), 3, 5)
    for (let i = 0; i < 5; i++) {
        if (i < 3) {
            arktest.assertEQ(arrUndef[i], undefined);
            continue
        }
        arktest.assertEQ(arrUnion[i] instanceof TestClass, true);
        let tmpObj = arrUnion[i] as TestClass
        arktest.assertEQ(tmpObj.get(), 2);
    }
    let arrObj = new Array<TestClass>(50000);
    arrObj.fill(new TestClass(3), 0, 50000);
    for (let i = 0; i < 50000; i++) {
        arktest.assertEQ(arrObj[i].get(), 3);
    }

    for (let i = 0; i < 20; i++) {
        fill_big_array();
    }
}

class MyArray<T> extends Array<T> {
    counter: int;
    public constructor(len: number) {
        super(len.toInt());
        this.counter = 0;
    }

    public override fill(value: T, start?: int, end?: int): this {
        this.counter += 1;
        super.fill(value, start, end);
        return this;
    }

    public getCounter(): int {
        return this.counter;
    }
}

function test_array_call_virtual_fill() {
    let array2: MyArray<int> = new MyArray<int>(array_size);
    array2.fill(99, 0, array_size);
    check_array(array2, 0, array_size, 99);
    arktest.assertEQ(array2.getCounter(), 1);

    array2.fill(11, array_size / 2 - array_size, array_size);
    check_array(array2, 0, array_size / 2, 99);
    check_array(array2, array_size / 2, array_size, 11);
    arktest.assertEQ(array2.getCounter(), 2);

    array2.fill(12, -(array_size + 2), array_size);
    check_array(array2, 0, array_size, 12);
    arktest.assertEQ(array2.getCounter(), 3);

    array2.fill(13, array_size / 2 - array_size, array_size + 3);
    check_array(array2, 0, array_size / 2, 12);
    check_array(array2, array_size / 2, array_size, 13);
    arktest.assertEQ(array2.getCounter(), 4);

    array2.fill(14, 0, array_size + 2);
    check_array(array2, 0, array_size, 14);
    arktest.assertEQ(array2.getCounter(), 5);

    arktest.assertEQ(array2.fill(15, 0, 1)[0], 15);
    arktest.assertEQ(array2.getCounter(), 6);
    arktest.assertEQ(array2.fill(15, 0, 1)[1], 14);
    arktest.assertEQ(array2.getCounter(), 7);
    arktest.assertEQ(array2.fill(16, 1, 2)[1], 16);
    arktest.assertEQ(array2.getCounter(), 8);
}

function main() {
    test_array_fill();
    test_array_call_virtual_fill();
}
