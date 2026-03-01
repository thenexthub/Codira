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

function testTransferArrayBufferDynamicToStatic(dynamicArrayBuffer: Any): void {
    let staticArrayBuffer: ArrayBuffer =
        InteropTransferHelper.transferArrayBufferToStatic(dynamicArrayBuffer) as ArrayBuffer;
    arktest.assertTrue(staticArrayBuffer instanceof ArrayBuffer);
    arktest.assertTrue(staticArrayBuffer.byteLength === 8);

    let uint32View = new Uint32Array(staticArrayBuffer);
    arktest.assertTrue(uint32View[0] === 0x55aa);
    arktest.assertTrue(uint32View[1] === 0x7c00);

    uint32View[0] = 0xcafe;
    uint32View[1] = 0xbabe;

    arktest.assertTrue(uint32View[0] === 0xcafe);
    arktest.assertTrue(uint32View[1] === 0xbabe);

    arktest.expectThrow(() => { uint32View[2]; }, (e) => e instanceof RangeError);
}

function testTransferInt8ArrayDynamicToStatic(dynamicTypedArray: Any): void {
    let staticTypedArray: Int8Array =
        InteropTransferHelper.transferInt8ArrayToStatic(dynamicTypedArray) as Int8Array;
    arktest.assertTrue(staticTypedArray instanceof Int8Array);
    arktest.assertTrue(staticTypedArray.length === 2);

    arktest.assertTrue(staticTypedArray[0] === 1);
    arktest.assertTrue(staticTypedArray[1] === 2);

    staticTypedArray[0] = 3;
    staticTypedArray[1] = 4;

    arktest.assertTrue(staticTypedArray[0] === 3);
    arktest.assertTrue(staticTypedArray[1] === 4);

    arktest.expectThrow(() => { staticTypedArray[2]; }, (e) => e instanceof RangeError);
}

function testTransferUint8ArrayDynamicToStatic(dynamicTypedArray: Any): void {
    let staticTypedArray: Uint8Array =
        InteropTransferHelper.transferUint8ArrayToStatic(dynamicTypedArray) as Uint8Array;
    arktest.assertTrue(staticTypedArray instanceof Uint8Array);
    arktest.assertTrue(staticTypedArray.length === 2);

    arktest.assertTrue(staticTypedArray[0] === 1);
    arktest.assertTrue(staticTypedArray[1] === 2);

    staticTypedArray[0] = 3;
    staticTypedArray[1] = 4;

    arktest.assertTrue(staticTypedArray[0] === 3);
    arktest.assertTrue(staticTypedArray[1] === 4);

    arktest.expectThrow(() => { staticTypedArray[2]; }, (e) => e instanceof RangeError);
}

function testTransferUint8ClampedArrayDynamicToStatic(dynamicTypedArray: Any): void {
    let staticTypedArray: Uint8ClampedArray =
        InteropTransferHelper.transferUint8ClampedArrayToStatic(dynamicTypedArray) as Uint8ClampedArray;
    arktest.assertTrue(staticTypedArray instanceof Uint8ClampedArray);
    arktest.assertTrue(staticTypedArray.length === 2);

    arktest.assertTrue(staticTypedArray[0] === 1);
    arktest.assertTrue(staticTypedArray[1] === 2);

    staticTypedArray[0] = 3;
    staticTypedArray[1] = 4;

    arktest.assertTrue(staticTypedArray[0] === 3);
    arktest.assertTrue(staticTypedArray[1] === 4);

    arktest.expectThrow(() => { staticTypedArray[2]; }, (e) => e instanceof RangeError);
}

function testTransferInt16ArrayDynamicToStatic(dynamicTypedArray: Any): void {
    let staticTypedArray: Int16Array =
        InteropTransferHelper.transferInt16ArrayToStatic(dynamicTypedArray) as Int16Array;
    arktest.assertTrue(staticTypedArray instanceof Int16Array);
    arktest.assertTrue(staticTypedArray.length === 2);

    arktest.assertTrue(staticTypedArray[0] === 1);
    arktest.assertTrue(staticTypedArray[1] === 2);

    staticTypedArray[0] = 3;
    staticTypedArray[1] = 4;

    arktest.assertTrue(staticTypedArray[0] === 3);
    arktest.assertTrue(staticTypedArray[1] === 4);

    arktest.expectThrow(() => { staticTypedArray[2]; }, (e) => e instanceof RangeError);
}

function testTransferUint16ArrayDynamicToStatic(dynamicTypedArray: Any): void {
    let staticTypedArray: Uint16Array =
        InteropTransferHelper.transferUint16ArrayToStatic(dynamicTypedArray) as Uint16Array;
    arktest.assertTrue(staticTypedArray instanceof Uint16Array);

    arktest.assertTrue(staticTypedArray[0] === 1);
    arktest.assertTrue(staticTypedArray[1] === 2);

    staticTypedArray[0] = 3;
    staticTypedArray[1] = 4;

    arktest.assertTrue(staticTypedArray[0] === 3);
    arktest.assertTrue(staticTypedArray[1] === 4);

    arktest.expectThrow(() => { staticTypedArray[2]; }, (e) => e instanceof RangeError);
}

function testTransferInt32ArrayDynamicToStatic(dynamicTypedArray: Any): void {
    let staticTypedArray: Int32Array =
        InteropTransferHelper.transferInt32ArrayToStatic(dynamicTypedArray) as Int32Array;
    arktest.assertTrue(staticTypedArray instanceof Int32Array);

    arktest.assertTrue(staticTypedArray[0] === 1);
    arktest.assertTrue(staticTypedArray[1] === 2);

    staticTypedArray[0] = 3;
    staticTypedArray[1] = 4;

    arktest.assertTrue(staticTypedArray[0] === 3);
    arktest.assertTrue(staticTypedArray[1] === 4);

    arktest.expectThrow(() => { staticTypedArray[2]; }, (e) => e instanceof RangeError);
}

function testTransferUint32ArrayDynamicToStatic(dynamicTypedArray: Any): void {
    let staticTypedArray: Uint32Array =
        InteropTransferHelper.transferUint32ArrayToStatic(dynamicTypedArray) as Uint32Array;
    arktest.assertTrue(staticTypedArray instanceof Uint32Array)
    arktest.assertTrue(staticTypedArray.length === 2);

    arktest.assertTrue(staticTypedArray[0] === 0x55aa);
    arktest.assertTrue(staticTypedArray[1] === 0x7c00);

    staticTypedArray[0] = 0xcafe;
    staticTypedArray[1] = 0xbabe;

    arktest.assertTrue(staticTypedArray[0] === 0xcafe);
    arktest.assertTrue(staticTypedArray[1] === 0xbabe);

    arktest.expectThrow(() => { staticTypedArray[2]; }, (e) => e instanceof RangeError);
}

function testTransferFloat32ArrayDynamicToStatic(dynamicTypedArray: Any): void {
    let staticTypedArray: Float32Array =
        InteropTransferHelper.transferFloat32ArrayToStatic(dynamicTypedArray) as Float32Array;
    arktest.assertTrue(staticTypedArray instanceof Float32Array);
    arktest.assertTrue(staticTypedArray.length === 2);

    arktest.assertTrue(staticTypedArray[0] === 1.0);
    arktest.assertTrue(staticTypedArray[1] === 2.0);

    staticTypedArray[0] = 3.0;
    staticTypedArray[1] = 4.0;

    arktest.assertTrue(staticTypedArray[0] === 3.0);
    arktest.assertTrue(staticTypedArray[1] === 4.0);

    arktest.expectThrow(() => { staticTypedArray[2]; }, (e) => e instanceof RangeError);
}

function testTransferFloat64ArrayDynamicToStatic(dynamicTypedArray: Any): void {
    let staticTypedArray: Float64Array =
        InteropTransferHelper.transferFloat64ArrayToStatic(dynamicTypedArray) as Float64Array;
    arktest.assertTrue(staticTypedArray instanceof Float64Array);
    arktest.assertTrue(staticTypedArray.length === 2);

    arktest.assertTrue(staticTypedArray[0] === 1.0);
    arktest.assertTrue(staticTypedArray[1] === 2.0);

    staticTypedArray[0] = 3.0;
    staticTypedArray[1] = 4.0;

    arktest.assertTrue(staticTypedArray[0] === 3.0);
    arktest.assertTrue(staticTypedArray[1] === 4.0);

    arktest.expectThrow(() => { staticTypedArray[2]; }, (e) => e instanceof RangeError);
}

function testTransferBigInt64ArrayDynamicToStatic(dynamicTypedArray: Any): void {
    let staticTypedArray: BigInt64Array =
        InteropTransferHelper.transferBigInt64ArrayToStatic(dynamicTypedArray) as BigInt64Array;
    arktest.assertTrue(staticTypedArray instanceof BigInt64Array);
    arktest.assertTrue(staticTypedArray.length === 2);

    arktest.assertTrue(staticTypedArray[0] === 1n);
    arktest.assertTrue(staticTypedArray[1] === 2n);

    staticTypedArray[0] = 3n;
    staticTypedArray[1] = 4n;

    arktest.assertTrue(staticTypedArray[0] === 3n);
    arktest.assertTrue(staticTypedArray[1] === 4n);

    arktest.expectThrow(() => { staticTypedArray[2]; }, (e) => e instanceof RangeError);
}

function testTransferBigUint64ArrayDynamicToStatic(dynamicTypedArray: Any): void {
    let staticTypedArray: BigUint64Array =
        InteropTransferHelper.transferBigUint64ArrayToStatic(dynamicTypedArray) as BigUint64Array;
    arktest.assertTrue(staticTypedArray instanceof BigUint64Array);

    arktest.assertTrue(staticTypedArray[0] === 1n);
    arktest.assertTrue(staticTypedArray[1] === 2n);

    staticTypedArray[0] = 3n;
    staticTypedArray[1] = 4n;

    arktest.assertTrue(staticTypedArray[0] === 3n);
    arktest.assertTrue(staticTypedArray[1] === 4n);

    arktest.expectThrow(() => { staticTypedArray[2]; }, (e) => e instanceof RangeError);
}

function testTransferDataViewyDynamicToStatic(dynamicDateView: Any): void {
    let staticDateView: DataView =
        InteropTransferHelper.transferDataViewToStatic(dynamicDateView) as DataView;
    arktest.assertTrue(staticDateView instanceof DataView);
    arktest.assertTrue(staticDateView.byteLength === 8);

    arktest.assertTrue(staticDateView.getUint32(0, true) === 0x55aa);
    arktest.assertTrue(staticDateView.getUint32(4, true) === 0x7c00);

    staticDateView.setUint32(0, 0xcafe, true);
    staticDateView.setUint32(4, 0xbabe, true);

    arktest.assertTrue(staticDateView.getUint32(0, true) === 0xcafe);
    arktest.assertTrue(staticDateView.getUint32(4, true) === 0xbabe);

    arktest.expectThrow(() => { staticDateView.getUint32(8, true); }, (e) => e instanceof RangeError);
}

function testDirectTransferArrayBuffer(param: ArrayBuffer): boolean {
    return true;
}

function testDirectTransferTypedArray(param: Uint8Array): boolean {
    return true;
}

function testDirectTransferDataView(param: DataView): boolean {
    return true;
}