/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

function checkRangeError(e: Error): boolean {
    return e instanceof RangeError;
}

function int8ArrayLengthTest() {
    arktest.expectThrow(() => { new Int8Array(-1) }, checkRangeError);
    arktest.expectThrow(() => { new Int8Array(new ArrayBuffer(8), 0, -1) }, checkRangeError);
    arktest.expectThrow(() => { new Int8Array(new ArrayBuffer(8), 0, 9) }, checkRangeError);
}

function int16ArrayLengthTest() {
    arktest.expectThrow(() => { new Int16Array(-1) }, checkRangeError);
    arktest.expectThrow(() => { new Int16Array(Int.MAX_VALUE) }, checkRangeError);
    arktest.expectThrow(() => { new Int16Array(new ArrayBuffer(8), 0, -1) }, checkRangeError);
    arktest.expectThrow(() => { new Int16Array(new ArrayBuffer(8), 0, 5) }, checkRangeError);
}

function int32ArrayLengthTest() {
    arktest.expectThrow(() => { new Int32Array(-1) }, checkRangeError);
    arktest.expectThrow(() => { new Int32Array(Int.MAX_VALUE) }, checkRangeError);
    arktest.expectThrow(() => { new Int32Array(new ArrayBuffer(8), 0, -1) }, checkRangeError);
    arktest.expectThrow(() => { new Int32Array(new ArrayBuffer(8), 0, 3) }, checkRangeError);
}

function float32ArrayLengthTest() {
    arktest.expectThrow(() => { new Float32Array(-1) }, checkRangeError);
    arktest.expectThrow(() => { new Float32Array(Int.MAX_VALUE) }, checkRangeError);
    arktest.expectThrow(() => { new Float32Array(new ArrayBuffer(8), 0, -1) }, checkRangeError);
    arktest.expectThrow(() => { new Float32Array(new ArrayBuffer(8), 0, 3) }, checkRangeError);
}

function float64ArrayLengthTest() {
    arktest.expectThrow(() => { new Float64Array(-1) }, checkRangeError);
    arktest.expectThrow(() => { new Float64Array(Int.MAX_VALUE) }, checkRangeError);
    arktest.expectThrow(() => { new Float64Array(new ArrayBuffer(8), 0, -1) }, checkRangeError);
    arktest.expectThrow(() => { new Float64Array(new ArrayBuffer(8), 0, 2) }, checkRangeError);
}

function bigInt64ArrayLengthTest() {
    arktest.expectThrow(() => { new BigInt64Array(-1) }, checkRangeError);
    arktest.expectThrow(() => { new BigInt64Array(Int.MAX_VALUE) }, checkRangeError);
    arktest.expectThrow(() => { new BigInt64Array(new ArrayBuffer(8), 0, -1) }, checkRangeError);
    arktest.expectThrow(() => { new BigInt64Array(new ArrayBuffer(8), 0, 2) }, checkRangeError);
}

function uint8ClampedArrayLengthTest() {
    arktest.expectThrow(() => { new Uint8ClampedArray(-1) }, checkRangeError);
    arktest.expectThrow(() => { new Uint8ClampedArray(new ArrayBuffer(8), 0, -1) }, checkRangeError);
    arktest.expectThrow(() => { new Uint8ClampedArray(new ArrayBuffer(8), 0, 9) }, checkRangeError);
}

function uint8ArrayLengthTest() {
    arktest.expectThrow(() => { new Uint8Array(-1) }, checkRangeError);
    arktest.expectThrow(() => { new Uint8Array(new ArrayBuffer(8), 0, -1) }, checkRangeError);
    arktest.expectThrow(() => { new Uint8Array(new ArrayBuffer(8), 0, 9) }, checkRangeError);
}

function uint16ArrayLengthTest() {
    arktest.expectThrow(() => { new Uint16Array(-1) }, checkRangeError);
    arktest.expectThrow(() => { new Uint16Array(Int.MAX_VALUE) }, checkRangeError);
    arktest.expectThrow(() => { new Uint16Array(new ArrayBuffer(8), 0, -1) }, checkRangeError);
    arktest.expectThrow(() => { new Uint16Array(new ArrayBuffer(8), 0, 5) }, checkRangeError);
}

function uint32ArrayLengthTest() {
    arktest.expectThrow(() => { new Uint32Array(-1) }, checkRangeError);
    arktest.expectThrow(() => { new Uint32Array(Int.MAX_VALUE) }, checkRangeError);
    arktest.expectThrow(() => { new Uint32Array(new ArrayBuffer(8), 0, -1) }, checkRangeError);
    arktest.expectThrow(() => { new Uint32Array(new ArrayBuffer(8), 0, 3) }, checkRangeError);
}

function bigUint64ArrayLengthTest() {
    arktest.expectThrow(() => { new BigUint64Array(-1) }, checkRangeError);
    arktest.expectThrow(() => { new BigUint64Array(Int.MAX_VALUE) }, checkRangeError);
    arktest.expectThrow(() => { new BigUint64Array(new ArrayBuffer(8), 0, -1) }, checkRangeError);
    arktest.expectThrow(() => { new BigUint64Array(new ArrayBuffer(8), 0, 3) }, checkRangeError);
}

function main(): int {
    const suite = new arktest.ArkTestsuite('TypedArray and TypedUArray length tests');
    suite.addTest('for Int8Array', int8ArrayLengthTest);
    suite.addTest('for Int16Array', int16ArrayLengthTest);
    suite.addTest('for Int32Array', int32ArrayLengthTest);
    suite.addTest('for Float32Array', float32ArrayLengthTest);
    suite.addTest('for Float64Array', float64ArrayLengthTest);
    suite.addTest('for BigInt64Array', bigInt64ArrayLengthTest);
    suite.addTest('for Uint8ClampedArray', uint8ClampedArrayLengthTest);
    suite.addTest('for Uint8Array', uint8ArrayLengthTest);
    suite.addTest('for Uint16Array', uint16ArrayLengthTest);
    suite.addTest('for Uint32Array', uint32ArrayLengthTest);
    suite.addTest('for BigUint64Array', bigUint64ArrayLengthTest);
    return suite.run();
}
