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

let module = ESValue.load('ts_required');
let testInEts = module.getProperty('Test');
let requiredTestInTs = module.getProperty('requiredTestInTs');
let optionalTestInTs = module.getProperty('optionalTestInTs');
let checkRequiredDataInTs = module.getProperty('checkRequiredDataInTs');
let checkOptionalDataInTs = module.getProperty('checkOptionalDataInTs');

let requiredTestInEts: ESValue = testInEts.instantiate('required in ets', 5, 6);
let optionalTestInEts: ESValue = testInEts.instantiate('optional in ets', 7, 8);

function checkRequiredDataInEts(): number {
    return requiredTestInTs.getProperty('data1').toNumber() + requiredTestInTs.getProperty('data2').toNumber();
}

function checkOptionalDataInEts(): number {
    let val: number = 0;
    if (optionalTestInTs.getProperty('data1').isUndefined()) {
        console.log('data1 = undefined');
    } else {
        console.log('data1 = ' + optionalTestInTs.getProperty('data1').toNumber());
        val += optionalTestInTs.getProperty('data1').toNumber();
    }
    if (optionalTestInTs.getProperty('data2').isUndefined()) {
        console.log('data2 = undefined');
    } else {
        console.log('data2 = ' + optionalTestInTs.getProperty('data2').toNumber());
        val += optionalTestInTs.getProperty('data2').toNumber();
    }
    return val;
}

function checkRequiredDataNewInEts(): number {
    return requiredTestInEts.getProperty('data1').toNumber() + requiredTestInEts.getProperty('data2').toNumber();
}

function checkOptionalDataNewInEts(): number {
    return optionalTestInEts.getProperty('data1').toNumber() + optionalTestInEts.getProperty('data2').toNumber();
}

function checkDataEts(): boolean {
    let ret: boolean = true;
    ret = ret && checkRequiredDataInTs.invoke(requiredTestInEts).toNumber() === 11;
    ret = ret && checkRequiredDataInTs.invoke(optionalTestInEts).toNumber() === 15;
    ret = ret && checkOptionalDataInTs.invoke(requiredTestInEts).toNumber() === 11;
    ret = ret && checkOptionalDataInTs.invoke(optionalTestInEts).toNumber() === 15;
    return ret;
}
