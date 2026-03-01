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

function int8ArraySliceTest() {
    let arr = Int8Array.from([1, 2, 3, 4, 5, 6]);
    let sub = arr.subarray(2);
    let newArr = sub.slice(1, 3);
    arktest.assertEQ(arr.toString(), '1,2,3,4,5,6')
    arktest.assertEQ(sub.toString(), '3,4,5,6');
    arktest.assertEQ(newArr.toString(), '4,5');
    arr[4] = 99;
    arktest.assertEQ(arr.toString(), '1,2,3,4,99,6')
    arktest.assertEQ(sub.toString(), '3,4,99,6');
    arktest.assertEQ(newArr.toString(), '4,5');
}

function int16ArraySliceTest() {
    let arr = Int16Array.from([1, 2, 3, 4, 5, 6]);
    let sub = arr.subarray(2);
    let newArr = sub.slice(1, 3);
    arktest.assertEQ(arr.toString(), '1,2,3,4,5,6')
    arktest.assertEQ(sub.toString(), '3,4,5,6');
    arktest.assertEQ(newArr.toString(), '4,5');
    arr[4] = 99;
    arktest.assertEQ(arr.toString(), '1,2,3,4,99,6')
    arktest.assertEQ(sub.toString(), '3,4,99,6');
    arktest.assertEQ(newArr.toString(), '4,5');
}

function int32ArraySliceTest() {
    let arr = Int32Array.from([1, 2, 3, 4, 5, 6]);
    let sub = arr.subarray(2);
    let newArr = sub.slice(1, 3);
    arktest.assertEQ(arr.toString(), '1,2,3,4,5,6')
    arktest.assertEQ(sub.toString(), '3,4,5,6');
    arktest.assertEQ(newArr.toString(), '4,5');
    arr[4] = 99;
    arktest.assertEQ(arr.toString(), '1,2,3,4,99,6')
    arktest.assertEQ(sub.toString(), '3,4,99,6');
    arktest.assertEQ(newArr.toString(), '4,5');
}

function float32ArraySliceTest() {
    let arr = Float32Array.from([1, 2, 3, 4, 5, 6]);
    let sub = arr.subarray(2);
    let newArr = sub.slice(1, 3);
    arktest.assertEQ(arr.toString(), '1,2,3,4,5,6')
    arktest.assertEQ(sub.toString(), '3,4,5,6');
    arktest.assertEQ(newArr.toString(), '4,5');
    arr[4] = 99;
    arktest.assertEQ(arr.toString(), '1,2,3,4,99,6')
    arktest.assertEQ(sub.toString(), '3,4,99,6');
    arktest.assertEQ(newArr.toString(), '4,5');
}

function float64ArraySliceTest() {
    let arr = Float64Array.from([1, 2, 3, 4, 5, 6]);
    let sub = arr.subarray(2);
    let newArr = sub.slice(1, 3);
    arktest.assertEQ(arr.toString(), '1,2,3,4,5,6')
    arktest.assertEQ(sub.toString(), '3,4,5,6');
    arktest.assertEQ(newArr.toString(), '4,5');
    arr[4] = 99;
    arktest.assertEQ(arr.toString(), '1,2,3,4,99,6')
    arktest.assertEQ(sub.toString(), '3,4,99,6');
    arktest.assertEQ(newArr.toString(), '4,5');
}

function bigInt64ArraySliceTest() {
    let arr = BigInt64Array.from([1n, 2n, 3n, 4n, 5n, 6n]);
    let sub = arr.subarray(2);
    let newArr = sub.slice(1, 3);
    arktest.assertEQ(arr.toString(), '1,2,3,4,5,6')
    arktest.assertEQ(sub.toString(), '3,4,5,6');
    arktest.assertEQ(newArr.toString(), '4,5');
    arr[4] = 99n;
    arktest.assertEQ(arr.toString(), '1,2,3,4,99,6')
    arktest.assertEQ(sub.toString(), '3,4,99,6');
    arktest.assertEQ(newArr.toString(), '4,5');
}

function uint8ClampedArraySliceTest() {
    let arr = Uint8ClampedArray.from([1, 2, 3, 4, 5, 6]);
    let sub = arr.subarray(2);
    let newArr = sub.slice(1, 3);
    arktest.assertEQ(arr.toString(), '1,2,3,4,5,6')
    arktest.assertEQ(sub.toString(), '3,4,5,6');
    arktest.assertEQ(newArr.toString(), '4,5');
    arr[4] = 99;
    arktest.assertEQ(arr.toString(), '1,2,3,4,99,6')
    arktest.assertEQ(sub.toString(), '3,4,99,6');
    arktest.assertEQ(newArr.toString(), '4,5');
}

function uint8ArraySliceTest() {
    let arr = Uint8Array.from([1, 2, 3, 4, 5, 6]);
    let sub = arr.subarray(2);
    let newArr = sub.slice(1, 3);
    arktest.assertEQ(arr.toString(), '1,2,3,4,5,6')
    arktest.assertEQ(sub.toString(), '3,4,5,6');
    arktest.assertEQ(newArr.toString(), '4,5');
    arr[4] = 99;
    arktest.assertEQ(arr.toString(), '1,2,3,4,99,6')
    arktest.assertEQ(sub.toString(), '3,4,99,6');
    arktest.assertEQ(newArr.toString(), '4,5');
}

function uint16ArraySliceTest() {
    let arr = Uint16Array.from([1, 2, 3, 4, 5, 6]);
    let sub = arr.subarray(2);
    let newArr = sub.slice(1, 3);
    arktest.assertEQ(arr.toString(), '1,2,3,4,5,6')
    arktest.assertEQ(sub.toString(), '3,4,5,6');
    arktest.assertEQ(newArr.toString(), '4,5');
    arr[4] = 99;
    arktest.assertEQ(arr.toString(), '1,2,3,4,99,6')
    arktest.assertEQ(sub.toString(), '3,4,99,6');
    arktest.assertEQ(newArr.toString(), '4,5');
}

function uint32ArraySliceTest() {
    let arr = Uint32Array.from([1, 2, 3, 4, 5, 6]);
    let sub = arr.subarray(2);
    let newArr = sub.slice(1, 3);
    arktest.assertEQ(arr.toString(), '1,2,3,4,5,6')
    arktest.assertEQ(sub.toString(), '3,4,5,6');
    arktest.assertEQ(newArr.toString(), '4,5');
    arr[4] = 99;
    arktest.assertEQ(arr.toString(), '1,2,3,4,99,6')
    arktest.assertEQ(sub.toString(), '3,4,99,6');
    arktest.assertEQ(newArr.toString(), '4,5');
}

function bigUint64ArraySliceTest() {
    let arr = BigUint64Array.from([1n, 2n, 3n, 4n, 5n, 6n]);
    let sub = arr.subarray(2);
    let newArr = sub.slice(1, 3);
    arktest.assertEQ(arr.toString(), '1,2,3,4,5,6')
    arktest.assertEQ(sub.toString(), '3,4,5,6');
    arktest.assertEQ(newArr.toString(), '4,5');
    arr[4] = 99n;
    arktest.assertEQ(arr.toString(), '1,2,3,4,99,6')
    arktest.assertEQ(sub.toString(), '3,4,99,6');
    arktest.assertEQ(newArr.toString(), '4,5');
}

function main(): int {
    const suite = new arktest.ArkTestsuite('TypedArray and TypedUArray slice tests');
    suite.addTest('for Int8Array', int8ArraySliceTest);
    suite.addTest('for Int16Array', int16ArraySliceTest);
    suite.addTest('for Int32Array', int32ArraySliceTest);
    suite.addTest('for Float32Array', float32ArraySliceTest);
    suite.addTest('for Float64Array', float64ArraySliceTest);
    suite.addTest('for BigInt64Array', bigInt64ArraySliceTest);
    suite.addTest('for Uint8ClampedArray', uint8ClampedArraySliceTest);
    suite.addTest('for Uint8Array', uint8ArraySliceTest);
    suite.addTest('for Uint16Array', uint16ArraySliceTest);
    suite.addTest('for Uint32Array', uint32ArraySliceTest);
    suite.addTest('for BigUint64Array', bigUint64ArraySliceTest);
    return suite.run();
}
