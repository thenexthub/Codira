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

let module1 = ESValue.load('indexed_type');
let getArray = module1.getProperty('getArray');
let returnCustomArray = module1.getProperty('returnCustomArray');
let createStringRecord = module1.getProperty('createStringRecord');
let createInt8Array = module1.getProperty('createInt8Array');
let handler = module1.getProperty('handler');

let module2 = ESValue.load('proxy');
let createProxy = module2.getProperty('createProxy')

const arr: FixedArray<string> = ['A', 'B', 'C'];

function getArrValueByIndex(): boolean {
    const jsArray = getArray.invoke(arr).unwrap() as Array<string>;
    return jsArray[0] === 'A';
}

function changeArrValueByIndex(): boolean {
    let jsArray = getArray.invoke(arr).unwrap() as Array<string>;
    jsArray[0] = 'D';
    return jsArray[0] === 'D';
}

function checkLengthArr(): boolean {
    const jsArray: Array<string> = getArray.invoke(arr).unwrap() as Array<string>;
    return jsArray.length.toDouble() === arr.length.toDouble();
}

function checkAllArrValue(): boolean {
    const jsArray: Array<string> = getArray.invoke(arr).unwrap() as Array<string>;
    return jsArray[0] === 'A' && jsArray[1] === 'B' && jsArray[2] === 'C';
}

function getCustomArrValueByIndex(): boolean {
    const jsArray = returnCustomArray.invoke(arr);
    return jsArray.getProperty(0).toString() === 'A';
}

function changeCustomArrValueByIndex(): boolean {
    let jsArray = returnCustomArray.invoke(arr).unwrap() as Array<string>;
    jsArray[0] = 'D';
    return jsArray[0] === 'D';
}

function checkLengthCustomArr(): boolean {
    const jsArray: Array<string> = returnCustomArray.invoke(arr).unwrap() as Array<string>;
    return jsArray.length.toDouble() === arr.length.toDouble();
}

function checkAllCustomArrValue(): boolean {
    const jsArray: Array<string> = returnCustomArray.invoke(arr).unwrap() as Array<string>;
    return jsArray[0] === 'A' && jsArray[1] === 'B' && jsArray[2] === 'C';
}

function getRecordValue(): boolean {
    const recordObj = createStringRecord.invoke();
    return recordObj.getProperty('key1').toString() === 'A';
}

function changeRecordValue(): boolean {
    let recordObj = createStringRecord.invoke();
    recordObj.setProperty('key1', 'D');
    return recordObj.getProperty('key1').toString() === 'D';
}
// NOTE(andreypetukhov) enable after fixibng #17821
//  function getRecordValueIndex():boolean{
//     const recordObj:Record<string, string> = createStringRecord();
//     return recordObj['key1'] == 'A'
//  }
// NOTE(andreypetukhov) enable after fixibng #17821
//  function changeRecordValueIndex():boolean{
//     let recordObj:Record<string, string> = createStringRecord();
//     recordObj['key1'] = 'F';
//     return recordObj['key1'] == 'F';
//  }
// NOTE(andreypetukhov) enable after fixibng #17821
// function checkRecordValueLength():boolean{
//     const recordObj:Record<string, string> = createStringRecord();
//     return Object.entries(recordObj).length as number == 3
//  }

// NOTE(andreypetukhov) enable after fixibng #17821
//  function checkRecordValue():boolean{
//   const recordObj:Record<string, string> = createStringRecord();
//   return recordObj['key1'] == 'A' && recordObj['key2'] == 'B' && recordObj['key3'] == 'C'
// }

function getTypedArrayValueByIndex(): boolean {
    const jsTypedArray = createInt8Array.invoke(3);
    return jsTypedArray.getProperty(0).toNumber() === 0;
}
// NOTE(andreypetukhov) enable after fixibng #18131
// function changeTypedArrayValueByIndex():boolean{
//     const jsTypedArray = createInt8Array(3) as Int8Array;
//     jsTypedArray[1] = 99;
//     return jsTypedArray[1] as number == 99;
// }

function checkLengthTypedArray(): boolean {
    const jsTypedArray = createInt8Array.invoke(3).unwrap() as Int8Array;
    return jsTypedArray.length.toDouble() === 3 as number;
}

function checkAllTypedArrayValue(): boolean {
    const jsTypedArray = createInt8Array.invoke(3).unwrap() as Int8Array;
    return jsTypedArray[0] as number === 0 &&
        jsTypedArray[1] as number === 1 &&
        jsTypedArray[2] as number === 2;
}

type RecordObject = Record<string, string>;
const testObj: RecordObject = {
    'keyA': 'A',
    'keyB': 'B',
}

function getValueFromProxyRecord(): boolean {
    const proxyRecord = createProxy.invoke(testObj, handler);
    return proxyRecord.getProperty('keyA').toString() === 'A';
}

function changeValueFromProxyRecord(): boolean {
    const proxyRecord = createProxy.invoke(testObj, handler);
    proxyRecord.setProperty('keyB', 'T');
    return proxyRecord.getProperty('keyB').toString() === 'T';
}
// NOTE(andreypetukhov) enable after fixibng #18238
//  function getValueFromProxyByKeyRecord():boolean{
//     const proxyRecord = createProxy(testObj,handler)
//     return proxyRecord['keyA'] as string == 'A'
//  }
