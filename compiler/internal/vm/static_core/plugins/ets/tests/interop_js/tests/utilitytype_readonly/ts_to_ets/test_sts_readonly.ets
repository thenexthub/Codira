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

let module = ESValue.load('ts_readonly');
let testInEts = module.getProperty('Test');
let readonlyTestInTs = module.getProperty('readonlyTestInTs');
let writableTestInTs = module.getProperty('writableTestInTs');
let checkReadonlyDataInTs = module.getProperty('checkReadonlyDataInTs');
let checkWritableDataInTs = module.getProperty('checkWritableDataInTs');

let readonlyTestInEts: ESValue = testInEts.instantiate('new in ets', 5, 6);
let writableTestInEts: ESValue = testInEts.instantiate('new in ets', 7, 8);

function checkReadonlyDataFromTs(): number {
    console.log(readonlyTestInTs.getProperty('data1').toNumber());
    console.log(readonlyTestInTs.getProperty('data2').toNumber());
    return readonlyTestInTs.getProperty('data1').toNumber() + readonlyTestInTs.getProperty('data2').toNumber();
}

function checkWritableDataFromTs(): number {
    console.log(writableTestInTs.getProperty('data1').toNumber());
    console.log(writableTestInTs.getProperty('data2').toNumber());
    return writableTestInTs.getProperty('data1').toNumber() + writableTestInTs.getProperty('data2').toNumber();
}

function checkReadonlyDataInEts(): number {
    console.log(readonlyTestInEts.getProperty('data1').toNumber());
    console.log(readonlyTestInEts.getProperty('data2').toNumber());
    return readonlyTestInEts.getProperty('data1').toNumber() + readonlyTestInEts.getProperty('data2').toNumber();
}

function checkWritableDataInEts(): number {
    console.log(writableTestInEts.getProperty('data1').toNumber());
    console.log(writableTestInEts.getProperty('data2').toNumber());
    return writableTestInEts.getProperty('data1').toNumber() + writableTestInEts.getProperty('data2').toNumber();
}

function checkDataEts(): boolean {
    let ret: boolean = true;
    ret = ret && checkReadonlyDataInTs.invoke(readonlyTestInEts).toNumber() === 11;
    ret = ret && checkReadonlyDataInTs.invoke(writableTestInEts).toNumber() === 15;
    ret = ret && checkWritableDataInTs.invoke(readonlyTestInEts).toNumber() === 11;
    ret = ret && checkWritableDataInTs.invoke(writableTestInEts).toNumber() === 15;
    return ret;
}
