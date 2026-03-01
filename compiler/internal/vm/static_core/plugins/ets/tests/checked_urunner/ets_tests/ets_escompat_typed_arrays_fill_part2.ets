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

//! CHECKER       Array fill intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*testFill.*", entry: "ets_escompat_typed_arrays_fill_part2.ETSGLOBAL::testMain"
//! METHOD        "ets_escompat_typed_arrays_fill_part2.ETSGLOBAL::testFillBigInt64Array"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.BigInt64ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
//! METHOD        "ets_escompat_typed_arrays_fill_part2.ETSGLOBAL::testFillUint8Array"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.UInt8ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
//! METHOD        "ets_escompat_typed_arrays_fill_part2.ETSGLOBAL::testFillUint8ClampedArray"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.UInt8ClampedArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
//! METHOD        "ets_escompat_typed_arrays_fill_part2.ETSGLOBAL::testFillUint16Array"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.UInt16ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
//! METHOD        "ets_escompat_typed_arrays_fill_part2.ETSGLOBAL::testFillUint32Array"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.UInt32ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/
//! METHOD        "ets_escompat_typed_arrays_fill_part2.ETSGLOBAL::testFillBigUint64Array"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.BigUInt64ArrayFillInternal "
//! INST_NOT      /Call.*Array::fillInternal/

function testFillBigInt64Array() {
    let a = new BigInt64Array(6)
    a.fill(0)
    a.fill(77, 2, 5)
    return checkArray(a, BigInt64Array.of(0, 0, 77, 77, 77, 0))
}

function testFillUint8Array() {
    let a = new Uint8Array(6)
    a.fill(0)
    a.fill(77, 2, 5)
    return checkArray(a, Uint8Array.of(0, 0, 77, 77, 77, 0))
}

function testFillUint8ClampedArray() {
    let a = new Uint8ClampedArray(6)
    a.fill(0)
    a.fill(256, 2, 5)
    return checkArray(a, Uint8ClampedArray.of(0, 0, 255, 255, 255, 0))
}

function testFillUint16Array() {
    let a = new Uint16Array(6)
    a.fill(0)
    a.fill(77, 2, 5)
    return checkArray(a, Uint16Array.of(0, 0, 77, 77, 77, 0))
}

function testFillUint32Array() {
    let a = new Uint32Array(6)
    a.fill(0)
    a.fill(77, 2, 5)
    return checkArray(a, Uint32Array.of(0, 0, 77, 77, 77, 0))
}

function testFillBigUint64Array() {
    let a = new BigUint64Array(6)
    a.fill(0)
    a.fill(77, 2, 5)
    return checkArray(a, BigUint64Array.of(0, 0, 77, 77, 77, 0))
}

function testMain() : int {
	if (!testFillBigInt64Array()) return 1;
	if (!testFillUint8Array()) return 1;
	if (!testFillUint8ClampedArray()) return 1;
	if (!testFillUint16Array()) return 1;
	if (!testFillUint32Array()) return 1;
	if (!testFillBigUint64Array()) return 1;

	return 0;
}

function checkArray(arr: ArrayLike<number>, expected: ArrayLike<number>) {
    if (arr.length != expected.length) return false;

    for (let i = 0; i < expected.length; i++) {
        if (isNaN(arr[i]) && isNaN(expected[i])) {
            continue
        }
        if (arr[i] != expected[i]) return false;
    }
    return true;
}

function checkArray(arr: BigInt64Array, expected: BigInt64Array) {
    if (arr.length != expected.length) return false;

    for (let i = 0; i < expected.length; i++) {
        if (arr[i] != expected[i]) return false;
    }
    return true;
}

function checkArray(arr: BigUint64Array, expected: BigUint64Array) {
    if (arr.length != expected.length) return false;

    for (let i = 0; i < expected.length; i++) {
        if (arr[i] != expected[i]) return false;
    }
    return true;
}
