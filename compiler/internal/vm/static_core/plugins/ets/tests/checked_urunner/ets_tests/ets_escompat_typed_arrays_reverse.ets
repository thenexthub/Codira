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

//! CHECKER       JIT: reverse() and toReversed()
//! RUN           force_jit: true,  options: "--compiler-regex=.*::test_.*_array", entry: "ets_escompat_typed_arrays_reverse.ETSGLOBAL::main"
//!
//! METHOD        "test_iu8_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArrayReverse "
//! INST          /Call.* escompat.Int8Array::toReversed /
//! INST          "Intrinsic.UInt8ArrayReverse "
//! INST          /Call.* escompat.Uint8Array::toReversed /
//!
//! METHOD        "test_iu16_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArrayReverse "
//! INST          /Call.* escompat.Int16Array::toReversed /
//! INST          "Intrinsic.UInt16ArrayReverse "
//! INST          /Call.* escompat.Uint16Array::toReversed /
//!
//! METHOD        "test_iu32_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArrayReverse "
//! INST          /Call.* escompat.Int32Array::toReversed /
//! INST          "Intrinsic.UInt32ArrayReverse "
//! INST          /Call.* escompat.Uint32Array::toReversed /
//!
//! METHOD        "test_iu64_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArrayReverse "
//! INST          /Call.* escompat.BigInt64Array::toReversed /
//! INST          "Intrinsic.BigUInt64ArrayReverse "
//! INST          /Call.* escompat.BigUint64Array::toReversed /
//!
//! METHOD        "test_f32_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArrayReverse "
//! INST          /Call.* escompat.Float32Array::toReversed /
//!
//! METHOD        "test_f64_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArrayReverse "
//! INST          /Call.* escompat.Float64Array::toReversed /


//! CHECKER       AOT: reverse() and toReversed()
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex=.*::test_.*_array", entry: "ets_escompat_typed_arrays_reverse.ETSGLOBAL::main"
//!
//! METHOD        "test_iu8_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArrayReverse "
//! INST          /Call.* escompat.Int8Array::toReversed /
//! INST          "Intrinsic.UInt8ArrayReverse "
//! INST          /Call.* escompat.Uint8Array::toReversed /
//!
//! METHOD        "test_iu16_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArrayReverse "
//! INST          /Call.* escompat.Int16Array::toReversed /
//! INST          "Intrinsic.UInt16ArrayReverse "
//! INST          /Call.* escompat.Uint16Array::toReversed /
//!
//! METHOD        "test_iu32_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArrayReverse "
//! INST          /Call.* escompat.Int32Array::toReversed /
//! INST          "Intrinsic.UInt32ArrayReverse "
//! INST          /Call.* escompat.Uint32Array::toReversed /
//!
//! METHOD        "test_iu64_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArrayReverse "
//! INST          /Call.* escompat.BigInt64Array::toReversed /
//! INST          "Intrinsic.BigUInt64ArrayReverse "
//! INST          /Call.* escompat.BigUint64Array::toReversed /
//!
//! METHOD        "test_f32_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArrayReverse "
//! INST          /Call.* escompat.Float32Array::toReversed /
//!
//! METHOD        "test_f64_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArrayReverse "
//! INST          /Call.* escompat.Float64Array::toReversed /

function test_iu8_array() {
    let inp = Int8Array.of(-128, -42, -14, 0, 16, 48, 127)
    let exp = Int8Array.of(127, 48, 16, 0, -14, -42, -128)
    check_array(new Int8Array(inp).reverse(), inp.toReversed(), exp)

    let uinp = Uint8Array.of(0, 16, 48, 127, 160, 255)
    let uexp = Uint8Array.of(255, 160, 127, 48, 16, 0)
    check_array(new Uint8Array(uinp).reverse(), uinp.toReversed(), uexp)
}

function test_iu16_array() {
    let inp = Int16Array.of(-32768, -2731, -227, 0, 128, 2564, 32767)
    let exp = Int16Array.of(32767, 2564, 128, 0, -227, -2731, -32768)
    check_array(new Int16Array(inp).reverse(), inp.toReversed(), exp)

    let uinp = Uint16Array.of(0, 128, 2564, 32767, 65535)
    let uexp = Uint16Array.of(65535, 32767, 2564, 128, 0)
    check_array(new Uint16Array(uinp).reverse(), uinp.toReversed(), uexp)
}

function test_iu32_array() {
    let inp = Int32Array.of(-2147483648, -1398, 0, 11184, 89478, 536872, 2147483647)
    let exp = Int32Array.of(2147483647, 536872, 89478, 11184, 0, -1398, -2147483648)
    check_array(new Int32Array(inp).reverse(), inp.toReversed(), exp)

    let uinp = Uint32Array.of(0, 11184, 89478, 536872, 2147483647)
    let uexp = Uint32Array.of(2147483647, 536872, 89478, 11184, 0)
    check_array(new Uint32Array(uinp).reverse(), uinp.toReversed(), uexp)
}

function test_iu64_array() {
    let inp = BigInt64Array.of(-9.223e+18, -8589934588, 0, 4294967294, 2.3058e+16, 9.223e+18)
    let exp = BigInt64Array.of(9.223e+18, 2.3058e+16, 4294967294, 0, -8589934588, -9.223e+18)
    check_array(new BigInt64Array(inp).reverse(), inp.toReversed(), exp)

    let uinp = BigUint64Array.of(0, 4294967294, 2.3058e+16, 9.223e+18, 18.446e+18)
    let uexp = BigUint64Array.of(18.446e+18, 9.223e+18, 2.3058e+16, 4294967294, 0)
    check_array(new BigUint64Array(uinp).reverse(), uinp.toReversed(), uexp)
}

function test_f32_array() {
    let inp = Float32Array.of(-3.4e+38, -2.02e+16, 1.06e+12, 2.8e+24,  3.4e+38)
    let exp = Float32Array.of(3.4e+38, 2.8e+24, 1.06e+12, -2.02e+16, -3.4e+38)
    check_array(new Float32Array(inp).reverse(), inp.toReversed(), exp)
}

function test_f64_array() {
    let inp = Float64Array.of(-1.7e+308, -3.6e+42, -0.6e+16, 0.4e+105, 1.7e+308)
    let exp = Float64Array.of(1.7e+308, 0.4e+105, -0.6e+16, -3.6e+42, -1.7e+308)
    check_array(new Float64Array(inp).reverse(), inp.toReversed(), exp)
}

function check_array<T extends Number | BigInt>(src: ArrayLike<T>, src2: ArrayLike<T>, exp: ArrayLike<T>) {
    arktest.assertEQ(src.length, exp.length, "reverse(): bad array len")
    arktest.assertEQ(src2.length, exp.length, "toReversed(): bad array len")
    for (let i = 0; i < exp.length; i++) {
        arktest.assertEQ(src[i], exp[i], "reverse(): unexpected element: i: " + i)
        arktest.assertEQ(src2[i], exp[i], "toReversed(): unexpected element: i: " + i)
    }
}

function main(): void {
    test_iu8_array()
    test_iu16_array()
    test_iu32_array()
    test_iu64_array()
    test_f32_array()
    test_f64_array()
}
