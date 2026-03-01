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
//! RUN           force_jit: true, options: "--compiler-regex=.*test.*ArrayOf.*", entry: "ets_escompat_typed_arrays_of_part3.ETSGLOBAL::checkTypedArrayOf"
//! METHOD        "ets_escompat_typed_arrays_of_part3.ETSGLOBAL::testUint8ClampedArrayOfNumber"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Uint8ClampedArrayOfNumber"
//! METHOD        "ets_escompat_typed_arrays_of_part3.ETSGLOBAL::testUint8ClampedArrayOfInt"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Uint8ClampedArrayOfInt"
//! METHOD        "ets_escompat_typed_arrays_of_part3.ETSGLOBAL::testUint8ClampedArrayOfShort"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Uint8ClampedArrayOfShort"
//! METHOD        "ets_escompat_typed_arrays_of_part3.ETSGLOBAL::testUint8ArrayOfNumber"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Uint8ArrayOfNumber"
//! METHOD        "ets_escompat_typed_arrays_of_part3.ETSGLOBAL::testUint8ArrayOfInt"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Uint8ArrayOfInt"
//! METHOD        "ets_escompat_typed_arrays_of_part3.ETSGLOBAL::testUint8ArrayOfShort"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Uint8ArrayOfShort"
//! METHOD        "ets_escompat_typed_arrays_of_part3.ETSGLOBAL::testUint16ArrayOfNumber"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Uint16ArrayOfNumber"
//! METHOD        "ets_escompat_typed_arrays_of_part3.ETSGLOBAL::testUint16ArrayOfInt"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.Uint16ArrayOfInt"

function checkTypedArrayOf() {
    testUint8ClampedArrayOfNumber()
    testUint8ClampedArrayOfInt()
    testUint8ClampedArrayOfShort()

    testUint8ArrayOfNumber()
    testUint8ArrayOfInt()
    testUint8ArrayOfShort()

    testUint16ArrayOfNumber()
    testUint16ArrayOfInt()
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

function testUint8ClampedArrayOfNumber() {
    let expected: FixedArray<number> = [12, 0, 255, 42, 33, 0, 255, 0]
    let arr = Uint8ClampedArray.of(12.0, -6.0, 332.0, 42.0, 33.0, NaN, Infinity, -Infinity)
    checkArray(arr, expected)
}

function testUint8ClampedArrayOfInt() {
    let expected: FixedArray<number> = [12, 0, 255, 42, 33]
    let arr = Uint8ClampedArray.of((12).toInt(), (-6).toInt(), (332).toInt(), (42).toInt(), (33).toInt())
    checkArray(arr, expected)
}

function testUint8ClampedArrayOfShort() {
    let expected: FixedArray<number> = [12, 0, 255, 42, 33]
    let arr = Uint8ClampedArray.of(12, -6, 332, 42, 33)
    checkArray(arr, expected)
}

function testUint8ArrayOfNumber() {
    let expected: FixedArray<number> = [12, 6, 76, 42, 33, 0, 0, 0]
    let arr = Uint8Array.of(12.0, 6.0, 76.0, 42.0, 33.0, NaN, Infinity, -Infinity)
    checkArray(arr, expected)
}

function testUint8ArrayOfInt() {
    let expected: FixedArray<number> = [12, 6, 76, 42, 33]
    let arr = Uint8Array.of((12).toInt(), (6).toInt(), (76).toInt(), (42).toInt(), (33).toInt())
    checkArray(arr, expected)
}

function testUint8ArrayOfShort() {
    let expected: FixedArray<number> = [12, 6, 76, 42, 33]
    let arr = Uint8Array.of(12, 6, 76, 42, 33)
    checkArray(arr, expected)
}

function testUint16ArrayOfNumber() {
    let expected: FixedArray<number> = [12, 6, 76, 42, 33, 0, 0, 0]
    let arr = Uint16Array.of(12.0, 6.0, 76.0, 42.0, 33.0, NaN, Infinity, -Infinity)
    checkArray(arr, expected)
}

function testUint16ArrayOfInt() {
    let expected: FixedArray<number> = [12, 6, 332, 42, 33]
    let arr = Uint16Array.of(12, 6, 332, 42, 33)
    checkArray(arr, expected)
}

function checkArray(arr: ArrayLike<number>, expected: FixedArray<number>) {
    arktest.assertEQ(arr.length, expected.length, "Unexpected array length")
    for (let i = 0; i < expected.length; i++) {
        arktest.assertEQ(arr[i], expected[i], "Error in index: " + i + ", value: " + arr[i] + " expected value: " + expected[i])
    }
}
