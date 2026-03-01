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
//! RUN           force_jit: true, options: "--compiler-regex=.*test.*ArrayOf.*", entry: "ets_escompat_typed_arrays_of.ETSGLOBAL::checkTypedArrayOf"
//! METHOD        "ets_escompat_typed_arrays_of.ETSGLOBAL::testInt8ArrayOfByte"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int8ArrayOfByte"
//! METHOD        "ets_escompat_typed_arrays_of.ETSGLOBAL::testInt8ArrayOfInt"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int8ArrayOfInt"
//! METHOD        "ets_escompat_typed_arrays_of.ETSGLOBAL::testInt8ArrayOfNumber"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int8ArrayOfNumber"
//! METHOD        "ets_escompat_typed_arrays_of.ETSGLOBAL::testInt16ArrayOfShort"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int16ArrayOfShort"
//! METHOD        "ets_escompat_typed_arrays_of.ETSGLOBAL::testInt16ArrayOfInt"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int16ArrayOfInt"
//! METHOD        "ets_escompat_typed_arrays_of.ETSGLOBAL::testInt16ArrayOfNumber"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int16ArrayOfNumber"
//! METHOD        "ets_escompat_typed_arrays_of.ETSGLOBAL::testInt32ArrayOfInt"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int32ArrayOfInt"
//! METHOD        "ets_escompat_typed_arrays_of.ETSGLOBAL::testInt32ArrayOfNumber"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Int32ArrayOfNumber"

function checkTypedArrayOf() {
    testInt8ArrayOfByte()
    testInt8ArrayOfInt()
    testInt8ArrayOfNumber()

    testInt16ArrayOfShort()
    testInt16ArrayOfInt()
    testInt16ArrayOfNumber()

    testInt32ArrayOfInt()
    testInt32ArrayOfNumber()
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
    let expected: FixedArray<number> = [12., 22., 32., 42., 33., 0, 0, 0]
    let arr = Int8Array.of(12., 22., 32., 42., 33., NaN, Infinity, -Infinity)
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
    let expected: FixedArray<number> = [12., 22., 32., 42., 33., 0, 0, 0]
    let arr = Int16Array.of(12., 22., 32., 42., 33., NaN, Infinity, -Infinity)
    checkArray(arr, expected)
}

function testInt32ArrayOfInt() {
    let expected: FixedArray<number> = [1, 2, 3, 700000]
    let arr = Int32Array.of(1, 2, 3, 700000)
    checkArray(arr, expected)
}

function testInt32ArrayOfNumber() {
    let expected: FixedArray<number> = [12., 22., 32., 42., 33., -2147483648, 0, 0, 0, 0]
    let arr = Int32Array.of(12., 22., 32., 42., 33., 1.93428e+25, 3.86856e+25, NaN, Infinity, -Infinity)
    checkArray(arr, expected)
}

function checkArray(arr: ArrayLike<number>, expected: FixedArray<number>) {
    arktest.assertEQ(arr.length, expected.length, "Unexpected array length")
    for (let i = 0; i < expected.length; i++) {
        arktest.assertEQ(arr[i], expected[i], "Error in index: " + i + ", value: " + arr[i] + " expected value: " + expected[i])
    }
}
