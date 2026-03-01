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

let tmpArr: Array<Number> = new Array<Number>(1, 2, 3, 4);
const strTime = '2024-03-01T01:02:03.000Z';
let tmpData: Date = new Date(strTime);
let tmpMap: Map<string, number> = new Map<string, number>();
let tmpSet: Set<number> = new Set<number>();
let tmpRangeError: RangeError = new RangeError('tmpRange error example');
let tmpSyntaxError: SyntaxError = new SyntaxError('tmpSyntax error example');
let tmpURIError: URIError = new URIError('tmpURI error example');

function callbackJsFunctionEtsArray(fn: (x: Array<Number>) => Array<Number>): Array<Number> {
    return fn(tmpArr);
}

function callbackJsFunctionEtsDate(fn: (x: Date) => Date): Date {
    return fn(tmpData);
}

function callbackJsFunctionEtsMap(fn: (x: Map<string, number>) => Map<string, number>): Map<string, number> {
    return fn(tmpMap);
}

function callbackJsFunctionEtsSet(fn: (x: Set<number>) => Set<number>): Set<number> {
    return fn(tmpSet);
}

function callbackJsFunctionEtsRangeError(fn: (x: RangeError) => RangeError): RangeError {
    return fn(tmpRangeError);
}

function callbackJsFunctionEtsSyntaxError(fn: (x: SyntaxError) => SyntaxError): SyntaxError {
    return fn(tmpSyntaxError);
}

function callbackJsFunctionEtsURIError(fn: (x: URIError) => URIError): URIError {
    return fn(tmpURIError);
}