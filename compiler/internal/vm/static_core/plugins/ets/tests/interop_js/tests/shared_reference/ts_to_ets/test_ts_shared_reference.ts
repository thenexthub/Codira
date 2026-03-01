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

const etsVm = globalThis.gtest.etsVm;

const etsArray1 = etsVm.getClass('Lets_shared_reference/ETSGLOBAL;').tmpArr;
const etsArray2 = etsVm.getClass('Lets_shared_reference/ETSGLOBAL;').tmpArr;

let tmpArray: Array<number> = new Array<number>(1, 2, 3);
let enterFlag = 0;
function areArraysEqual1(arr: Array<number>): Array<number> {
    if (enterFlag === 0) {
        ASSERT_TRUE(tmpArray !== arr);
    } else {
        ASSERT_TRUE(tmpArray === arr);
    }
    tmpArray = arr;
    enterFlag = 1;
    return arr;
}

function areArraysEqual2(x: Array<number>): Array<number> {
    ASSERT_TRUE(etsArray1 === x);
    return x;
}

const strTime = '2024-03-01T01:02:03.000Z';
let tmpData: Date = new Date(strTime);
function areDatasEqual(data: Date): Date {
    if (enterFlag === 0) {
        ASSERT_TRUE(tmpData !== data);
    } else {
        ASSERT_TRUE(tmpData === data);
    }
    tmpData = data;
    enterFlag = 1;
    return data;
}

let tmpMap: Map<string, number> = new Map<string, number>();
function areMapsEqual(map: Map<string, number>): Map<string, number> {
    if (enterFlag === 0) {
        ASSERT_TRUE(tmpMap !== map);
    } else {
        ASSERT_TRUE(tmpMap === map);
    }
    tmpMap = map;
    enterFlag = 1;
    return map;
}

let tmpSet: Set<number> = new Set<number>();
function areSetsEqual(set: Set<number>): Set<number> {
    if (enterFlag === 0) {
        ASSERT_TRUE(tmpSet !== set);
    } else {
        ASSERT_TRUE(tmpSet === set);
    }
    tmpSet = set;
    enterFlag = 1;
    return set;
}

let tmpRangeError: RangeError = new RangeError('tmpRange error example');
function areRangeErrorEqual(error: RangeError): RangeError {
    if (enterFlag === 0) {
        ASSERT_TRUE(tmpRangeError !== error);
    } else {
        ASSERT_TRUE(tmpRangeError === error);
    }
    tmpRangeError = error;
    enterFlag = 1;
    return error;
}

let tmpSyntaxError: SyntaxError = new SyntaxError('tmpSyntax error example');
function areSyntaxErrorEqual(error: SyntaxError): SyntaxError {
    if (enterFlag === 0) {
        ASSERT_TRUE(tmpSyntaxError !== error);
    } else {
        ASSERT_TRUE(tmpSyntaxError === error);
    }
    tmpSyntaxError = error;
    enterFlag = 1;
    return error;
}

let tmpURIError: URIError = new URIError('tmpURI error example');
function areURIErrorEqual(error: URIError): URIError {
    if (enterFlag === 0) {
        ASSERT_TRUE(tmpURIError !== error);
    } else {
        ASSERT_TRUE(tmpURIError === error);
    }
    tmpURIError = error;
    enterFlag = 1;
    return error; 
}

let callbackJsFunctionEtsArray = etsVm.getFunction('Lets_shared_reference/ETSGLOBAL;', 'callbackJsFunctionEtsArray');
let callbackJsFunctionEtsDate = etsVm.getFunction('Lets_shared_reference/ETSGLOBAL;', 'callbackJsFunctionEtsDate');
let callbackJsFunctionEtsMap = etsVm.getFunction('Lets_shared_reference/ETSGLOBAL;', 'callbackJsFunctionEtsMap');
let callbackJsFunctionEtsSet = etsVm.getFunction('Lets_shared_reference/ETSGLOBAL;', 'callbackJsFunctionEtsSet');
let callbackJsFunctionEtsRangeError = etsVm.getFunction('Lets_shared_reference/ETSGLOBAL;', 'callbackJsFunctionEtsRangeError');
let callbackJsFunctionEtsSyntaxError = etsVm.getFunction('Lets_shared_reference/ETSGLOBAL;', 'callbackJsFunctionEtsSyntaxError');
let callbackJsFunctionEtsURIError = etsVm.getFunction('Lets_shared_reference/ETSGLOBAL;', 'callbackJsFunctionEtsURIError');

// The same function is called twice to verify that the parameters passed multiple times are consistent. 
// The first call has no cache, while the second call has a cache. Therefore, 
// the first call is expected to return false, and the second call is expected to return successfully.
let callBackResult = callbackJsFunctionEtsArray(areArraysEqual1);
callBackResult = callbackJsFunctionEtsArray(areArraysEqual1);
callBackResult = callbackJsFunctionEtsArray(areArraysEqual2);
ASSERT_TRUE(etsArray1 === etsArray2);

enterFlag = 0;
callBackResult = callbackJsFunctionEtsDate(areDatasEqual);
callBackResult = callbackJsFunctionEtsDate(areDatasEqual);

enterFlag = 0;
callBackResult = callbackJsFunctionEtsMap(areMapsEqual);
callBackResult = callbackJsFunctionEtsMap(areMapsEqual);

enterFlag = 0;
callBackResult = callbackJsFunctionEtsSet(areSetsEqual);
callBackResult = callbackJsFunctionEtsSet(areSetsEqual);

enterFlag = 0;
callBackResult = callbackJsFunctionEtsRangeError(areRangeErrorEqual);
callBackResult = callbackJsFunctionEtsRangeError(areRangeErrorEqual);

enterFlag = 0;
callBackResult = callbackJsFunctionEtsSyntaxError(areSyntaxErrorEqual);
callBackResult = callbackJsFunctionEtsSyntaxError(areSyntaxErrorEqual);

enterFlag = 0;
callBackResult = callbackJsFunctionEtsURIError(areURIErrorEqual);
callBackResult = callbackJsFunctionEtsURIError(areURIErrorEqual);