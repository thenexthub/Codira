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

function testInt8ArraySet() {
    let sample = new Int8Array([1, 2, 3, 4])
    let src = new Int8Array(sample.buffer, 0, 2)
    sample.set(src, 1)
    arktest.assertEQ(sample.toString(), '1,1,2,4')
}

function testInt16ArraySet() {
    let sample = new Int16Array([1, 2, 3, 4])
    let src = new Int16Array(sample.buffer, 0, 2)
    sample.set(src, 1)
    arktest.assertEQ(sample.toString(), '1,1,2,4')
}

function testInt32ArraySet() {
    let sample = new Int32Array([1, 2, 3, 4])
    let src = new Int32Array(sample.buffer, 0, 2)
    sample.set(src, 1)
    arktest.assertEQ(sample.toString(), '1,1,2,4')
}

function testBigInt64ArraySet() {
    let sample = new BigInt64Array([1, 2, 3, 4])
    let src = new BigInt64Array(sample.buffer, 0, 2)
    sample.set(src, 1)
    arktest.assertEQ(sample.toString(), '1,1,2,4')
}

function testFloat32ArraySet() {
    let sample = new Float32Array([1, 2, 3, 4])
    let src = new Float32Array(sample.buffer, 0, 2)
    sample.set(src, 1)
    arktest.assertEQ(sample.toString(), '1,1,2,4')
}

function testFloat64ArraySet() {
    let sample = new Float64Array([1, 2, 3, 4])
    let src = new Float64Array(sample.buffer, 0, 2)
    sample.set(src, 1)
    arktest.assertEQ(sample.toString(), '1,1,2,4')
}

function testUint8ClampedArraySet() {
    let sample = new Uint8ClampedArray([1, 2, 3, 4])
    let src = new Uint8ClampedArray(sample.buffer, 0, 2)
    sample.set(src, 1)
    arktest.assertEQ(sample.toString(), '1,1,2,4')
}

function testUint8ArraySet() {
    let sample = new Uint8Array([1, 2, 3, 4])
    let src = new Uint8Array(sample.buffer, 0, 2)
    sample.set(src, 1)
    arktest.assertEQ(sample.toString(), '1,1,2,4')
}

function testUint16ArraySet() {
    let sample = new Uint16Array([1, 2, 3, 4])
    let src = new Uint16Array(sample.buffer, 0, 2)
    sample.set(src, 1)
    arktest.assertEQ(sample.toString(), '1,1,2,4')
}

function testUint32ArraySet() {
    let sample = new Uint32Array([1, 2, 3, 4])
    let src = new Uint32Array(sample.buffer, 0, 2)
    sample.set(src, 1)
    arktest.assertEQ(sample.toString(), '1,1,2,4')
}

function testBigUint64ArraySet() {
    let sample = new BigUint64Array([1, 2, 3, 4])
    let src = new BigUint64Array(sample.buffer, 0, 2)
    sample.set(src, 1)
    arktest.assertEQ(sample.toString(), '1,1,2,4')
}



function main() {
    testInt8ArraySet()
    testInt16ArraySet()
    testInt32ArraySet()
    testBigInt64ArraySet()
    testFloat32ArraySet()
    testFloat64ArraySet()
    testUint8ClampedArraySet()
    testUint8ArraySet()
    testUint16ArraySet()
    testUint32ArraySet()
    testBigUint64ArraySet()
}
