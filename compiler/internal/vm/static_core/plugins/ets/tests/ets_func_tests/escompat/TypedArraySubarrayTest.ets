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

function int8ArraySubarrayTest() {
    let arr = Int8Array.from([1, 2, 3, 4, 5, 6]);
    let sub1 = arr.subarray(2);
    let sub2 = sub1.subarray(2);
    arktest.assertEQ(arr.toString(), '1,2,3,4,5,6')
    arktest.assertEQ(sub1.toString(), '3,4,5,6');
    arktest.assertEQ(sub2.toString(), '5,6');
    arr[4] = 99;
    arktest.assertEQ(arr.toString(), '1,2,3,4,99,6')
    arktest.assertEQ(sub1.toString(), '3,4,99,6');
    arktest.assertEQ(sub2.toString(), '99,6');
}

function int16ArraySubarrayTest() {
    let arr = Int16Array.from([1, 2, 3, 4, 5, 6]);
    let sub1 = arr.subarray(2);
    let sub2 = sub1.subarray(2);
    arktest.assertEQ(arr.toString(), '1,2,3,4,5,6')
    arktest.assertEQ(sub1.toString(), '3,4,5,6');
    arktest.assertEQ(sub2.toString(), '5,6');
    arr[4] = 99;
    arktest.assertEQ(arr.toString(), '1,2,3,4,99,6')
    arktest.assertEQ(sub1.toString(), '3,4,99,6');
    arktest.assertEQ(sub2.toString(), '99,6');
}

function int32ArraySubarrayTest() {
    let arr = Int32Array.from([1, 2, 3, 4, 5, 6]);
    let sub1 = arr.subarray(2);
    let sub2 = sub1.subarray(2);
    arktest.assertEQ(arr.toString(), '1,2,3,4,5,6')
    arktest.assertEQ(sub1.toString(), '3,4,5,6');
    arktest.assertEQ(sub2.toString(), '5,6');
    arr[4] = 99;
    arktest.assertEQ(arr.toString(), '1,2,3,4,99,6')
    arktest.assertEQ(sub1.toString(), '3,4,99,6');
    arktest.assertEQ(sub2.toString(), '99,6');
}

function float32ArraySubarrayTest() {
    let arr = Float32Array.from([1, 2, 3, 4, 5, 6]);
    let sub1 = arr.subarray(2);
    let sub2 = sub1.subarray(2);
    arktest.assertEQ(arr.toString(), '1,2,3,4,5,6')
    arktest.assertEQ(sub1.toString(), '3,4,5,6');
    arktest.assertEQ(sub2.toString(), '5,6');
    arr[4] = 99;
    arktest.assertEQ(arr.toString(), '1,2,3,4,99,6')
    arktest.assertEQ(sub1.toString(), '3,4,99,6');
    arktest.assertEQ(sub2.toString(), '99,6');
}

function float64ArraySubarrayTest() {
    let arr = Float64Array.from([1, 2, 3, 4, 5, 6]);
    let sub1 = arr.subarray(2);
    let sub2 = sub1.subarray(2);
    arktest.assertEQ(arr.toString(), '1,2,3,4,5,6')
    arktest.assertEQ(sub1.toString(), '3,4,5,6');
    arktest.assertEQ(sub2.toString(), '5,6');
    arr[4] = 99;
    arktest.assertEQ(arr.toString(), '1,2,3,4,99,6')
    arktest.assertEQ(sub1.toString(), '3,4,99,6');
    arktest.assertEQ(sub2.toString(), '99,6');
}

function bigInt64ArraySubarrayTest() {
    let arr = BigInt64Array.from([1n, 2n, 3n, 4n, 5n, 6n]);
    let sub1 = arr.subarray(2);
    let sub2 = sub1.subarray(2);
    arktest.assertEQ(arr.toString(), '1,2,3,4,5,6')
    arktest.assertEQ(sub1.toString(), '3,4,5,6');
    arktest.assertEQ(sub2.toString(), '5,6');
    arr[4] = 99n;
    arktest.assertEQ(arr.toString(), '1,2,3,4,99,6')
    arktest.assertEQ(sub1.toString(), '3,4,99,6');
    arktest.assertEQ(sub2.toString(), '99,6');
}

function uint8ClampedArraySubarrayTest() {
    let arr = Uint8ClampedArray.from([1, 2, 3, 4, 5, 6]);
    let sub1 = arr.subarray(2);
    let sub2 = sub1.subarray(2);
    arktest.assertEQ(arr.toString(), '1,2,3,4,5,6')
    arktest.assertEQ(sub1.toString(), '3,4,5,6');
    arktest.assertEQ(sub2.toString(), '5,6');
    arr[4] = 99;
    arktest.assertEQ(arr.toString(), '1,2,3,4,99,6')
    arktest.assertEQ(sub1.toString(), '3,4,99,6');
    arktest.assertEQ(sub2.toString(), '99,6');
}

function uint8ArraySubarrayTest() {
    let arr = Uint8Array.from([1, 2, 3, 4, 5, 6]);
    let sub1 = arr.subarray(2);
    let sub2 = sub1.subarray(2);
    arktest.assertEQ(arr.toString(), '1,2,3,4,5,6')
    arktest.assertEQ(sub1.toString(), '3,4,5,6');
    arktest.assertEQ(sub2.toString(), '5,6');
    arr[4] = 99;
    arktest.assertEQ(arr.toString(), '1,2,3,4,99,6')
    arktest.assertEQ(sub1.toString(), '3,4,99,6');
    arktest.assertEQ(sub2.toString(), '99,6');
}

function uint16ArraySubarrayTest() {
    let arr = Uint16Array.from([1, 2, 3, 4, 5, 6]);
    let sub1 = arr.subarray(2);
    let sub2 = sub1.subarray(2);
    arktest.assertEQ(arr.toString(), '1,2,3,4,5,6')
    arktest.assertEQ(sub1.toString(), '3,4,5,6');
    arktest.assertEQ(sub2.toString(), '5,6');
    arr[4] = 99;
    arktest.assertEQ(arr.toString(), '1,2,3,4,99,6')
    arktest.assertEQ(sub1.toString(), '3,4,99,6');
    arktest.assertEQ(sub2.toString(), '99,6');
}

function uint32ArraySubarrayTest() {
    let arr = Uint32Array.from([1, 2, 3, 4, 5, 6]);
    let sub1 = arr.subarray(2);
    let sub2 = sub1.subarray(2);
    arktest.assertEQ(arr.toString(), '1,2,3,4,5,6')
    arktest.assertEQ(sub1.toString(), '3,4,5,6');
    arktest.assertEQ(sub2.toString(), '5,6');
    arr[4] = 99;
    arktest.assertEQ(arr.toString(), '1,2,3,4,99,6')
    arktest.assertEQ(sub1.toString(), '3,4,99,6');
    arktest.assertEQ(sub2.toString(), '99,6');
}

function bigUint64ArraySubarrayTest() {
    let arr = BigUint64Array.from([1n, 2n, 3n, 4n, 5n, 6n]);
    let sub1 = arr.subarray(2);
    let sub2 = sub1.subarray(2);
    arktest.assertEQ(arr.toString(), '1,2,3,4,5,6')
    arktest.assertEQ(sub1.toString(), '3,4,5,6');
    arktest.assertEQ(sub2.toString(), '5,6');
    arr[4] = 99n;
    arktest.assertEQ(arr.toString(), '1,2,3,4,99,6')
    arktest.assertEQ(sub1.toString(), '3,4,99,6');
    arktest.assertEQ(sub2.toString(), '99,6');
}

function main(): int {
    const suite = new arktest.ArkTestsuite('TypedArray and TypedUArray subarray tests');
    suite.addTest('for Int8Array', int8ArraySubarrayTest);
    suite.addTest('for Int16Array', int16ArraySubarrayTest);
    suite.addTest('for Int32Array', int32ArraySubarrayTest);
    suite.addTest('for Float32Array', float32ArraySubarrayTest);
    suite.addTest('for Float64Array', float64ArraySubarrayTest);
    suite.addTest('for BigInt64Array', bigInt64ArraySubarrayTest);
    suite.addTest('for Uint8ClampedArray', uint8ClampedArraySubarrayTest);
    suite.addTest('for Uint8Array', uint8ArraySubarrayTest);
    suite.addTest('for Uint16Array', uint16ArraySubarrayTest);
    suite.addTest('for Uint32Array', uint32ArraySubarrayTest);
    suite.addTest('for BigUint64Array', bigUint64ArraySubarrayTest);
    return suite.run();
}
