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

//! CHECKER       TypedArray of()
//! RUN           force_jit: true, options: "--compiler-regex=.*test.*ArrayOf.*", entry: "ets_escompat_typed_arrays_of_part4.ETSGLOBAL::checkTypedArrayOf"
//! METHOD        "ets_escompat_typed_arrays_of_part4.ETSGLOBAL::testUint32ArrayOfNumber"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Uint32ArrayOfNumber"
//! METHOD        "ets_escompat_typed_arrays_of_part4.ETSGLOBAL::testUint32ArrayOfInt"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Uint32ArrayOfInt"
//! METHOD        "ets_escompat_typed_arrays_of_part4.ETSGLOBAL::testUint32ArrayOfLong"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Uint32ArrayOfLong"
//! METHOD        "ets_escompat_typed_arrays_of_part4.ETSGLOBAL::testBigUint64ArrayOfInt"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.BigUint64ArrayOfInt"
//! METHOD        "ets_escompat_typed_arrays_of_part4.ETSGLOBAL::testBigUint64ArrayOfLong"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.BigUint64ArrayOfLong"

function checkTypedArrayOf() {
    testUint32ArrayOfNumber()
    testUint32ArrayOfInt()
    testUint32ArrayOfLong()

    testBigUint64ArrayOfNumber()
    testBigUint64ArrayOfInt()
    testBigUint64ArrayOfLong()
}

function testUint32ArrayOfNumber() {
    let expected: FixedArray<number> = [12, 6, 332, 42, 33, 0, 0, 0]
    let arr = Uint32Array.of(12.0, 6.0, 332.0, 42.0, 33.0, NaN, Infinity, -Infinity)
    checkArray(arr, expected)
}

function testUint32ArrayOfInt() {
    let expected: FixedArray<number> = [12, 6, 332, 42, 33]
    let arr = Uint32Array.of(12, 6, 332, 42, 33)
    checkArray(arr, expected)
}

function testUint32ArrayOfLong() {
    let expected: FixedArray<number> = [12, 6, 332, 42, 33]
    let arr = Uint32Array.of((12).toLong(), (6).toLong(), (332).toLong(), (42).toLong(), (33).toLong())
    checkArray(arr, expected)
}

function testBigUint64ArrayOfNumber() {
    let expected: FixedArray<number> = [12, 6, 332, 42, 33]
    let arr = BigUint64Array.of(12.0, 6.0, 332.0, 42.0, 33.0)
    checkArray(arr, expected)
}

function testBigUint64ArrayOfInt() {
    let expected: FixedArray<number> = [12, 6, 332, 42, 33]
    let arr = BigUint64Array.of(12, 6, 332, 42, 33)
    checkArray(arr, expected)
}

function testBigUint64ArrayOfLong() {
    let expected: FixedArray<number> = [12, 6, 332, 42, 33]
    let arr = BigUint64Array.of((12).toLong(), (6).toLong(), (332).toLong(), (42).toLong(), (33).toLong())
    checkArray(arr, expected)
}

function checkArray(arr: ArrayLike<number>, expected: FixedArray<number>) {
    arktest.assertEQ(arr.length, expected.length, "Unexpected array length")
    for (let i = 0; i < expected.length; i++) {
        arktest.assertEQ(arr[i], expected[i], "Error in index: " + i + ", value: " + arr[i] + " expected value: " + expected[i])
    }
}

function checkArray(arr: BigInt64Array, expected: FixedArray<number>) {
    arktest.assertEQ(arr.length, expected.length, "Unexpected array length")
    for (let i = 0; i < expected.length; i++) {
        arktest.assertEQ(arr[i].getLong(), expected[i], "Error in index: " + i + ", value: " + arr[i] + " expected value: " + expected[i])
    }
}

function checkArray(arr: BigUint64Array, expected: FixedArray<number>) {
    arktest.assertEQ(arr.length, expected.length, "Unexpected array length")
    for (let i = 0; i < expected.length; i++) {
        arktest.assertEQ(arr[i].getLong(), expected[i], "Error in index: " + i + ", value: " + arr[i] + " expected value: " + expected[i])
    }
}
