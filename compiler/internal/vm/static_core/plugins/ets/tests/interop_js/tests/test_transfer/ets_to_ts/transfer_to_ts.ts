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

export function testDynamicArrayBuffer(arrayBuffer: ArrayBuffer): void {
    ASSERT_TRUE(arrayBuffer instanceof ArrayBuffer);
    ASSERT_TRUE(arrayBuffer.byteLength === 8);
    let uint32View = new Uint32Array(arrayBuffer);
    ASSERT_TRUE(uint32View[0] === 0xbabe);
    ASSERT_TRUE(uint32View[1] === 0xcafe);

    uint32View[0] = 0xbeef;
    uint32View[1] = 0xf00d;
    ASSERT_TRUE(uint32View[0] === 0xbeef);
    ASSERT_TRUE(uint32View[1] === 0xf00d);

    ASSERT_TRUE(uint32View[2] === undefined);
}

export function testDynamicInt8Array(typedArray: Int8Array): void {
    ASSERT_TRUE(typedArray instanceof Int8Array);
    ASSERT_TRUE(typedArray.length === 2);
    ASSERT_TRUE(typedArray[0] === 1);
    ASSERT_TRUE(typedArray[1] === 2);

    typedArray[0] = 3;
    typedArray[1] = 4;
    ASSERT_TRUE(typedArray[0] === 3);
    ASSERT_TRUE(typedArray[1] === 4);

    ASSERT_TRUE(typedArray[2] === undefined);
}

export function testDynamicUint8Array(typedArray: Uint8Array): void {
    ASSERT_TRUE(typedArray instanceof Uint8Array);
    ASSERT_TRUE(typedArray.length === 2);
    ASSERT_TRUE(typedArray[0] === 1);
    ASSERT_TRUE(typedArray[1] === 2);

    typedArray[0] = 3;
    typedArray[1] = 4;
    ASSERT_TRUE(typedArray[0] === 3);
    ASSERT_TRUE(typedArray[1] === 4);

    ASSERT_TRUE(typedArray[2] === undefined);
}

export function testDynamicUint8ClampedArray(typedArray: Uint8ClampedArray): void {
    ASSERT_TRUE(typedArray instanceof Uint8ClampedArray);
    ASSERT_TRUE(typedArray.length === 2);
    ASSERT_TRUE(typedArray[0] === 1);
    ASSERT_TRUE(typedArray[1] === 2);

    typedArray[0] = 3;
    typedArray[1] = 4;
    ASSERT_TRUE(typedArray[0] === 3);
    ASSERT_TRUE(typedArray[1] === 4);

    ASSERT_TRUE(typedArray[2] === undefined);
}

export function testDynamicInt16Array(typedArray: Int16Array): void {
    ASSERT_TRUE(typedArray instanceof Int16Array);
    ASSERT_TRUE(typedArray.length === 2);
    ASSERT_TRUE(typedArray[0] === 1);
    ASSERT_TRUE(typedArray[1] === 2);

    typedArray[0] = 3;
    typedArray[1] = 4;
    ASSERT_TRUE(typedArray[0] === 3);
    ASSERT_TRUE(typedArray[1] === 4);
    ASSERT_TRUE(typedArray[2] === undefined);
}

export function testDynamicUint16Array(typedArray: Uint16Array): void {
    ASSERT_TRUE(typedArray instanceof Uint16Array);
    ASSERT_TRUE(typedArray.length === 2);
    ASSERT_TRUE(typedArray[0] === 1);
    ASSERT_TRUE(typedArray[1] === 2);

    typedArray[0] = 3;
    typedArray[1] = 4;
    ASSERT_TRUE(typedArray[0] === 3);
    ASSERT_TRUE(typedArray[1] === 4);

    ASSERT_TRUE(typedArray[2] === undefined);
}

export function testDynamicInt32Array(typedArray: Int32Array): void {
    ASSERT_TRUE(typedArray instanceof Int32Array);
    ASSERT_TRUE(typedArray.length === 2);
    ASSERT_TRUE(typedArray[0] === 0xbabe);
    ASSERT_TRUE(typedArray[1] === 0xcafe);

    typedArray[0] = 0xbeef;
    typedArray[1] = 0xf00d;
    ASSERT_TRUE(typedArray[0] === 0xbeef);
    ASSERT_TRUE(typedArray[1] === 0xf00d);

    ASSERT_TRUE(typedArray[2] === undefined);
}

export function testDynamicUint32Array(typedArray: Uint32Array): void {
    ASSERT_TRUE(typedArray instanceof Uint32Array);
    ASSERT_TRUE(typedArray.length === 2);
    ASSERT_TRUE(typedArray[0] === 0xbabe);
    ASSERT_TRUE(typedArray[1] === 0xcafe);

    typedArray[0] = 0xbeef;
    typedArray[1] = 0xf00d;
    ASSERT_TRUE(typedArray[0] === 0xbeef);
    ASSERT_TRUE(typedArray[1] === 0xf00d);

    ASSERT_TRUE(typedArray[2] === undefined);
}

export function testDynamicFloat32Array(typedArray: Float32Array): void {
    ASSERT_TRUE(typedArray instanceof Float32Array);
    ASSERT_TRUE(typedArray.length === 2);
    ASSERT_TRUE(typedArray[0] === 1.0);
    ASSERT_TRUE(typedArray[1] === 2.0);

    typedArray[0] = 3.0;
    typedArray[1] = 4.0;
    ASSERT_TRUE(typedArray[0] === 3.0);
    ASSERT_TRUE(typedArray[1] === 4.0);

    ASSERT_TRUE(typedArray[2] === undefined);
}

export function testDynamicFloat64Array(typedArray: Float64Array): void {
    ASSERT_TRUE(typedArray instanceof Float64Array);
    ASSERT_TRUE(typedArray.length === 2);
    ASSERT_TRUE(typedArray[0] === 1.0);
    ASSERT_TRUE(typedArray[1] === 2.0);

    typedArray[0] = 3.0;
    typedArray[1] = 4.0;
    ASSERT_TRUE(typedArray[0] === 3.0);
    ASSERT_TRUE(typedArray[1] === 4.0);

    ASSERT_TRUE(typedArray[2] === undefined);
}

export function testDynamicBigInt64Array(typedArray: BigInt64Array): void {
    ASSERT_TRUE(typedArray instanceof BigInt64Array);
    ASSERT_TRUE(typedArray.length === 2);
    ASSERT_TRUE(typedArray[0] === 1n);
    ASSERT_TRUE(typedArray[1] === 2n);

    typedArray[0] = 3n;
    typedArray[1] = 4n;
    ASSERT_TRUE(typedArray[0] === 3n);
    ASSERT_TRUE(typedArray[1] === 4n);

    ASSERT_TRUE(typedArray[2] === undefined);
}

export function testDynamicBigUint64Array(typedArray: BigUint64Array): void {
    ASSERT_TRUE(typedArray instanceof BigUint64Array);
    ASSERT_TRUE(typedArray.length === 2);
    ASSERT_TRUE(typedArray[0] === 1n);
    ASSERT_TRUE(typedArray[1] === 2n);

    typedArray[0] = 3n;
    typedArray[1] = 4n;
    ASSERT_TRUE(typedArray[0] === 3n);
    ASSERT_TRUE(typedArray[1] === 4n);

    ASSERT_TRUE(typedArray[2] === undefined);
}

export function testDynamicDataView(dataView: DataView): void {
    ASSERT_TRUE(dataView instanceof DataView);
    ASSERT_TRUE(dataView.byteLength === 8);

    ASSERT_TRUE(dataView.getUint32(0, true) === 0xbabe);
    ASSERT_TRUE(dataView.getUint32(4, true) === 0xcafe);

    dataView.setUint32(0, 0xbeef, true);
    dataView.setUint32(4, 0xf00d, true);

    ASSERT_TRUE(dataView.getUint32(0, true) === 0xbeef);
    ASSERT_TRUE(dataView.getUint32(4, true) === 0xf00d);

    try {
        dataView.getUint32(8, true);
        ASSERT_TRUE(false);
    } catch (e) {
        ASSERT_TRUE(e.message === 'getIndex +elementSize > viewSize');
    }
}

export function testDirectTransfer(param: Object): boolean {
    return true;
}