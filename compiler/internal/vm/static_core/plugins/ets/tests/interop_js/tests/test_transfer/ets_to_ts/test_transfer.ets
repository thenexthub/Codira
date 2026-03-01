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

let module = ESValue.load('transfer_to_ts');

let testDynamicArrayBuffer = module.getProperty('testDynamicArrayBuffer');
let testDynamicInt8Array = module.getProperty('testDynamicInt8Array');
let testDynamicUint8Array = module.getProperty('testDynamicUint8Array');
let testDynamicUint8ClampedArray = module.getProperty('testDynamicUint8ClampedArray');
let testDynamicInt16Array = module.getProperty('testDynamicInt16Array');
let testDynamicUint16Array = module.getProperty('testDynamicUint16Array');
let testDynamicInt32Array = module.getProperty('testDynamicInt32Array');
let testDynamicUint32Array = module.getProperty('testDynamicUint32Array');
let testDynamicFloat32Array = module.getProperty('testDynamicFloat32Array');
let testDynamicFloat64Array = module.getProperty('testDynamicFloat64Array');
let testDynamicBigInt64Array = module.getProperty('testDynamicBigInt64Array');
let testDynamicBigUint64Array = module.getProperty('testDynamicBigUint64Array');
let testDynamicDataView = module.getProperty('testDynamicDataView');
let testDirectTransfer = module.getProperty('testDirectTransfer');

function testTransferArrayBufferStaticToDynamic(): void {
    let staticArrayBuffer = new ArrayBuffer(8);
    let uint32View = new Uint32Array(staticArrayBuffer);
    uint32View[0] = 0xbabe;
    uint32View[1] = 0xcafe;

    let dynamicArrayBuffer = InteropTransferHelper.transferArrayBufferToDynamic(staticArrayBuffer);
    testDynamicArrayBuffer.invoke(ESValue.wrap(dynamicArrayBuffer));
}

function testTransferInt8ArrayStaticToDynamic(): void {
    let staticTypedArray = new Int8Array(2);
    staticTypedArray[0] = 1;
    staticTypedArray[1] = 2;

    let dynamicTypedArray = InteropTransferHelper.transferInt8ArrayToDynamic(staticTypedArray);
    testDynamicInt8Array.invoke(ESValue.wrap(dynamicTypedArray));
}

function testTransferUint8ArrayStaticToDynamic(): void {
    let staticTypedArray = new Uint8Array(2);
    staticTypedArray[0] = 1;
    staticTypedArray[1] = 2;

    let dynamicTypedArray = InteropTransferHelper.transferUint8ArrayToDynamic(staticTypedArray);
    testDynamicUint8Array.invoke(ESValue.wrap(dynamicTypedArray));
}

function testTransferUint8ClampedArrayStaticToDynamic(): void {
    let staticTypedArray = new Uint8ClampedArray(2);
    staticTypedArray[0] = 1;
    staticTypedArray[1] = 2;

    let dynamicTypedArray = InteropTransferHelper.transferUint8ClampedArrayToDynamic(staticTypedArray);
    testDynamicUint8ClampedArray.invoke(ESValue.wrap(dynamicTypedArray));
}

function testTransferInt16ArrayStaticToDynamic(): void {
    let staticTypedArray = new Int16Array(2);
    staticTypedArray[0] = 1;
    staticTypedArray[1] = 2;

    let dynamicTypedArray = InteropTransferHelper.transferInt16ArrayToDynamic(staticTypedArray);
    testDynamicInt16Array.invoke(ESValue.wrap(dynamicTypedArray));
}

function testTransferUint16ArrayStaticToDynamic(): void {
    let staticTypedArray = new Uint16Array(2);
    staticTypedArray[0] = 1;
    staticTypedArray[1] = 2;

    let dynamicTypedArray = InteropTransferHelper.transferUint16ArrayToDynamic(staticTypedArray);
    testDynamicUint16Array.invoke(ESValue.wrap(dynamicTypedArray));
}

function testTransferInt32ArrayStaticToDynamic(): void {
    let staticTypedArray = new Int32Array(2);
    staticTypedArray[0] = 0xbabe;
    staticTypedArray[1] = 0xcafe;

    let dynamicTypedArray = InteropTransferHelper.transferInt32ArrayToDynamic(staticTypedArray);
    testDynamicInt32Array.invoke(ESValue.wrap(dynamicTypedArray));
}

function testTransferUint32ArrayStaticToDynamic(): void {
    let staticTypedArray = new Uint32Array(2);
    staticTypedArray[0] = 0xbabe;
    staticTypedArray[1] = 0xcafe;

    let dynamicTypedArray = InteropTransferHelper.transferUint32ArrayToDynamic(staticTypedArray);
    testDynamicUint32Array.invoke(ESValue.wrap(dynamicTypedArray));
}

function testTransferFloat32ArrayStaticToDynamic(): void {
    let staticTypedArray = new Float32Array(2);
    staticTypedArray[0] = 1.0;
    staticTypedArray[1] = 2.0;

    let dynamicTypedArray = InteropTransferHelper.transferFloat32ArrayToDynamic(staticTypedArray);
    testDynamicFloat32Array.invoke(ESValue.wrap(dynamicTypedArray));
}

function testTransferFloat64ArrayStaticToDynamic(): void {
    let staticTypedArray = new Float64Array(2);
    staticTypedArray[0] = 1.0;
    staticTypedArray[1] = 2.0;

    let dynamicTypedArray = InteropTransferHelper.transferFloat64ArrayToDynamic(staticTypedArray);
    testDynamicFloat64Array.invoke(ESValue.wrap(dynamicTypedArray));
}

function testTransferDataViewStaticToDynamic(): void {
    let staticDynamicDataView = new DataView(new ArrayBuffer(8));
    staticDynamicDataView.setUint32(0, 0xbabe, true);
    staticDynamicDataView.setUint32(4, 0xcafe, true);

    let dynamicDataView = InteropTransferHelper.transferDataViewToDynamic(staticDynamicDataView);
    testDynamicDataView.invoke(ESValue.wrap(dynamicDataView));
}

function testTransferBigInt64ArrayStaticToDynamic(): void {
    let staticTypedArray = new BigInt64Array(2);
    staticTypedArray[0] = 1n;
    staticTypedArray[1] = 2n;

    let dynamicTypedArray = InteropTransferHelper.transferBigInt64ArrayToDynamic(staticTypedArray);
    testDynamicBigInt64Array.invoke(ESValue.wrap(dynamicTypedArray));
}

function testTransferBigUint64ArrayStaticToDynamic(): void {
    let staticTypedArray = new BigUint64Array(2);
    staticTypedArray[0] = 1n;
    staticTypedArray[1] = 2n;

    let dynamicTypedArray = InteropTransferHelper.transferBigUint64ArrayToDynamic(staticTypedArray);
    testDynamicBigUint64Array.invoke(ESValue.wrap(dynamicTypedArray));
}

const errStrPre = 'Error: Seamless conversion for class ';
const errStrPost = ' is not supported';

function testNoSeamlessForClass(func: ESValue, arg: Object, className: String): void {
    let res = false;
    try {
        func.invoke(ESValue.wrap(arg));
    } catch (err) {
        res = err.toString() === errStrPre + className + errStrPost;
    }
    arktest.assertTrue(res);
}

function testDirectTransferStaticToDynamic(): void {
    let buffer = new ArrayBuffer(100);
    testNoSeamlessForClass(testDirectTransfer, buffer, 'Lstd/core/ArrayBuffer;');
    testNoSeamlessForClass(testDirectTransfer, new Uint8Array(buffer), 'Lescompat/Uint8Array;');
    testNoSeamlessForClass(testDirectTransfer, new DataView(buffer), 'Lstd/core/DataView;');
}

function testTransferStaticToDynamic(): boolean {
    testTransferArrayBufferStaticToDynamic();
    testTransferInt8ArrayStaticToDynamic();
    testTransferUint8ArrayStaticToDynamic();
    testTransferUint8ClampedArrayStaticToDynamic();
    testTransferInt16ArrayStaticToDynamic();
    testTransferUint16ArrayStaticToDynamic();
    testTransferInt32ArrayStaticToDynamic();
    testTransferUint32ArrayStaticToDynamic();
    testTransferDataViewStaticToDynamic();
    testTransferFloat32ArrayStaticToDynamic();
    testTransferFloat64ArrayStaticToDynamic();
    testTransferBigInt64ArrayStaticToDynamic();
    testTransferBigUint64ArrayStaticToDynamic();
    testDirectTransferStaticToDynamic();

    return true;
}
