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

let module = ESValue.load('js_code');

let tmpFunc1: Function = () => {};
function areFuncsEqual1(func: Function): boolean {
    let res: boolean = tmpFunc1 === func;
    tmpFunc1 = func;
    return res;
}

let tmpFunc2: ()=>void = () => {};
function areFuncsEqual2(func: ()=>void): boolean {
    let res: boolean = tmpFunc2 === func;
    tmpFunc2 = func;
    return res;
}

let tmpArray: number[] = [];
function areArraysEqual1(arr: Array<number>): boolean {
    let res: boolean = tmpArray === arr;
    tmpArray = arr;
    return res;
}

function areArraysEqual2(arr1: Array<number>, arr2: Array<number>): boolean {
    let res: boolean = arr1 === arr2;
    return res;
}

let jsArray1: ESValue;
let jsArray2: ESValue;
function areArraysEqual3(arr1: Array<number>){
    jsArray1 = module.getProperty('jsArray');
    jsArray2 = module.getProperty('jsArray');
    let o1 =  module.getProperty('o');
    let o2 =  module.getProperty('o');
    let res: boolean = ESValue.areEqual(jsArray1, jsArray2) && ESValue.areStrictlyEqual(jsArray1, jsArray2) 
                       && ESValue.areEqual(o1, o2) && ESValue.areStrictlyEqual(o1, o2)
                       && ESValue.areEqual(jsArray1, ESValue.wrap(arr1)) && !(jsArray1 === ESValue.wrap(arr1));
    return res;
}

const strTime = '2024-03-01T01:02:03.000Z';
let tmpData: Date = new Date(strTime);
function areDatasEqual(data: Date): boolean {
    let res: boolean = tmpData === data;
    tmpData = data;
    return res;
}

let tmpMap: Map<string, number> = new Map<string, number>();
function areMapsEqual(map: Map<string, number>): boolean {
    let res: boolean = tmpMap === map;
    tmpMap = map;
    return res;
}

let tmpSet: Set<number> = new Set<number>();
function areSetsEqual(set: Set<number>): boolean {
    let res: boolean = tmpSet === set;
    tmpSet = set;
    return res;
}

let tmpRangeError: RangeError = new RangeError('tmpRange error example');
function areRangeErrorEqual(error: RangeError): boolean {
    let res: boolean = tmpRangeError === error;
    tmpRangeError = error;
    return res;
}

let tmpSyntaxError: SyntaxError = new SyntaxError('tmpSyntax error example');
function areSyntaxErrorEqual(error: SyntaxError): boolean {
    let res: boolean = tmpSyntaxError === error;
    tmpSyntaxError = error;
    return res;
}

let tmpURIError: URIError = new URIError('tmpURI error example');
function areURIErrorEqual(error: URIError): boolean {
    let res: boolean = tmpURIError === error;
    tmpURIError = error;
    return res;
}