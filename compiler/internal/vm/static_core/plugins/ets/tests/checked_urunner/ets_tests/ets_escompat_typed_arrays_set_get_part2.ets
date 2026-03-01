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
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays_set_get_part2.ETSGLOBAL::main",
//!               options: "--compiler-encode-intrinsics=false --compiler-loop-idioms=false \
//!                 --compiler-regex=.*::test.*ArraySet.*Cpp"
//!
//! METHOD        "testUint8ArraySetIntCpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.UInt8ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//!
//! METHOD        "testUint8ArraySetRangeErrorCpp"
//!
//! METHOD        "testUint8ClampedArraySetIntCpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ClampedArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.UInt8ClampedArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//!
//! METHOD        "testUint8ClampedArraySetRangeErrorCpp"
//!
//! METHOD        "testUint16ArraySetIntCpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt16ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.UInt16ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//!
//! METHOD        "testUint16ArraySetRangeErrorCpp"
//!
//! METHOD        "testUint32ArraySetIntCpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArraySetInt "
//! INST_NOT      "Intrinsic.UInt32ArraySetLong "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.UInt32ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//!
//! METHOD        "testUint32ArraySetRangeErrorCpp"
//!
//! METHOD        "testUint32ArraySetLongCpp"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArraySetLong "
//! INST_NOT      "Intrinsic.UInt32ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.UInt32ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//!
//! METHOD        "testBigUint64ArraySetIntCpp"
//! PASS_AFTER    "Inline"
//! INST          "Intrinsic.BigUInt64ArraySetInt "
//! INST_NOT      "Intrinsic.BigUInt64ArraySetLong "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          "Intrinsic.BigUInt64ArrayGet "
//!
//! METHOD        "testBigUint64ArraySetLongCpp"
//! METHOD        "escompat_BigUint64Array::__set"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigUInt64ArraySetLong "
//! INST_NOT      "Intrinsic.BigUInt64ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! METHOD        "escompat_BigUint64Array::__get"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigUInt64ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//!
//! METHOD        "testBigUint64ArraySetRangeErrorCpp"
//! METHOD        "escompat_BigUint64Array::__set"


//! CHECKER       JIT Test IR intrinsics
//! RUN           force_jit: true, entry: "ets_escompat_typed_arrays_set_get_part2.ETSGLOBAL::main",
//!               options: "--compiler-regex=.*::test.*ArraySet.*Ir"
//!
//! METHOD        "testUint8ArraySetIntIr"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt8ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.UInt8ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "testUint8ClampedArraySetIntIr"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt8ClampedArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.UInt8ClampedArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! INST          /LoadObject.*escompat\.Uint[\d]+ClampedArray\.buffer/
//! INST          /LoadObject.*escompat\.Uint[\d]+ClampedArray\.lengthInt/
//! INST          /LoadObject.*escompat\.Uint[\d]+ClampedArray\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "testUint16ArraySetIntIr"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt16ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.UInt16ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "testUint32ArraySetIntIr"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt32ArraySetInt "
//! INST_NOT      "Intrinsic.UInt32ArraySetLong "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.UInt32ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "testUint32ArraySetLongIr"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.UInt32ArraySetLong "
//! INST_NOT      "Intrinsic.UInt32ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.UInt32ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.Uint[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "testBigUint64ArraySetIntIr"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.BigUInt64ArraySetInt "
//! INST_NOT      "Intrinsic.BigUInt64ArraySetLong "
//! INST_NOT      /Call.*Array::\$_set/
//! INST_NOT      "Intrinsic.BigUInt64ArrayGet "
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/
//!
//! METHOD        "testBigUint64ArraySetLongIr"
//! METHOD        "escompat_BigUint64Array::__set"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.BigUInt64ArraySetLong "
//! INST_NOT      "Intrinsic.BigUInt64ArraySetInt "
//! INST_NOT      /Call.*Array::\$_set/
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! METHOD        "escompat_BigUint64Array::__get"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      "Intrinsic.BigUInt64ArrayGet "
//! INST_NOT      /Call.*Array::\$_get/
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.buffer/
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.lengthInt/
//! INST          /LoadObject.*escompat\.BigUint[\d]+Array\.byteOffset/
//! INST          /LoadObject.*std\.core\.ArrayBuffer\.data/
//! EVENT_NOT     /Deoptimization/

function testUint8ArraySetIntCpp(): void {
    let ar = new Uint8Array(6)
    ar[2] = -1
    ar[3] = 255
    ar[4] = 257
    arktest.assertEQ(255, ar[2])
    arktest.assertEQ(255, ar[3])
    arktest.assertEQ(1, ar[4])
}

function testUint8ArraySetRangeErrorCpp(): void {
    let ar = new Uint8Array(6)
    arktest.expectThrow(() => { ar[11] = 1 }, expectedRangeErrors)
}

function testUint8ArraySetIntIr(): void {
    let ar = new Uint8Array(6)
    ar[2] = -1
    ar[3] = 255
    ar[4] = 257
    arktest.assertEQ(255, ar[2])
    arktest.assertEQ(255, ar[3])
    arktest.assertEQ(1, ar[4])
}

function testUint8ClampedArraySetIntCpp(): void {
    let ar = new Uint8ClampedArray(6)
    ar.fill(77)
    ar[2] = -1
    ar[3] = 255
    ar[4] = 256
    arktest.assertEQ(0, ar[2])
    arktest.assertEQ(255, ar[3])
    arktest.assertEQ(255, ar[4])
}

function testUint8ClampedArraySetRangeErrorCpp(): void {
    let ar = new Uint8ClampedArray(6)
    arktest.expectThrow(() => { ar[11] = 1 }, expectedRangeErrors)
}

function testUint8ClampedArraySetIntIr(): void {
    let ar = new Uint8ClampedArray(6)
    ar.fill(77)
    ar[2] = -1
    ar[3] = 255
    ar[4] = 256
    arktest.assertEQ(0, ar[2])
    arktest.assertEQ(255, ar[3])
    arktest.assertEQ(255, ar[4])
}

function testUint16ArraySetIntCpp(): void {
    let ar = new Uint16Array(6)
    ar[2] = -1
    ar[3] = 65535
    ar[4] = 65537
    arktest.assertEQ(65535, ar[2])
    arktest.assertEQ(65535, ar[3])
    arktest.assertEQ(1, ar[4])
}

function testUint16ArraySetRangeErrorCpp(): void {
    let ar = new Uint16Array(6)
    arktest.expectThrow(() => { ar[11] = 1 }, expectedRangeErrors)
}

function testUint16ArraySetIntIr(): void {
    let ar = new Uint16Array(6)
    ar[2] = -1
    ar[3] = 65535
    ar[4] = 65537
    arktest.assertEQ(65535, ar[2])
    arktest.assertEQ(65535, ar[3])
    arktest.assertEQ(1, ar[4])
}


function testUint32ArraySetIntCpp(): void {
    let ar = new Uint32Array(6)
    ar[2] = -1
    ar[3] = 2147483647
    arktest.assertEQ(4294967295, ar[2])
    arktest.assertEQ(2147483647, ar[3])
}

function testUint32ArraySetRangeErrorCpp(): void {
    let ar = new Uint32Array(6)
    arktest.expectThrow(() => { ar[11] = 1 }, expectedRangeErrors)
}

function testUint32ArraySetIntIr(): void {
    let ar = new Uint32Array(6)
    ar[2] = -1
    ar[3] = 2147483647
    arktest.assertEQ(4294967295, ar[2])
    arktest.assertEQ(2147483647, ar[3])
}

function testUint32ArraySetLongCpp(): void {
    let ar = new Uint32Array(6)
    ar[2] = (-1).toLong()
    ar[3] = 4294967295
    ar[4] = 4294967297
    arktest.assertEQ(4294967295, ar[2])
    arktest.assertEQ(4294967295, ar[3])
    arktest.assertEQ(1, ar[4])
}

function testUint32ArraySetLongIr(): void {
    let ar = new Uint32Array(6)
    ar[2] = (-1).toLong()
    ar[3] = 4294967295
    ar[4] = 4294967297
    arktest.assertEQ(4294967295, ar[2])
    arktest.assertEQ(4294967295, ar[3])
    arktest.assertEQ(1, ar[4])
}

function testBigUint64ArraySetIntCpp(): void {
    let ar = new BigUint64Array(6)
    ar[2] = -1
    ar[3] = 2147483647
    arktest.assertEQ(18446744073709551615n, ar[2])
    arktest.assertEQ(2147483647n, ar[3])
}

function testBigUint64ArraySetIntIr(): void {
    let ar = new BigUint64Array(6)
    ar[2] = -1
    ar[3] = 2147483647
    arktest.assertEQ(18446744073709551615n, ar[2])
    arktest.assertEQ(2147483647n, ar[3])
}

function testBigUint64ArraySetLongCpp(): void {
    let ar = new BigUint64Array(6)
    ar[2] = 18446744073709551618n
    ar[3] = -18446744073709551618n
    ar[4] = -1n
    ar[5] = 18446744073709551615n
    arktest.assertEQ(2n, ar[2])
    arktest.assertEQ(18446744073709551614n, ar[3])
    arktest.assertEQ(18446744073709551615n, ar[4])
    arktest.assertEQ(18446744073709551615n, ar[5])
}

function testBigUint64ArraySetRangeErrorCpp(): void {
    let ar = new BigUint64Array(6)
    arktest.expectThrow(() => { ar[11] = 1n }, expectedRangeErrors)
}

function testBigUint64ArraySetLongIr(): void {
    let ar = new BigUint64Array(6)
    ar[2] = 18446744073709551618n
    ar[3] = -18446744073709551618n
    ar[4] = -1n
    ar[5] = 18446744073709551615n
    arktest.assertEQ(2n, ar[2])
    arktest.assertEQ(18446744073709551614n, ar[3])
    arktest.assertEQ(18446744073709551615n, ar[4])
    arktest.assertEQ(18446744073709551615n, ar[5])
}

// Helpers
const expectedRangeErrors = (e: Error): boolean => {
    return (e instanceof RangeError)
}

function main() {
    testUint8ArraySetIntCpp()
    testUint8ArraySetRangeErrorCpp()
    testUint8ArraySetIntIr()

    testUint8ClampedArraySetIntCpp()
    testUint8ClampedArraySetRangeErrorCpp()
    testUint8ClampedArraySetIntIr()

    testUint16ArraySetIntCpp()
    testUint16ArraySetRangeErrorCpp()
    testUint16ArraySetIntIr()

    testUint32ArraySetIntCpp()
    testUint32ArraySetRangeErrorCpp()
    testUint32ArraySetIntIr()
    testUint32ArraySetLongCpp()
    testUint32ArraySetLongIr()

    testBigUint64ArraySetIntCpp()
    testBigUint64ArraySetIntIr()
    testBigUint64ArraySetLongCpp()
    testBigUint64ArraySetRangeErrorCpp()
    testBigUint64ArraySetLongIr()
}
