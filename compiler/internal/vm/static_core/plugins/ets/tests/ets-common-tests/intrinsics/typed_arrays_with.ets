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

const expectedRangeErrors = (e: Error): boolean => {
    return (e instanceof RangeError) || (e instanceof IndexOutOfBoundsError)
}

function check_array<T extends Number | BigInt>(data: ArrayLike<T>, expected: ArrayLike<T>): void {
    arktest.assertEQ(data.length, expected.length, "Unexpected array length")
    for (let i: int = 0; i < data.length; i++) {
        arktest.assertEQ(data[i], expected[i], "Unexpected element with index " + i)
    }
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite("typedArrays.with")
    testSuite.addTest("UInt8ClampedArrayWithTest", test_u8_clamped_array);
    testSuite.addTest("UInt8ClampedArrayWithTestRangeError", test_u8_clamped_array_rangeerror);
    testSuite.addTest("Int8ArrayWithTest", test_i8_array);
    testSuite.addTest("Int8ArrayWithTestRangeError", test_i8_array_rangeerror);
    testSuite.addTest("UInt8ArrayWithTest", test_u8_array);
    testSuite.addTest("UInt8ArrayWithTestRangeError", test_u8_array_rangeerror);
    testSuite.addTest("Int16ArrayWithTest", test_i16_array);
    testSuite.addTest("Int16ArrayWithTestRangeError", test_i16_array_rangeerror);
    testSuite.addTest("UInt16ArrayWithTest", test_u16_array);
    testSuite.addTest("UInt16ArrayWithTestRangeError", test_u16_array_rangeerror);
    testSuite.addTest("Int32ArrayWithTest", test_i32_array);
    testSuite.addTest("Int32ArrayWithTestRangeError", test_i32_array_rangeerror);
    testSuite.addTest("UInt32ArrayWithTest", test_u32_array);
    testSuite.addTest("UInt32ArrayWithTestRangeError", test_u32_array_rangeerror);
    testSuite.addTest("BigInt64ArrayWithTest", test_i64_array);
    testSuite.addTest("BigInt64ArrayWithTestRangeError", test_i64_array_rangeerror);
    testSuite.addTest("BigUInt64ArrayWithTest", test_u64_array);
    testSuite.addTest("BigUInt64ArrayWithTestRangeError", test_u64_array_rangeerror);
    testSuite.addTest("Float32ArrayWithTest", test_f32_array);
    testSuite.addTest("Float32ArrayWithTestRangeError", test_f32_array_rangeerror);
    testSuite.addTest("Float64ArrayWithTest", test_f64_array);
    testSuite.addTest("Float64ArrayWithTestRangeError", test_f64_array_rangeerror);
    return testSuite.run()
}

function test_u8_clamped_array(): void {
    let src  = new Uint8ClampedArray([0, 7, 74, 181, 255])
    let exp1 = new Uint8ClampedArray([0, 0, 74, 181, 255])
    let exp2 = new Uint8ClampedArray([0, 255, 74, 181, 255])
    let exp3 = new Uint8ClampedArray([0, 7, 74, 181, 255])
    let val1: int = -1;
    let val2: int = 256;
    check_array(src.with(1 as int, val1), exp1)
    check_array(src.with(1 as int, val2), exp2)
    check_array(src, exp3)
}

function test_u8_clamped_array_rangeerror() {
    let src = new Uint8ClampedArray([1, 1, 1])
    let val: int = 0;
    arktest.expectThrow(() => { src.with(-1 as int, val) }, expectedRangeErrors)
    arktest.expectThrow(() => { src.with(src.length.toInt(), val) }, expectedRangeErrors)
}

function test_i8_array(): void {
    let src  = new Int8Array([-128, -56, 0, 64, 128])
    let exp1 = new Int8Array([-128, 100, 0, 64, 128])
    let exp2 = new Int8Array([-128, -56, 0, 64, 128])
    let val: byte = 100
    check_array(src.with(1, val), exp1)
    check_array(src, exp2)
}

function test_i8_array_rangeerror() {
    let src = new Int8Array([1, 1, 1])
    let val: byte = 0;
    arktest.expectThrow(() => { src.with(-1 as int, val) }, expectedRangeErrors)
    arktest.expectThrow(() => { src.with(src.length.toInt(), val) }, expectedRangeErrors)
}

function test_u8_array(): void {
    let src  = new Uint8Array([0, 7, 74, 181, 255])
    let exp1 = new Uint8Array([0, 4, 74, 181, 255])
    let exp2 = new Uint8Array([0, 7, 74, 181, 255])
    let val: int = 260
    check_array(src.with(1, val), exp1)
    check_array(src, exp2)
}

function test_u8_array_rangeerror() {
    let src = new Uint8Array([1, 1, 1])
    let val: int = 0;
    arktest.expectThrow(() => { src.with(-1 as int, val) }, expectedRangeErrors)
    arktest.expectThrow(() => { src.with(src.length.toInt(), val) }, expectedRangeErrors)
}

function test_i16_array(): void {
    let src  = new Int16Array([-32768, -1321, 0, 10321, 32767])
    let exp1 = new Int16Array([-32768, 10000, 0, 10321, 32767])
    let exp2 = new Int16Array([-32768, -1321, 0, 10321, 32767])
    let val: short = 10000;
    check_array(src.with(1, val), exp1)
    check_array(src, exp2)
}

function test_i16_array_rangeerror() {
    let src = new Int16Array([1, 1, 1])
    let val: short = 0;
    arktest.expectThrow(() => { src.with(-1 as int, val) }, expectedRangeErrors)
    arktest.expectThrow(() => { src.with(src.length.toInt(), val) }, expectedRangeErrors)
}

function test_u16_array(): void {
    let src  = new Uint16Array([0, 62, 538, 35891, 65535])
    let exp1 = new Uint16Array([0, 14, 538, 35891, 65535])
    let exp2 = new Uint16Array([0, 62, 538, 35891, 65535])
    let val: int = 65550;
    check_array(src.with(1, val), exp1)
    check_array(src, exp2)
}

function test_u16_array_rangeerror() {
    let src = new Uint16Array([1, 1, 1])
    let val: int = 0;
    arktest.expectThrow(() => { src.with(-1 as int, val) }, expectedRangeErrors)
    arktest.expectThrow(() => { src.with(src.length.toInt(), val) }, expectedRangeErrors)
}

function test_i32_array(): void {
    let src  = new Int32Array([-2147483648, -20543, 0, 20543, 2147483647])
    let exp1 = new Int32Array([-2147483648, 100000, 0, 20543, 2147483647])
    let exp2 = new Int32Array([-2147483648, -20543, 0, 20543, 2147483647])
    let val: int = 100000;
    check_array(src.with(1, val), exp1)
    check_array(src, exp2)
}

function test_i32_array_rangeerror() {
    let src = new Int32Array([1, 1, 1])
    let val: int = 0;
    arktest.expectThrow(() => { src.with(-1 as int, val) }, expectedRangeErrors)
    arktest.expectThrow(() => { src.with(src.length.toInt(), val) }, expectedRangeErrors)
}

function test_u32_array(): void {
    let src  = new Uint32Array([0, 25974, 57356, 2054306, 4294967295])
    let exp1 = new Uint32Array([0, 10004, 57356, 2054306, 4294967295])
    let exp2 = new Uint32Array([0, 25974, 57356, 2054306, 4294967295])
    let val: long = 4294977300;
    check_array(src.with(1, val), exp1)
    check_array(src, exp2)
}

function test_u32_array_rangeerror() {
    let src = new Uint32Array([1, 1, 1])
    let val: long = 0;
    arktest.expectThrow(() => { src.with(-1 as int, val) }, expectedRangeErrors)
    arktest.expectThrow(() => { src.with(src.length.toInt(), val) }, expectedRangeErrors)
}

function test_i64_array(): void {
    let src  = new BigInt64Array([-9.223e+18, -404221, 0, 404221, 9.223e+18])
    let exp1 = new BigInt64Array([-9.223e+18, 1000000, 0, 404221, 9.223e+18])
    let exp2 = new BigInt64Array([-9.223e+18, -404221, 0, 404221, 9.223e+18])
    let val: long = 1000000;
    check_array(src.with(1, val), exp1)
    check_array(src, exp2)
}

function test_i64_array_rangeerror() {
    let src = new BigInt64Array([1, 1, 1])
    let val: long = 0;
    arktest.expectThrow(() => { src.with(-1 as int, val) }, expectedRangeErrors)
    arktest.expectThrow(() => { src.with(src.length.toInt(), val) }, expectedRangeErrors)
}

function test_u64_array(): void {
    let src  = new BigUint64Array([0, 2598574, 4042214, 6.446e+11, 18.446e+18])
    let exp1 = new BigUint64Array([0, 1000000, 4042214, 6.446e+11, 18.446e+18])
    let exp2 = new BigUint64Array([0, 2598574, 4042214, 6.446e+11, 18.446e+18])
    let val: long = 1000000;
    check_array(src.with(1, val), exp1)
    check_array(src, exp2)
}

function test_u64_array_rangeerror() {
    let src = new BigUint64Array([1, 1, 1])
    let val: long = 0;
    arktest.expectThrow(() => { src.with(-1 as int, val) }, expectedRangeErrors)
    arktest.expectThrow(() => { src.with(src.length.toInt(), val) }, expectedRangeErrors)
}

function test_f32_array(): void {
    let src  = new Float32Array([-3.4e+38, -1.5e+16, 0.0, 1.5e+16, 3.4e+38])
    let exp1 = new Float32Array([-3.4e+38, 5.05e+16, 0.0, 1.5e+16, 3.4e+38])
    let exp2 = new Float32Array([-3.4e+38, -1.5e+16, 0.0, 1.5e+16, 3.4e+38])
    let val: float = 5.05e+16f;
    check_array(src.with(1, val), exp1)
    check_array(src, exp2)
}

function test_f32_array_rangeerror() {
    let src = new Float32Array([1, 1, 1])
    let val: float = 0.0f;
    arktest.expectThrow(() => { src.with(-1 as int, val) }, expectedRangeErrors)
    arktest.expectThrow(() => { src.with(src.length.toInt(), val) }, expectedRangeErrors)
}

function test_f64_array(): void {
    let src  = new Float64Array([-1.7e+308, -3.4e+72, 0.0, 3.4e+72, 1.7e+308])
    let exp1 = new Float64Array([-1.7e+308, 5.05e+32, 0.0, 3.4e+72, 1.7e+308])
    let exp2 = new Float64Array([-1.7e+308, -3.4e+72, 0.0, 3.4e+72, 1.7e+308])
    let val: double = 5.05e+32;
    check_array(src.with(1, val), exp1)
    check_array(src, exp2)
}

function test_f64_array_rangeerror() {
    let src = new Float64Array([1, 1, 1])
    let val: double = 0.0;
    arktest.expectThrow(() => { src.with(-1 as int, val) }, expectedRangeErrors)
    arktest.expectThrow(() => { src.with(src.length.toInt(), val) }, expectedRangeErrors)
}
