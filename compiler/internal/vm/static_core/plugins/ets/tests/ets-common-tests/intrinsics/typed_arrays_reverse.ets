/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

function test_iu8_array(): void {
    let data = Int8Array.of(-128, -42, -14, 0, 16, 48, 127)
    let data_rev = new Int8Array(data).reverse()
    let data_rev2 = data.toReversed()
    check_array(data, data_rev, data_rev2)

    let udata = Uint8Array.of(0, 16, 48, 127, 160, 255)
    let udata_rev = new Uint8Array(udata).reverse()
    let udata_rev2 = udata.toReversed()
    check_array(udata, udata_rev, udata_rev2)
}

function test_iu16_array(): void {
    let data = Int16Array.of(-32768, -2731, -227, 0, 128, 2564, 32767)
    let data_rev = new Int16Array(data).reverse()
    let data_rev2 = data.toReversed()
    check_array(data, data_rev, data_rev2)

    let udata = Uint16Array.of(0, 128, 2564, 32767, 65535)
    let udata_rev = new Uint16Array(udata).reverse()
    let udata_rev2 = udata.toReversed()
    check_array(udata, udata_rev, udata_rev2)
}

function test_iu32_array(): void {
    let data = Int32Array.of(-2147483648, -1398, 0, 11184, 89478, 536872, 2147483647)
    let data_rev = new Int32Array(data).reverse()
    let data_rev2 = data.toReversed()
    check_array(data, data_rev, data_rev2)

    let udata = Uint32Array.of(0, 11184, 89478, 536872, 2147483647)
    let udata_rev = new Uint32Array(udata).reverse()
    let udata_rev2 = udata.toReversed()
    check_array(udata, udata_rev, udata_rev2)
    
}

function test_iu64_array(): void {
    let data = BigInt64Array.of(-9.223e+18, -8589934588, 0, 4294967294, 2.3058e+16, 9.223e+18)
    let data_rev = new BigInt64Array(data).reverse()
    let data_rev2 = data.toReversed()
    check_array(data, data_rev, data_rev2)

    let udata = BigUint64Array.of(0, 4294967294, 2.3058e+16, 9.223e+18, 18.446e+18)
    let udata_rev = new BigUint64Array(udata).reverse()
    let udata_rev2 = udata.toReversed()
    check_array(udata, udata_rev, udata_rev2)
}

function test_f32_array(): void {
    let data = Float32Array.of(-3.4e+38, -2.02e+16, 1.06e+12, 2.8e+24, 3.4e+38)
    let data_rev = new Float32Array(data).reverse()
    let data_rev2 = data.toReversed()
    check_array(data, data_rev, data_rev2)
}

function test_f64_array(): void {
    let data = Float64Array.of(-1.7e+308, -3.6e+42, -0.6e+16, 0.4e+105, 1.7e+308)
    let data_rev = new Float64Array(data).reverse()
    let data_rev2 = data.toReversed()
    check_array(data, data_rev, data_rev2)
}


function check_array<T extends Number | BigInt>(data: ArrayLike<T>, dat_r: ArrayLike<T>, dat_r2: ArrayLike<T>): void {
    let size = data.length - 1
    arktest.assertEQ(data.length, dat_r.length, "reverse(): bad array len")
    arktest.assertEQ(data.length, dat_r2.length, "toReversed(): bad array len")

    for (let i: int = 0; i <= size; i++) {
        arktest.assertEQ(data[i], dat_r[size - i], "reverse(): unexpected element: i: " + i)
        arktest.assertEQ(data[i], dat_r2[size - i], "toReversed(): unexpected element: i: " + i)
    }
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite("typedArrays.reverse");
    testSuite.addTest("Int8,Uint8 Arrays: reverse test", test_iu8_array);
    testSuite.addTest("Int16,Uint16 Arrays: reverse test", test_iu16_array)
    testSuite.addTest("Int32,Uint32 Arrays: reverse test", test_iu32_array)
    testSuite.addTest("BigInt64,BigUint64 Arrays: reverse test", test_iu64_array)
    testSuite.addTest("Float32 Arrays: reverse test", test_f32_array)
    testSuite.addTest("Float64 Arrays: reverse test", test_f64_array)
    return testSuite.run()
}
