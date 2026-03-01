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
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays_set_get_part4.ETSGLOBAL::main",
//!               options: "--compiler-encode-intrinsics=false --compiler-regex='.*test.*ArrayGetUnsafe'"
//!
//! METHOD        "testUint8ArrayGetUnsafe"
//! METHOD        "escompat_Uint8Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//!
//! METHOD        "testUint8ClampedArrayGetUnsafe"
//! METHOD        "escompat_Uint8ClampedArray::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ClampedArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//!
//! METHOD        "testUint16ArrayGetUnsafe"
//! METHOD        "escompat_Uint16Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt16ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//!
//! METHOD        "testUint32ArrayGetUnsafe"
//! METHOD        "escompat_Uint32Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/

//! CHECKER       JIT Test IR intrinsics
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays_set_get_part4.ETSGLOBAL::main",
//!               options: "--compiler-regex='(.*test.*ArrayGetUnsafe)|(escompat\.*Array::at)'"
//!
//! METHOD        "testUint8ArrayGetUnsafe"
//! METHOD        "escompat_Uint8Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt8ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "testUint8ClampedArrayGetUnsafe"
//! METHOD        "escompat_Uint8ClampedArray::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt8ClampedArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! INST          /LoadObject.*escompat\.Uint[\d]+ClampedArray\.buffer/
//! INST          /LoadObject.*escompat\.Uint[\d]+ClampedArray\.lengthInt/
//! INST          /LoadObject.*escompat\.Uint[\d]+ClampedArray\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "testUint16ArrayGetUnsafe"
//! METHOD        "escompat_Uint16Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt16ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "testUint32ArrayGetUnsafe"
//! METHOD        "escompat_Uint32Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt32ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "testBigUint64ArrayGetUnsafe"
//! METHOD        "escompat_BigUint64Array::at"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.BigUInt64ArrayGetUnsafe "
//! INST_NOT      /Call.*Array::getUnsafe/
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/

function testUint8ArrayGetUnsafe(): void {
    let ar = new Uint8Array(6)
    ar[2] = -1
    ar[3] = 255
    ar[4] = 257
    arktest.assertEQ(255, ar.at(2))
    arktest.assertEQ(255, ar.at(3))
    arktest.assertEQ(1, ar.at(4))
}

function testUint8ClampedArrayGetUnsafe(): void {
    let ar = new Uint8ClampedArray(6)
    ar.fill(77)
    ar[2] = -1
    ar[3] = 255
    ar[4] = 256
    arktest.assertEQ(0, ar.at(2))
    arktest.assertEQ(255, ar.at(3))
    arktest.assertEQ(255, ar.at(4))
}

function testUint16ArrayGetUnsafe(): void {
    let ar = new Uint16Array(6)
    ar[2] = -1
    ar[3] = 65535
    ar[4] = 65537
    arktest.assertEQ(65535, ar.at(2))
    arktest.assertEQ(65535, ar.at(3))
    arktest.assertEQ(1, ar.at(4))
}

function testUint32ArrayGetUnsafe(): void {
    let ar = new Uint32Array(6)
    ar[2] = (-1).toLong()
    ar[3] = 4294967295
    ar[4] = 4294967297
    arktest.assertEQ(4294967295, ar.at(2))
    arktest.assertEQ(4294967295, ar.at(3))
    arktest.assertEQ(1, ar.at(4))
}

function testBigUint64ArrayGetUnsafe(): void {
    let ar = new BigUint64Array(6)
    ar[2] = 18446744073709551618n
    ar[3] = -18446744073709551618n
    ar[4] = -1n
    ar[5] = 18446744073709551615n
    arktest.assertEQ(2n, ar.at(2))
    arktest.assertEQ(18446744073709551614n, ar.at(3))
    arktest.assertEQ(18446744073709551615n, ar.at(4))
    arktest.assertEQ(18446744073709551615n, ar.at(5))
}

function main() {
    testUint8ArrayGetUnsafe()
    testUint8ClampedArrayGetUnsafe()
    testUint16ArrayGetUnsafe()
    testUint32ArrayGetUnsafe()
    testBigUint64ArrayGetUnsafe()
}
