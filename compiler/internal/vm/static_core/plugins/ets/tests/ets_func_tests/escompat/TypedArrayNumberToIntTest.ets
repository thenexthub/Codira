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

const fixedArraySource: FixedArray<number> = [NaN, Infinity, -Infinity, -2147483649, 2147483648];
const arraySource: Array<number> = [NaN, Infinity, -Infinity, -2147483649, 2147483648];
const arrayLikeSource: ArrayLike<Number> | ArrayBuffer = Array.of<number>(NaN, Infinity, -Infinity, -2147483649, 2147483648);
const result1: FixedArray<int> = [0, 0, 0, -1, 0];
const result2: FixedArray<int> = [0, 0, 0, 2147483647, -2147483648];
const result3: FixedArray<int> = [0, 0, 0, 255, 0];
const result4: FixedArray<int> = [0, 0, 0, 65535, 0];
const result5: FixedArray<long> = [0, 0, 0, 2147483647, 2147483648];

function testInt8Array(): void {
    let arr1 = new Int8Array(fixedArraySource);
    let arr2 = new Int8Array(arraySource);
    let arr3 = new Int8Array(arrayLikeSource);
    let arr4 = new Int8Array(arraySource.length);
    let arr5 = new Int8Array(arraySource.length);
    let arr6 = new Int8Array(arraySource.length);
    arr6.set(arraySource);
    let arr7 = Int8Array.from(arraySource.$_iterator());
    let arr8 = Int8Array.from<number>(arraySource, (v: number, k: number) => v);
    for (let i = 0; i < arr4.length; i++) {
        arr4[i] = arraySource[i];
        arr5.fill(arraySource[i]);
        arktest.assertEQ(arr1[i], result1[i]);
        arktest.assertEQ(arr2[i], result1[i]);
        arktest.assertEQ(arr3[i], result1[i]);
        arktest.assertEQ(arr4[i], result1[i]);
        arktest.assertEQ(arr5[i], result1[i]);
        arktest.assertEQ(arr6[i], result1[i]);
        arktest.assertEQ(arr7[i], result1[i]);
        arktest.assertEQ(arr8[i], result1[i]);
    }
}

function testInt16Array(): void {
    let arr1 = new Int16Array(fixedArraySource);
    let arr2 = new Int16Array(arraySource);
    let arr3 = new Int16Array(arrayLikeSource);
    let arr4 = new Int16Array(arraySource.length);
    let arr5 = new Int16Array(arraySource.length);
    let arr6 = new Int16Array(arraySource.length);
    arr6.set(arraySource);
    let arr7 = Int16Array.from(arraySource.$_iterator());
    let arr8 = Int16Array.from<number>(arraySource, (v: number, k: number) => v);
    for (let i = 0; i < arr4.length; i++) {
        arr4[i] = arraySource[i];
        arr5.fill(arraySource[i]);
        arktest.assertEQ(arr1[i], result1[i]);
        arktest.assertEQ(arr2[i], result1[i]);
        arktest.assertEQ(arr3[i], result1[i]);
        arktest.assertEQ(arr4[i], result1[i]);
        arktest.assertEQ(arr5[i], result1[i]);
        arktest.assertEQ(arr6[i], result1[i]);
        arktest.assertEQ(arr7[i], result1[i]);
        arktest.assertEQ(arr8[i], result1[i]);
    }
}

function testInt32Array(): void {
    let arr1 = new Int32Array(fixedArraySource);
    let arr2 = new Int32Array(arraySource);
    let arr3 = new Int32Array(arrayLikeSource);
    let arr4 = new Int32Array(arraySource.length);
    let arr5 = new Int32Array(arraySource.length);
    let arr6 = new Int32Array(arraySource.length);
    arr6.set(arraySource);
    let arr7 = Int32Array.from(arraySource.$_iterator());
    let arr8 = Int32Array.from<number>(arraySource, (v: number, k: number) => v);
    for (let i = 0; i < arr4.length; i++) {
        arr4[i] = arraySource[i];
        arr5.fill(arraySource[i]);
        arktest.assertEQ(arr1[i], result2[i]);
        arktest.assertEQ(arr2[i], result2[i]);
        arktest.assertEQ(arr3[i], result2[i]);
        arktest.assertEQ(arr4[i], result2[i]);
        arktest.assertEQ(arr5[i], result2[i]);
        arktest.assertEQ(arr6[i], result2[i]);
        arktest.assertEQ(arr7[i], result2[i]);
        arktest.assertEQ(arr8[i], result2[i]);
    }
}

function testUint8Array(): void {
    let arr1 = new Uint8Array(fixedArraySource);
    let arr2 = new Uint8Array(arraySource);
    let arr3 = new Uint8Array(arrayLikeSource);
    let arr4 = new Uint8Array(arraySource.length);
    let arr5 = new Uint8Array(arraySource.length);
    let arr6 = new Uint8Array(arraySource.length);
    arr6.set(arraySource);
    let arr7 = Uint8Array.from(arraySource.$_iterator());
    let arr8 = Uint8Array.from<number>(arraySource, (v: number, k: number) => v);
    for (let i = 0; i < arr4.length; i++) {
        arr4[i] = arraySource[i];
        arr5.fill(arraySource[i]);
        arktest.assertEQ(arr1[i], result3[i]);
        arktest.assertEQ(arr2[i], result3[i]);
        arktest.assertEQ(arr3[i], result3[i]);
        arktest.assertEQ(arr4[i], result3[i]);
        arktest.assertEQ(arr5[i], result3[i]);
        arktest.assertEQ(arr6[i], result3[i]);
        arktest.assertEQ(arr7[i], result3[i]);
        arktest.assertEQ(arr8[i], result3[i]);
    }
}

function testUint16Array(): void {
    let arr1 = new Uint16Array(fixedArraySource);
    let arr2 = new Uint16Array(arraySource);
    let arr3 = new Uint16Array(arrayLikeSource);
    let arr4 = new Uint16Array(arraySource.length);
    let arr5 = new Uint16Array(arraySource.length);
    let arr6 = new Uint16Array(arraySource.length);
    arr6.set(arraySource);
    let arr7 = Uint16Array.from(arraySource.$_iterator());
    let arr8 = Uint16Array.from<number>(arraySource, (v: number, k: number) => v);
    for (let i = 0; i < arr4.length; i++) {
        arr4[i] = arraySource[i];
        arr5.fill(arraySource[i]);
        arktest.assertEQ(arr1[i], result4[i]);
        arktest.assertEQ(arr2[i], result4[i]);
        arktest.assertEQ(arr3[i], result4[i]);
        arktest.assertEQ(arr4[i], result4[i]);
        arktest.assertEQ(arr5[i], result4[i]);
        arktest.assertEQ(arr6[i], result4[i]);
        arktest.assertEQ(arr7[i], result4[i]);
        arktest.assertEQ(arr8[i], result4[i]);
    }
}

function testUint32Array(): void {
    let arr1 = new Uint32Array(fixedArraySource);
    let arr2 = new Uint32Array(arraySource);
    let arr3 = new Uint32Array(arrayLikeSource);
    let arr4 = new Uint32Array(arraySource.length);
    let arr5 = new Uint32Array(arraySource.length);
    let arr6 = new Uint32Array(arraySource.length);
    arr6.set(arraySource);
    let arr7 = Uint32Array.from(arraySource.$_iterator());
    let arr8 = Uint32Array.from<number>(arraySource, (v: number, k: number) => v);
    for (let i = 0; i < arr4.length; i++) {
        arr4[i] = arraySource[i];
        arr5.fill(arraySource[i]);
        arktest.assertEQ(arr1[i], result5[i]);
        arktest.assertEQ(arr2[i], result5[i]);
        arktest.assertEQ(arr3[i], result5[i]);
        arktest.assertEQ(arr4[i], result5[i]);
        arktest.assertEQ(arr5[i], result5[i]);
        arktest.assertEQ(arr6[i], result5[i]);
        arktest.assertEQ(arr7[i], result5[i]);
        arktest.assertEQ(arr8[i], result5[i]);
    }
}

function main(): int {
    const suite = new arktest.ArkTestsuite("TypedArray and TypedUArray number2int");
    suite.addTest('Int8Array test', testInt8Array);
    suite.addTest('Int16Array test', testInt16Array);
    suite.addTest('Int32Array test', testInt32Array);
    suite.addTest('Uint8Array test', testUint8Array);
    suite.addTest('Uint16Array test', testUint16Array);
    suite.addTest('Uint32Array test', testUint32Array);
    return suite.run();
}
