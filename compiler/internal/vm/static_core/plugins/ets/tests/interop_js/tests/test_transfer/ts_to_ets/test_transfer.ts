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

let etsVm = globalThis.gtest.etsVm;

let testTransferArrayBufferDynamicToStatic = etsVm.getFunction(
    'Ltransfer_to_ets/ETSGLOBAL;',
    'testTransferArrayBufferDynamicToStatic'
);
let testTransferInt8ArrayDynamicToStatic = etsVm.getFunction(
    'Ltransfer_to_ets/ETSGLOBAL;',
    'testTransferInt8ArrayDynamicToStatic'
);
let testTransferUint8ArrayDynamicToStatic = etsVm.getFunction(
    'Ltransfer_to_ets/ETSGLOBAL;',
    'testTransferUint8ArrayDynamicToStatic'
);
let testTransferUint8ClampedArrayDynamicToStatic = etsVm.getFunction(
    'Ltransfer_to_ets/ETSGLOBAL;',
    'testTransferUint8ClampedArrayDynamicToStatic'
);
let testTransferInt16ArrayDynamicToStatic = etsVm.getFunction(
    'Ltransfer_to_ets/ETSGLOBAL;',
    'testTransferInt16ArrayDynamicToStatic'
);
let testTransferUint16ArrayDynamicToStatic = etsVm.getFunction(
    'Ltransfer_to_ets/ETSGLOBAL;',
    'testTransferUint16ArrayDynamicToStatic'
);
let testTransferInt32ArrayDynamicToStatic = etsVm.getFunction(
    'Ltransfer_to_ets/ETSGLOBAL;',
    'testTransferInt32ArrayDynamicToStatic'
);
let testTransferUint32ArrayDynamicToStatic = etsVm.getFunction(
    'Ltransfer_to_ets/ETSGLOBAL;',
    'testTransferUint32ArrayDynamicToStatic'
);
let testTransferDataViewyDynamicToStatic = etsVm.getFunction(
    'Ltransfer_to_ets/ETSGLOBAL;',
    'testTransferDataViewyDynamicToStatic'
);
let testTransferFloat32ArrayDynamicToStatic = etsVm.getFunction(
    'Ltransfer_to_ets/ETSGLOBAL;',
    'testTransferFloat32ArrayDynamicToStatic'
);
let testTransferFloat64ArrayDynamicToStatic = etsVm.getFunction(
    'Ltransfer_to_ets/ETSGLOBAL;',
    'testTransferFloat64ArrayDynamicToStatic'
);
let testTransferBigInt64ArrayDynamicToStatic = etsVm.getFunction(
    'Ltransfer_to_ets/ETSGLOBAL;',
    'testTransferBigInt64ArrayDynamicToStatic'
);
let testTransferBigUint64ArrayDynamicToStatic = etsVm.getFunction(
    'Ltransfer_to_ets/ETSGLOBAL;',
    'testTransferBigUint64ArrayDynamicToStatic'
);
let testDirectTransferArrayBuffer = etsVm.getFunction(
    'Ltransfer_to_ets/ETSGLOBAL;',
    'testDirectTransferArrayBuffer'
);
let testDirectTransferTypedArray = etsVm.getFunction(
    'Ltransfer_to_ets/ETSGLOBAL;',
    'testDirectTransferTypedArray'
);
let testDirectTransferDataView = etsVm.getFunction(
    'Ltransfer_to_ets/ETSGLOBAL;',
    'testDirectTransferDataView'
);

function testArrayBuffer(): void {
    let array_buffer = new ArrayBuffer(8);
    let uint32_view = new Uint32Array(array_buffer);
    uint32_view[0] = 0x55aa;
    uint32_view[1] = 0x7c00;

    testTransferArrayBufferDynamicToStatic(array_buffer);
}

function testInt8Array(): void {
    let int8Array = new Int8Array(2);
    int8Array[0] = 1;
    int8Array[1] = 2;

    testTransferInt8ArrayDynamicToStatic(int8Array);
}

function testUint8Array(): void {
    let uint8Array = new Uint8Array(2);
    uint8Array[0] = 1;
    uint8Array[1] = 2;

    testTransferUint8ArrayDynamicToStatic(uint8Array);
}


function testUint8ClampedArray(): void {
    let uint8ClampedArray = new Uint8ClampedArray(2);
    uint8ClampedArray[0] = 1;
    uint8ClampedArray[1] = 2;

    testTransferUint8ClampedArrayDynamicToStatic(uint8ClampedArray);
}


function testInt16Array(): void {
    let int16Array = new Int16Array(2);
    int16Array[0] = 1;
    int16Array[1] = 2;

    testTransferInt16ArrayDynamicToStatic(int16Array);
}

function testUint16Array(): void {
    let uint16Array = new Uint16Array(2);
    uint16Array[0] = 1;
    uint16Array[1] = 2;

    testTransferUint16ArrayDynamicToStatic(uint16Array);
}

function testInt32Array(): void {
    let int32Array = new Int32Array(2);
    int32Array[0] = 0x55aa;
    int32Array[1] = 0x7c00;

    testTransferUint32ArrayDynamicToStatic(int32Array);
}

function testUint32Array(): void {
    let uint32Array = new Uint32Array(2);
    uint32Array[0] = 0x55aa;
    uint32Array[1] = 0x7c00;

    testTransferUint32ArrayDynamicToStatic(uint32Array);
}

function testFloat32Array(): void {
    let float32Array = new Float32Array(2);
    float32Array[0] = 1.0;
    float32Array[1] = 2.0;

    testTransferFloat32ArrayDynamicToStatic(float32Array);
}

function testFloat64Array(): void {
    let float64Array = new Float64Array(2);
    float64Array[0] = 1.0;
    float64Array[1] = 2.0;

    testTransferFloat64ArrayDynamicToStatic(float64Array);
}

function testBigInt64Array(): void {
    let bigInt64Array = new BigInt64Array(2);
    bigInt64Array[0] = BigInt(1);
    bigInt64Array[1] = BigInt(2);

    testTransferBigInt64ArrayDynamicToStatic(bigInt64Array);
}

function testBigUint64Array(): void {
    let bigUint64Array = new BigUint64Array(2);
    bigUint64Array[0] = BigInt(1);
    bigUint64Array[1] = BigInt(2);

    testTransferBigUint64ArrayDynamicToStatic(bigUint64Array);
}

function testDataView(): void {
    let dynamicDataView = new DataView(new ArrayBuffer(8));
    dynamicDataView.setUint32(0, 0x55aa, true);
    dynamicDataView.setUint32(4, 0x7c00, true);

    testTransferDataViewyDynamicToStatic(dynamicDataView);
}

const errStrPre = 'Error: Seamless conversion for class ';
const errStrPost = ' is not supported';

function testNoSeamlessForClass(func, arg, className): void {
    let res = false;
    try {
        func(arg);
    } catch (err) {
        res = err.toString() === errStrPre + className + errStrPost;
    }
    ASSERT_TRUE(res);
}

function testDirectTransfer(): void {
    let buffer = new ArrayBuffer(100);
    testNoSeamlessForClass(testDirectTransferArrayBuffer, buffer, 'Lstd/core/ArrayBuffer;');
    testNoSeamlessForClass(testDirectTransferTypedArray, new Uint8Array(buffer), 'Lescompat/Uint8Array;');
    testNoSeamlessForClass(testDirectTransferDataView, new DataView(buffer), 'Lstd/core/DataView;');
}

function main(): void {
    testArrayBuffer();
    testInt8Array();
    testUint8Array();
    testUint8ClampedArray();
    testInt16Array();
    testUint16Array();
    testInt32Array();
    testUint32Array();
    testFloat32Array();
    testFloat64Array();
    testBigInt64Array();
    testBigUint64Array();
    testDataView();
    testDirectTransfer();
}

main();
