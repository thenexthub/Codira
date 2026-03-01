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

//! CHECKER       JIT Test cpp intrinsics
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays_set_get_part3.ETSGLOBAL::main",
//!               options: "--compiler-encode-intrinsics=false --compiler-regex='.*test.*ArrayGetUnsafe'"
//!
//! METHOD        "testInt8ArrayGetUnsafe"
//! METHOD        "escompat_Int8Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//!
//! METHOD        "testInt16ArrayGetUnsafe"
//! METHOD        "escompat_Int16Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//!
//! METHOD        "testInt32ArrayGetUnsafe"
//! METHOD        "escompat_Int32Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//!
//! METHOD        "testFloat32ArrayGetUnsafe"
//! METHOD        "escompat_Float32Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//!
//! METHOD        "testFloat64ArrayGetUnsafe"
//! METHOD        "escompat_Float64Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/

//! CHECKER       JIT Test IR intrinsics
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays_set_get_part3.ETSGLOBAL::main",
//!               options: "--compiler-regex='(.*test.*ArrayGetUnsafe)|(escompat\.*Array::at)'"
//!
//! METHOD        "testInt8ArrayGetUnsafe"
//! METHOD        "escompat_Int8Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Int8ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "testInt16ArrayGetUnsafe"
//! METHOD        "escompat_Int16Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Int16ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "testInt32ArrayGetUnsafe"
//! METHOD        "escompat_Int32Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Int32ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Int[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "testFloat32ArrayGetUnsafe"
//! METHOD        "escompat_Float32Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Float32ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! INST          /LoadObject.*escompat\.Float[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Float[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Float[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "testFloat64ArrayGetUnsafe"
//! METHOD        "escompat_Float64Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.Float64ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! INST          /LoadObject.*escompat\.Float[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Float[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Float[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "testBigInt64ArrayGetUnsafe"
//! METHOD        "escompat_BigInt64Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.BigInt64ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! INST          /LoadObject.*escompat\.BigInt[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.BigInt[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.BigInt[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/

function testInt8ArrayGetUnsafe(): void {
    let ar = new Int8Array(6)
    ar[2] = (-128).toByte()
    ar[4] = (127).toByte()
    arktest.assertEQ(-128, ar.at(2))
    arktest.assertEQ(127, ar.at(4))
}

function testInt16ArrayGetUnsafe(): void {
    let ar = new Int16Array(6)
    ar[2] = -32768
    ar[3] = -32769
    ar[4] = 32767
    ar[5] = 32768
    arktest.assertEQ(-32768, ar.at(2))
    arktest.assertEQ(32767, ar.at(3))
    arktest.assertEQ(32767, ar.at(4))
    arktest.assertEQ(-32768, ar.at(5))
}

function testInt32ArrayGetUnsafe(): void {
    let ar = new Int32Array(6)
    ar[2] = -2147483648
    ar[4] = 2147483647
    arktest.assertEQ(-2147483648, ar.at(2))
    arktest.assertEQ(2147483647, ar.at(4))
}

function testFloat32ArrayGetUnsafe(): void {
    let ar = new Float32Array(6)
    ar[2] = -3.4028235e+38f
    ar[4] = 3.4028235e+38f
    arktest.assertEQ(-3.4028235e+38f, ar.at(2))
    arktest.assertEQ(3.4028235e+38f, ar.at(4))
}

function testFloat64ArrayGetUnsafe(): void {
    let ar = new Float64Array(6)
    ar[2] = -3.4028235e+38
    ar[4] = 3.4028235e+38
    arktest.assertEQ(-3.4028235e+38, ar.at(2))
    arktest.assertEQ(3.4028235e+38, ar.at(4))
}

function testBigInt64ArrayGetUnsafe(): void {
    let ar = new BigInt64Array(6)
    ar[2] = 9223372036854775807n
    ar[3] = 18446744073709551618n
    ar[4] = -9223372036854775808n
    ar[5] = -18446744073709551618n
    arktest.assertEQ(9223372036854775807n, ar.at(2))
    arktest.assertEQ(2n, ar.at(3))
    arktest.assertEQ(-9223372036854775808n, ar.at(4))
    arktest.assertEQ(-2n, ar.at(5))
}

function main() {
    testInt8ArrayGetUnsafe()
    testInt16ArrayGetUnsafe()
    testInt32ArrayGetUnsafe()
    testFloat32ArrayGetUnsafe()
    testFloat64ArrayGetUnsafe()
    testBigInt64ArrayGetUnsafe()
}
