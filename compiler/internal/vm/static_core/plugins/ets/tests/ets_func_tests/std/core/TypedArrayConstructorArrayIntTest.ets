/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

function assertArrayEquals(result: ArrayLike<Number>, expected: ArrayLike<Number>): void {
    arktest.assertEQ(result.length, expected.length);
    for (let i = 0; i < result.length; i++) {
        arktest.assertEQ(result[i], expected[i]);
    }
}

function assertBigIntArrayEquals(result: ArrayLike<BigInt>, expected: ArrayLike<BigInt>): void {
    arktest.assertEQ(result.length, expected.length);
    for (let i = 0; i < result.length; i++) {
        arktest.assertEQ(result[i], expected[i]);
    }
}

function testInt8ArrayFromLiteral() {
    const arrayIntsLiteral = [1, 2, 3, 4, 5, -1, -65536, -128, -129, 127, 128, -32768, 32767];
    const result = new Int8Array(arrayIntsLiteral);
    const expected = Int8Array.from([1, 2, 3, 4, 5, -1, 0, -128, 127, 127, -128, 0, -1]);
    
    assertArrayEquals(result, expected);
}

function testInt16ArrayFromLiteral() {
    const arrayIntsLiteral = [1, 2, 3, 4, 5, -1, -65536, -128, -129, 127, 128, -32768, 32767, -32769, 32768];
    const result = new Int16Array(arrayIntsLiteral);
    const expected = Int16Array.from([1, 2, 3, 4, 5, -1, 0, -128, -129, 127, 128, -32768, 32767, 32767, -32768]);
    
    assertArrayEquals(result, expected);
}

function testInt32ArrayFromLiteral() {
    const arrayIntsLiteral = [1, 2, 3, 4, 5, -1, -65536, -128, -129, 127, 128, -32768, 32767, -2147483648, 2147483647];
    const result = new Int32Array(arrayIntsLiteral);
    const expected = Int32Array.from([1, 2, 3, 4, 5, -1, -65536, -128, -129, 127, 128, -32768, 32767, -2147483648, 2147483647]);
    
    assertArrayEquals(result, expected);
}

function testUint16ArrayFromLiteral() {
    const arrayIntsLiteral = [1, 2, 3, 4, 5, -1, -65536, -128, -129, 127, 128, -32768, 32767, 65535, 65536];
    const result = new Uint16Array(arrayIntsLiteral);
    const expected = Uint16Array.from([1, 2, 3, 4, 5, 65535, 0, 65408, 65407, 127, 128, 32768, 32767, 65535, 0]);
    
    assertArrayEquals(result, expected);
}

function testUint32ArrayFromLiteral() {
    const arrayIntsLiteral = [1, 2, 3, 4, 5, -1, -65536, -128, -129, 127, 128, -32768, 32767, -2147483648, 2147483647];
    const result = new Uint32Array(arrayIntsLiteral);
    const expected = new Uint32Array([1, 2, 3, 4, 5, 4294967295, 4294901760, 4294967168, 4294967167, 127, 128, 4294934528, 32767, 2147483648, 2147483647]);
    
    assertArrayEquals(result, expected);
}

function testUint8ArrayFromLiteral() {
    const arrayIntsLiteral = [1, 2, 3, 4, 5, -1, -65536, -128, -129, 127, 128, 255, 256, -32768];
    const result = new Uint8Array(arrayIntsLiteral);
    const expected = Uint8Array.from([1, 2, 3, 4, 5, 255, 0, 128, 127, 127, 128, 255, 0, 0]);
    
    assertArrayEquals(result, expected);
}

function testUint8ClampedArrayFromLiteral() {
    const arrayIntsLiteral = [1, 2, 3, 4, 5, -1, -65536, -128, -129, 127, 128, 255, 256, 300, -32768];
    const result = new Uint8ClampedArray(arrayIntsLiteral);
    // The constructor applies .toByte() first (which wraps the value to fit in a byte range: -128 to 127), and THEN applies clamping (which converts negative values to 0 and keeps positive values as-is up to 255).
    const expected = Uint8ClampedArray.from([1, 2, 3, 4, 5, 0, 0, 0, 127, 127, 0, 0, 0, 44, 0]);
    
    assertArrayEquals(result, expected);
}

function main() {
    let myTestsuite = new arktest.ArkTestsuite("TypedArray Constructor from Array<int> Suite");
    myTestsuite.addTest("testInt8ArrayFromLiteral", testInt8ArrayFromLiteral);
    myTestsuite.addTest("testInt16ArrayFromLiteral", testInt16ArrayFromLiteral);
    myTestsuite.addTest("testInt32ArrayFromLiteral", testInt32ArrayFromLiteral);
    myTestsuite.addTest("testUint16ArrayFromLiteral", testUint16ArrayFromLiteral);
    myTestsuite.addTest("testUint32ArrayFromLiteral", testUint32ArrayFromLiteral);
    myTestsuite.addTest("testUint8ArrayFromLiteral", testUint8ArrayFromLiteral);
    myTestsuite.addTest("testUint8ClampedArrayFromLiteral", testUint8ClampedArrayFromLiteral);
    myTestsuite.run();
}
