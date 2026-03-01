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

//! CHECKER       JIT: copyWithin()
//! RUN           force_jit: true,  options: "--compiler-regex=escompat.*Array::copyWithin", entry: "ets_escompat_typed_arrays_copy_within.ETSGLOBAL::main"
//! METHOD        "escompat.Int8Array::copyWithin"
//! PASS_AFTER    "IrBuilder"
//! INST          /Intrinsic.Int8ArrayCopyWithinImpl/
//! METHOD        "escompat.Int16Array::copyWithin"
//! PASS_AFTER    "IrBuilder"
//! INST          /Intrinsic.Int16ArrayCopyWithinImpl/
//! METHOD        "escompat.Int32Array::copyWithin"
//! PASS_AFTER    "IrBuilder"
//! INST          /Intrinsic.Int32ArrayCopyWithinImpl/
//! METHOD        "escompat.BigInt64Array::copyWithin"
//! PASS_AFTER    "IrBuilder"
//! INST          /Intrinsic.BigInt64ArrayCopyWithinImpl/
//!
//! METHOD        "escompat.Float32Array::copyWithin"
//! PASS_AFTER    "IrBuilder"
//! INST          /Intrinsic.Float32ArrayCopyWithinImpl/
//! METHOD        "escompat.Float64Array::copyWithin"
//! PASS_AFTER    "IrBuilder"
//! INST          /Intrinsic.Float64ArrayCopyWithinImpl/
//!
//! METHOD        "escompat.Uint8Array::copyWithin"
//! PASS_AFTER    "IrBuilder"
//! INST          /Intrinsic.UInt8ArrayCopyWithinImpl/
//! METHOD        "escompat.Uint8ClampedArray::copyWithin"
//! PASS_AFTER    "IrBuilder"
//! INST          /Intrinsic.UInt8ClampedArrayCopyWithinImpl/
//! METHOD        "escompat.Uint16Array::copyWithin"
//! PASS_AFTER    "IrBuilder"
//! INST          /Intrinsic.UInt16ArrayCopyWithinImpl/
//! METHOD        "escompat.Uint32Array::copyWithin"
//! PASS_AFTER    "IrBuilder"
//! INST          /Intrinsic.UInt32ArrayCopyWithinImpl/
//! METHOD        "escompat.BigUint64Array::copyWithin"
//! PASS_AFTER    "IrBuilder"
//! INST          /Intrinsic.BigUInt64ArrayCopyWithinImpl/

//! CHECKER       AOT: copyWithin()
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex=.*::test_.*array_.*", entry: "ets_escompat_typed_arrays_copy_within.ETSGLOBAL::main"
//! METHOD        "test_int8array_copy_within"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Int8Array::copyWithin/
//! METHOD        "test_int16array_copy_within"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Int16Array::copyWithin/
//! METHOD        "test_int32array_copy_within"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Int32Array::copyWithin/
//! METHOD        "test_bigint64array_copy_within"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.BigInt64Array::copyWithin/
//!
//! METHOD        "test_float32array_copy_within"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Float32Array::copyWithin/
//! METHOD        "test_float64array_copy_within"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Float64Array::copyWithin/
//!
//! METHOD        "test_uint8array_copy_within"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint8Array::copyWithin/
//! METHOD        "test_uint8clampedarray_copy_within"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint8ClampedArray::copyWithin/
//! METHOD        "test_uint16array_copy_within"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint16Array::copyWithin/
//! METHOD        "test_uint32array_copy_within"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint32Array::copyWithin/
//! METHOD        "test_biguint64array_copy_within"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.BigUint64Array::copyWithin/


function check_array(arr: ArrayLike<number>, expected: ArrayLike<number>) {
    if (arr.length != expected.length) {
        throw new Error("Unexpected array length")
    }
    for (let i = 0; i < expected.length; i++) {
        if (arr[i] != expected[i]) {
            throw new Error("Unexpected element with index " + i)
        }
    }
}

function check_array(arr: BigInt64Array, expected: BigInt64Array) {
    if (arr.length != expected.length) {
        throw new Error("Unexpected array length")
    }
    for (let i = 0; i < expected.length; i++) {
        if (arr[i] != expected[i]) {
            throw new Error("Unexpected element with index " + i)
        }
    }
}

function check_array(arr: BigUint64Array, expected: BigUint64Array) {
    if (arr.length != expected.length) {
        throw new Error("Unexpected array length")
    }
    for (let i = 0; i < expected.length; i++) {
        if (arr[i] != expected[i]) {
            throw new Error("Unexpected element with index " + i)
        }
    }
}

function test_int8array_copy_within() {
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 3), Int8Array.of(1, 1, 2, 3, 5))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin(-5, 0, 3), Int8Array.of(1, 2, 3, 4, 5))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin(-6, 0, 3), Int8Array.of(1, 2, 3, 4, 5))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin(9, 0, 3), Int8Array.of(1, 2, 3, 4, 5))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin(1, -1, 3), Int8Array.of(1, 2, 3, 4, 5))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin(1, -5, 3), Int8Array.of(1, 1, 2, 3, 5))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin(1, -6, 3), Int8Array.of(1, 1, 2, 3, 5))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin(1, 5, 3), Int8Array.of(1, 2, 3, 4, 5))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin(1, 9, 3), Int8Array.of(1, 2, 3, 4, 5))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -1), Int8Array.of(1, 1, 2, 3, 4))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -5), Int8Array.of(1, 2, 3, 4, 5))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -6), Int8Array.of(1, 2, 3, 4, 5))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 5), Int8Array.of(1, 1, 2, 3, 4))
    check_array(new Int8Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 9), Int8Array.of(1, 1, 2, 3, 4))
}

function test_int16array_copy_within() {
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 3), Int16Array.of(1, 1, 2, 3, 5))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin(-5, 0, 3), Int16Array.of(1, 2, 3, 4, 5))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin(-6, 0, 3), Int16Array.of(1, 2, 3, 4, 5))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin(9, 0, 3), Int16Array.of(1, 2, 3, 4, 5))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin(1, -1, 3), Int16Array.of(1, 2, 3, 4, 5))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin(1, -5, 3), Int16Array.of(1, 1, 2, 3, 5))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin(1, -6, 3), Int16Array.of(1, 1, 2, 3, 5))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin(1, 5, 3), Int16Array.of(1, 2, 3, 4, 5))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin(1, 9, 3), Int16Array.of(1, 2, 3, 4, 5))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -1), Int16Array.of(1, 1, 2, 3, 4))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -5), Int16Array.of(1, 2, 3, 4, 5))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -6), Int16Array.of(1, 2, 3, 4, 5))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 5), Int16Array.of(1, 1, 2, 3, 4))
    check_array(new Int16Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 9), Int16Array.of(1, 1, 2, 3, 4))
}

function test_int32array_copy_within() {
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 3), Int32Array.of(1, 1, 2, 3, 5))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin(-5, 0, 3), Int32Array.of(1, 2, 3, 4, 5))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin(-6, 0, 3), Int32Array.of(1, 2, 3, 4, 5))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin(9, 0, 3), Int32Array.of(1, 2, 3, 4, 5))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin(1, -1, 3), Int32Array.of(1, 2, 3, 4, 5))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin(1, -5, 3), Int32Array.of(1, 1, 2, 3, 5))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin(1, -6, 3), Int32Array.of(1, 1, 2, 3, 5))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin(1, 5, 3), Int32Array.of(1, 2, 3, 4, 5))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin(1, 9, 3), Int32Array.of(1, 2, 3, 4, 5))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -1), Int32Array.of(1, 1, 2, 3, 4))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -5), Int32Array.of(1, 2, 3, 4, 5))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -6), Int32Array.of(1, 2, 3, 4, 5))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 5), Int32Array.of(1, 1, 2, 3, 4))
    check_array(new Int32Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 9), Int32Array.of(1, 1, 2, 3, 4))
}

function test_bigint64array_copy_within() {
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 3), BigInt64Array.of(1, 1, 2, 3, 5))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin(-5, 0, 3), BigInt64Array.of(1, 2, 3, 4, 5))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin(-6, 0, 3), BigInt64Array.of(1, 2, 3, 4, 5))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin(9, 0, 3), BigInt64Array.of(1, 2, 3, 4, 5))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin(1, -1, 3), BigInt64Array.of(1, 2, 3, 4, 5))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin(1, -5, 3), BigInt64Array.of(1, 1, 2, 3, 5))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin(1, -6, 3), BigInt64Array.of(1, 1, 2, 3, 5))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin(1, 5, 3), BigInt64Array.of(1, 2, 3, 4, 5))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin(1, 9, 3), BigInt64Array.of(1, 2, 3, 4, 5))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -1), BigInt64Array.of(1, 1, 2, 3, 4))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -5), BigInt64Array.of(1, 2, 3, 4, 5))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -6), BigInt64Array.of(1, 2, 3, 4, 5))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 5), BigInt64Array.of(1, 1, 2, 3, 4))
    check_array(new BigInt64Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 9), BigInt64Array.of(1, 1, 2, 3, 4))
}


function test_float32array_copy_within() {
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 3), Float32Array.of(1, 1, 2, 3, 5))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin(-5, 0, 3), Float32Array.of(1, 2, 3, 4, 5))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin(-6, 0, 3), Float32Array.of(1, 2, 3, 4, 5))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin(9, 0, 3), Float32Array.of(1, 2, 3, 4, 5))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin(1, -1, 3), Float32Array.of(1, 2, 3, 4, 5))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin(1, -5, 3), Float32Array.of(1, 1, 2, 3, 5))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin(1, -6, 3), Float32Array.of(1, 1, 2, 3, 5))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin(1, 5, 3), Float32Array.of(1, 2, 3, 4, 5))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin(1, 9, 3), Float32Array.of(1, 2, 3, 4, 5))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -1), Float32Array.of(1, 1, 2, 3, 4))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -5), Float32Array.of(1, 2, 3, 4, 5))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -6), Float32Array.of(1, 2, 3, 4, 5))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 5), Float32Array.of(1, 1, 2, 3, 4))
    check_array(new Float32Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 9), Float32Array.of(1, 1, 2, 3, 4))
}

function test_float64array_copy_within() {
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 3), Float64Array.of(1, 1, 2, 3, 5))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin(-5, 0, 3), Float64Array.of(1, 2, 3, 4, 5))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin(-6, 0, 3), Float64Array.of(1, 2, 3, 4, 5))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin(9, 0, 3), Float64Array.of(1, 2, 3, 4, 5))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin(1, -1, 3), Float64Array.of(1, 2, 3, 4, 5))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin(1, -5, 3), Float64Array.of(1, 1, 2, 3, 5))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin(1, -6, 3), Float64Array.of(1, 1, 2, 3, 5))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin(1, 5, 3), Float64Array.of(1, 2, 3, 4, 5))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin(1, 9, 3), Float64Array.of(1, 2, 3, 4, 5))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -1), Float64Array.of(1, 1, 2, 3, 4))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -5), Float64Array.of(1, 2, 3, 4, 5))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -6), Float64Array.of(1, 2, 3, 4, 5))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 5), Float64Array.of(1, 1, 2, 3, 4))
    check_array(new Float64Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 9), Float64Array.of(1, 1, 2, 3, 4))
}


function test_uint8array_copy_within() {
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 3), Uint8Array.of(1, 1, 2, 3, 5))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin(-5, 0, 3), Uint8Array.of(1, 2, 3, 4, 5))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin(-6, 0, 3), Uint8Array.of(1, 2, 3, 4, 5))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin(9, 0, 3), Uint8Array.of(1, 2, 3, 4, 5))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin(1, -1, 3), Uint8Array.of(1, 2, 3, 4, 5))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin(1, -5, 3), Uint8Array.of(1, 1, 2, 3, 5))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin(1, -6, 3), Uint8Array.of(1, 1, 2, 3, 5))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin(1, 5, 3), Uint8Array.of(1, 2, 3, 4, 5))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin(1, 9, 3), Uint8Array.of(1, 2, 3, 4, 5))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -1), Uint8Array.of(1, 1, 2, 3, 4))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -5), Uint8Array.of(1, 2, 3, 4, 5))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -6), Uint8Array.of(1, 2, 3, 4, 5))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 5), Uint8Array.of(1, 1, 2, 3, 4))
    check_array(new Uint8Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 9), Uint8Array.of(1, 1, 2, 3, 4))
}

function test_uint8clampedarray_copy_within() {
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin(1, 0, 3), Uint8ClampedArray.of(1, 1, 2, 3, 5))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin(-5, 0, 3), Uint8ClampedArray.of(1, 2, 3, 4, 5))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin(-6, 0, 3), Uint8ClampedArray.of(1, 2, 3, 4, 5))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin(9, 0, 3), Uint8ClampedArray.of(1, 2, 3, 4, 5))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin(1, -1, 3), Uint8ClampedArray.of(1, 2, 3, 4, 5))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin(1, -5, 3), Uint8ClampedArray.of(1, 1, 2, 3, 5))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin(1, -6, 3), Uint8ClampedArray.of(1, 1, 2, 3, 5))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin(1, 5, 3), Uint8ClampedArray.of(1, 2, 3, 4, 5))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin(1, 9, 3), Uint8ClampedArray.of(1, 2, 3, 4, 5))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin(1, 0, -1), Uint8ClampedArray.of(1, 1, 2, 3, 4))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin(1, 0, -5), Uint8ClampedArray.of(1, 2, 3, 4, 5))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin(1, 0, -6), Uint8ClampedArray.of(1, 2, 3, 4, 5))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin(1, 0, 5), Uint8ClampedArray.of(1, 1, 2, 3, 4))
    check_array(new Uint8ClampedArray([1, 2, 3, 4, 5]).copyWithin(1, 0, 9), Uint8ClampedArray.of(1, 1, 2, 3, 4))
}

function test_uint16array_copy_within() {
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 3), Uint16Array.of(1, 1, 2, 3, 5))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin(-5, 0, 3), Uint16Array.of(1, 2, 3, 4, 5))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin(-6, 0, 3), Uint16Array.of(1, 2, 3, 4, 5))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin(9, 0, 3), Uint16Array.of(1, 2, 3, 4, 5))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin(1, -1, 3), Uint16Array.of(1, 2, 3, 4, 5))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin(1, -5, 3), Uint16Array.of(1, 1, 2, 3, 5))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin(1, -6, 3), Uint16Array.of(1, 1, 2, 3, 5))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin(1, 5, 3), Uint16Array.of(1, 2, 3, 4, 5))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin(1, 9, 3), Uint16Array.of(1, 2, 3, 4, 5))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -1), Uint16Array.of(1, 1, 2, 3, 4))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -5), Uint16Array.of(1, 2, 3, 4, 5))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -6), Uint16Array.of(1, 2, 3, 4, 5))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 5), Uint16Array.of(1, 1, 2, 3, 4))
    check_array(new Uint16Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 9), Uint16Array.of(1, 1, 2, 3, 4))
}

function test_uint32array_copy_within() {
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 3), Uint32Array.of(1, 1, 2, 3, 5))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin(-5, 0, 3), Uint32Array.of(1, 2, 3, 4, 5))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin(-6, 0, 3), Uint32Array.of(1, 2, 3, 4, 5))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin(9, 0, 3), Uint32Array.of(1, 2, 3, 4, 5))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin(1, -1, 3), Uint32Array.of(1, 2, 3, 4, 5))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin(1, -5, 3), Uint32Array.of(1, 1, 2, 3, 5))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin(1, -6, 3), Uint32Array.of(1, 1, 2, 3, 5))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin(1, 5, 3), Uint32Array.of(1, 2, 3, 4, 5))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin(1, 9, 3), Uint32Array.of(1, 2, 3, 4, 5))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -1), Uint32Array.of(1, 1, 2, 3, 4))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -5), Uint32Array.of(1, 2, 3, 4, 5))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -6), Uint32Array.of(1, 2, 3, 4, 5))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 5), Uint32Array.of(1, 1, 2, 3, 4))
    check_array(new Uint32Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 9), Uint32Array.of(1, 1, 2, 3, 4))
}

function test_biguint64array_copy_within() {
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 3), BigUint64Array.of(1, 1, 2, 3, 5))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin(-5, 0, 3), BigUint64Array.of(1, 2, 3, 4, 5))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin(-6, 0, 3), BigUint64Array.of(1, 2, 3, 4, 5))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin(9, 0, 3), BigUint64Array.of(1, 2, 3, 4, 5))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin(1, -1, 3), BigUint64Array.of(1, 2, 3, 4, 5))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin(1, -5, 3), BigUint64Array.of(1, 1, 2, 3, 5))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin(1, -6, 3), BigUint64Array.of(1, 1, 2, 3, 5))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin(1, 5, 3), BigUint64Array.of(1, 2, 3, 4, 5))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin(1, 9, 3), BigUint64Array.of(1, 2, 3, 4, 5))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -1), BigUint64Array.of(1, 1, 2, 3, 4))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -5), BigUint64Array.of(1, 2, 3, 4, 5))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin(1, 0, -6), BigUint64Array.of(1, 2, 3, 4, 5))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 5), BigUint64Array.of(1, 1, 2, 3, 4))
    check_array(new BigUint64Array([1, 2, 3, 4, 5]).copyWithin(1, 0, 9), BigUint64Array.of(1, 1, 2, 3, 4))
}


function main(): void {
    test_int8array_copy_within()
    test_int16array_copy_within()
    test_int32array_copy_within()
    test_bigint64array_copy_within()

    test_float32array_copy_within()
    test_float64array_copy_within()

    test_uint8array_copy_within()
    test_uint8clampedarray_copy_within()
    test_uint16array_copy_within()
    test_uint32array_copy_within()
    test_biguint64array_copy_within()
}
