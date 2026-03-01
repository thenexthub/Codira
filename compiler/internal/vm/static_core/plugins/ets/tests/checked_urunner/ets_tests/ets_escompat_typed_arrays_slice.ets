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

const beginPlus : int = 1
const endMinus : int = 1

//! CHECKER       JIT TypedArray slice intrinsic
//! RUN           force_jit: true,  options: "--compiler-regex=.*::test_slice_.*array", entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::main"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::test_slice_int8array"
//! PASS_AFTER    "Inline"
//! INST          /Call.* escompat.*Array::slice/
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::test_slice_int16array"
//! PASS_AFTER    "Inline"
//! INST          /Call.* escompat.*Array::slice/
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::test_slice_int32array"
//! PASS_AFTER    "Inline"
//! INST          /Call.* escompat.*Array::slice/
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::test_slice_bigint64array"
//! PASS_AFTER    "Inline"
//! INST          /Call.* escompat.*Array::slice/
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::test_slice_float32array"
//! PASS_AFTER    "Inline"
//! INST          /Call.* escompat.*Array::slice/
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::test_slice_float64array"
//! PASS_AFTER    "Inline"
//! INST          /Call.* escompat.*Array::slice/
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::test_slice_uint8array"
//! PASS_AFTER    "Inline"
//! INST          /Call.* escompat.*Array::slice/
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::test_slice_uint16array"
//! PASS_AFTER    "Inline"
//! INST          /Call.* escompat.*Array::slice/
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::test_slice_uint32array"
//! PASS_AFTER    "Inline"
//! INST          /Call.* escompat.*Array::slice/
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::test_slice_biguint64array"
//! PASS_AFTER    "Inline"
//! INST          /Call.* escompat.*Array::slice/
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::test_slice_uint8clampedarray"
//! PASS_AFTER    "Inline"
//! INST          /Call.* escompat.*Array::slice/


//! CHECKER       AOT TypedArray slice intrinsic
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex=.*::test_slice_.*array", entry: "ets_escompat_typed_arrays_slice.ETSGLOBAL::main"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::test_slice_int8array"
//! PASS_AFTER    "IrBuilder"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::test_slice_int16array"
//! PASS_AFTER    "IrBuilder"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::test_slice_int32array"
//! PASS_AFTER    "IrBuilder"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::test_slice_bigint64array"
//! PASS_AFTER    "IrBuilder"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::test_slice_float32array"
//! PASS_AFTER    "IrBuilder"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::test_slice_float64array"
//! PASS_AFTER    "IrBuilder"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::test_slice_uint8array"
//! PASS_AFTER    "IrBuilder"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::test_slice_uint16array"
//! PASS_AFTER    "IrBuilder"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::test_slice_uint32array"
//! PASS_AFTER    "IrBuilder"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::test_slice_biguint64array"
//! PASS_AFTER    "IrBuilder"
//!
//! METHOD        "ets_escompat_typed_arrays_slice.ETSGLOBAL::test_slice_uint8clampedarray"
//! PASS_AFTER    "IrBuilder"

function test_slice_int8array() {
    let src = new Int8Array([-128, -41, -14, 0, 17, 45, 127])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    check_array(src, dst)
}

function test_slice_int16array() {
    let src = new Int16Array([-32768, -2732, -227, 0, 128, 2563, 32767])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    check_array(src, dst)
}

function test_slice_int32array() {
    let src = new Int32Array([-2147483648, -1397, 0, 11185, 89478, 536873, 2147483647])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    check_array(src, dst)
}

function test_slice_bigint64array() {
    let src = new BigInt64Array([-9.223e+18, -8589934588, 4294967294, 2.3058e+16, 9.223e+18])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    check_array(src, dst)
}

function test_slice_float32array() {
    let src = new Float32Array([-3.4e+38, -2.02e+16, 1.06e+12, 2.8e+24,  3.4e+38])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    check_array(src, dst)
}

function test_slice_float64array() {
    let src = new Float64Array([-1.7e+308, -3.6e+42, -0.6e+16, 0.4e+105, 1.7e+308])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    check_array(src, dst)
}

function test_slice_uint8array() {
    let src = new Uint8Array([0, 17, 45, 127, 14, 41, 255])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    check_array(src, dst)
}

function test_slice_uint16array() {
    let src = new Uint16Array([128, 0, 2563, 32767, 227, 2732, 65535])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    check_array(src, dst)
}

function test_slice_uint32array() {
    let src = new Uint32Array([11185, 89478, 0, 536873, 2147483647, 1397, 4294967295])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    check_array(src, dst)
}

function test_slice_biguint64array() {
    let src = new BigUint64Array([4294967294, 2.3058e+16, 9.223e+18, 8589934588, 18.446e+18])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    check_array(src, dst)
}

function test_slice_uint8clampedarray() {
    let src = new Uint8ClampedArray([17, 45, 127, 0, 255, 43, 14])
    let dst = src.slice(0 + beginPlus, (src.length).toInt() - endMinus)
    check_array(src, dst)
}

function check_array<T extends Number | BigInt>(source_data: ArrayLike<T>, actual_data: ArrayLike<T>): void {
    const actualLen = (actual_data.length).toInt()
    const expectedLen = (source_data.length).toInt() - (beginPlus + endMinus)
    if (actualLen != expectedLen) {
        throw new Error("Unexpected array length! Current: " + actualLen + " Expected: " + expectedLen)
    }
    for (let i : int = 0; i < expectedLen; i++) {
        if (source_data[i + beginPlus] != actual_data[i]) {
            throw new Error("Unexpected element with index " + i)
        }
    }
}

function main(): void {
    test_slice_int8array()
    test_slice_int16array()
    test_slice_int32array()
    test_slice_bigint64array()
    test_slice_float32array()
    test_slice_float64array()
    test_slice_uint8array()
    test_slice_uint16array()
    test_slice_uint32array()
    test_slice_biguint64array()
    test_slice_uint8clampedarray()
}
