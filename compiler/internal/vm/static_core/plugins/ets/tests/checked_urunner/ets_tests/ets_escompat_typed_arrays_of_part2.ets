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
//! RUN           force_jit: true, options: "--compiler-regex=.*test.*ArrayOf.*", entry: "ets_escompat_typed_arrays_of_part2.ETSGLOBAL::checkTypedArrayOf"
//! METHOD        "ets_escompat_typed_arrays_of_part2.ETSGLOBAL::testBigInt64ArrayOfInt"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.BigInt64ArrayOfInt"
//! METHOD        "ets_escompat_typed_arrays_of_part2.ETSGLOBAL::testBigInt64ArrayOfLong"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.BigInt64ArrayOfLong"
//! METHOD        "ets_escompat_typed_arrays_of_part2.ETSGLOBAL::testBigInt64ArrayOfNumber"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.BigInt64ArrayOfNumber"
//! METHOD        "ets_escompat_typed_arrays_of_part2.ETSGLOBAL::testFloat32ArrayOfInt"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Float32ArrayOfInt"
//! METHOD        "ets_escompat_typed_arrays_of_part2.ETSGLOBAL::testFloat32ArrayOfFloat"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Float32ArrayOfFloat"
//! METHOD        "ets_escompat_typed_arrays_of_part2.ETSGLOBAL::testFloat32ArrayOfNumber"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Float32ArrayOfNumber"
//! METHOD        "ets_escompat_typed_arrays_of_part2.ETSGLOBAL::testFloat64ArrayOfInt"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Float64ArrayOfInt"
//! METHOD        "ets_escompat_typed_arrays_of_part2.ETSGLOBAL::testFloat64ArrayOfNumber"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Float64ArrayOfNumber"

function checkTypedArrayOf() {
    testBigInt64ArrayOfInt()
    testBigInt64ArrayOfLong()
    testBigInt64ArrayOfNumber()

    testFloat32ArrayOfInt()
    testFloat32ArrayOfFloat()
    testFloat32ArrayOfNumber()

    testFloat64ArrayOfInt()
    testFloat64ArrayOfNumber()
}

function testInt8ArrayOfByte() {
    let expected: FixedArray<number> = [(1).toByte(), (2).toByte(), (3).toByte(), (700).toByte()]
    let arr = Int8Array.of((1).toByte(), (2).toByte(), (3).toByte(), (700).toByte())
    checkArray(arr, expected)
}

function testInt8ArrayOfInt() {
    let expected: FixedArray<number> = [(1).toByte(), (2).toByte(), (3).toByte(), (700).toByte()]
    let arr = Int8Array.of(1, 2, 3, 700)
    checkArray(arr, expected)
}

function testInt8ArrayOfNumber() {
    let expected: FixedArray<number> = [12.0, 22.0, 32.0, 42.0, 33.0, 0, 0, 0]
    let arr = Int8Array.of(12.0, 22.0, 32.0, 42.0, 33.0, NaN, Infinity, -Infinity)
    checkArray(arr, expected)
}

function testInt16ArrayOfShort() {
    let expected: FixedArray<number> = [(1).toShort(), (2).toShort(), (3).toShort(), (700000).toShort()]
    let arr = Int16Array.of((1).toShort(), (2).toShort(), (3).toShort(), (700000).toShort())
    checkArray(arr, expected)
}

function testInt16ArrayOfInt() {
    let expected: FixedArray<number> = [(1).toShort(), (2).toShort(), (3).toShort(), (700000).toShort()]
    let arr = Int16Array.of(1, 2, 3, 700000)
    checkArray(arr, expected)
}

function testInt16ArrayOfNumber() {
    let expected: FixedArray<number> = [12.0, 22.0, 32.0, 42.0, 33.0, 0, 0, 0]
    let arr = Int16Array.of(12.0, 22.0, 32.0, 42.0, 33.0, NaN, Infinity, -Infinity)
    checkArray(arr, expected)
}

function testInt32ArrayOfInt() {
    let expected: FixedArray<number> = [1, 2, 3, 700000]
    let arr = Int32Array.of(1, 2, 3, 700000)
    checkArray(arr, expected)
}

function testInt32ArrayOfNumber() {
    let expected: FixedArray<number> = [12.0, 22.0, 32.0, 42.0 , 33.0, -2147483648, 0, 0, 0, 0]
    let arr = Int32Array.of(12.0, 22.0, 32.0, 42.0, 33.0, 1.93428e+25, 3.86856e+25, NaN, Infinity, -Infinity)
    checkArray(arr, expected)
}

function testBigInt64ArrayOfInt() {
    let expected: FixedArray<number> = [1, 2, 3, 700000]
    let arr = BigInt64Array.of(1, 2, 3, 700000)
    checkArray(arr, expected)
}

function testBigInt64ArrayOfLong() {
    let expected: FixedArray<number> = [1, 2, 3, 700000]
    let arr = BigInt64Array.of((1).toLong(), (2).toLong(), (3).toLong(), (700000).toLong())
    checkArray(arr, expected)
}

function testBigInt64ArrayOfNumber() {
    let expected: FixedArray<number> = [12, 22, 32, 42, 33]
    let arr = BigInt64Array.of(12.0, 22.0, 32.0, 42.0, 33.0)
    checkArray(arr, expected)
}

function testFloat32ArrayOfInt() {
    let expected: FixedArray<number> = [12, 22, 32, 42, 33]
    let arr = Float32Array.of(12, 22, 32, 42, 33)
    checkArray(arr, expected)
}

function testFloat32ArrayOfFloat() {
    let expected: FixedArray<number> = [12.0, 22.0, 32.0, 42.0, 33.0]
    let arr = Float32Array.of(12.0, 22.0, 32.0, 42.0, 33.0)
    checkArray(arr, expected)
}

function testFloat32ArrayOfNumber() {
    let expected: FixedArray<number> = [12.0, 22.0, 32.0, 42.0, 33.0]
    let arr = Float32Array.of((12).toDouble(), (22).toDouble(), (32).toDouble(), (42).toDouble(), (33).toDouble())
    checkArray(arr, expected)
}

function testFloat64ArrayOfInt() {
    let expected: FixedArray<number> = [12, 22, 32, 42, 33]
    let arr = Float64Array.of(12, 22, 32, 42, 33)
    checkArray(arr, expected)
}

function testFloat64ArrayOfNumber() {
    let expected: FixedArray<number> = [12.0, 22.0, 32.0, 42.0, 33.0]
    let arr = Float64Array.of(12.0, 22.0, 32.0, 42.0, 33.0)
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
