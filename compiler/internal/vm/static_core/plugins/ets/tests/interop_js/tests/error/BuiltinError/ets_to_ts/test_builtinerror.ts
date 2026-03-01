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

const rangeErr = etsVm.getClass('Lbuiltinerror/ETSGLOBAL;').rangeErr;
const referenceErr = etsVm.getClass('Lbuiltinerror/ETSGLOBAL;').referenceErr;
const syntaxErr = etsVm.getClass('Lbuiltinerror/ETSGLOBAL;').syntaxErr;
const uriErr = etsVm.getClass('Lbuiltinerror/ETSGLOBAL;').uriErr;
const typeErr = etsVm.getClass('Lbuiltinerror/ETSGLOBAL;').typeErr;
const customRangeError = etsVm.getClass('Lbuiltinerror/ETSGLOBAL;').customRangeError;
const customReferenceError = etsVm.getClass('Lbuiltinerror/ETSGLOBAL;').customReferenceError;
const customSyntaxError = etsVm.getClass('Lbuiltinerror/ETSGLOBAL;').customSyntaxError;
const customURIError = etsVm.getClass('Lbuiltinerror/ETSGLOBAL;').customURIError;
const customTypeError = etsVm.getClass('Lbuiltinerror/ETSGLOBAL;').customTypeError;

let GetThrowRangeError = etsVm.getFunction('Lbuiltinerror/ETSGLOBAL;', 'GetThrowRangeError');
let GetThrowReferenceError = etsVm.getFunction('Lbuiltinerror/ETSGLOBAL;', 'GetThrowReferenceError');
let GetThrowSyntaxError = etsVm.getFunction('Lbuiltinerror/ETSGLOBAL;', 'GetThrowSyntaxError');
let GetThrowURIError = etsVm.getFunction('Lbuiltinerror/ETSGLOBAL;', 'GetThrowURIError');
let GetThrowTypeError = etsVm.getFunction('Lbuiltinerror/ETSGLOBAL;', 'GetThrowTypeError');

let GetRangeError = etsVm.getFunction('Lbuiltinerror/ETSGLOBAL;', 'GetRangeError');
let GetReferenceError = etsVm.getFunction('Lbuiltinerror/ETSGLOBAL;', 'GetReferenceError');
let GetSyntaxError = etsVm.getFunction('Lbuiltinerror/ETSGLOBAL;', 'GetSyntaxError');
let GetURIError = etsVm.getFunction('Lbuiltinerror/ETSGLOBAL;', 'GetURIError');
let GetTypeError = etsVm.getFunction('Lbuiltinerror/ETSGLOBAL;', 'GetTypeError');

let GetCustomThrowRangeError = etsVm.getFunction('Lbuiltinerror/ETSGLOBAL;', 'GetCustomThrowRangeError');
let GetCustomThrowReferenceError = etsVm.getFunction('Lbuiltinerror/ETSGLOBAL;', 'GetCustomThrowReferenceError');
let GetCustomThrowSyntaxError = etsVm.getFunction('Lbuiltinerror/ETSGLOBAL;', 'GetCustomThrowSyntaxError');
let GetCustomThrowURIError = etsVm.getFunction('Lbuiltinerror/ETSGLOBAL;', 'GetCustomThrowURIError');
let GetCustomThrowTypeError = etsVm.getFunction('Lbuiltinerror/ETSGLOBAL;', 'GetCustomThrowTypeError');

const SYNTAX_MESSAGE = 'Builtin Error';
const CUSTOM_SYNTAX_MESSAGE = 'cunstom Error!';

function testCheckInstanceofRangeError(): boolean {
    let res: boolean;
    let ranError: Object = GetRangeError(SYNTAX_MESSAGE);
    let refError: Object = GetReferenceError(SYNTAX_MESSAGE);
    let synError: Object = GetSyntaxError(SYNTAX_MESSAGE);
    let uriError: Object = GetURIError(SYNTAX_MESSAGE);
    let typeError: Object = GetTypeError(SYNTAX_MESSAGE);
    res = (rangeErr instanceof RangeError) &&
        (referenceErr instanceof ReferenceError) &&
        (syntaxErr instanceof SyntaxError) &&
        (uriErr instanceof URIError) &&
        (typeErr instanceof TypeError) &&
        (ranError instanceof RangeError) &&
        (refError instanceof ReferenceError) &&
        (synError instanceof SyntaxError) &&
        (uriError instanceof URIError) &&
        (typeError instanceof TypeError) &&
        (customRangeError instanceof RangeError) &&
        (customReferenceError instanceof ReferenceError) &&
        (customSyntaxError instanceof SyntaxError) &&
        (customURIError instanceof URIError) &&
        (customTypeError instanceof TypeError);
    return res;
}

function testCheckCatchError(): boolean {
    let res: boolean;
    try {
        GetThrowRangeError();
        res = false;
    } catch (e) {
        res = (e.toString() === new RangeError().toString()) &&
            (e.message === new RangeError().message);
    }
    try {
        GetThrowReferenceError();
        res = false;
    } catch (e) {
        res = res && (e.toString() === new ReferenceError().toString()) &&
            (e.message === new ReferenceError().message);
    }
    try {
        GetThrowSyntaxError();
        res = false;
    } catch (e) {
        res = res && (e.toString() === new SyntaxError().toString()) &&
            (e.message === new SyntaxError().message);
    }
    try {
        GetThrowURIError();
        res = false;
    } catch (e) {
        res = res && (e.toString() === new URIError().toString()) &&
            (e.message === new URIError().message);
    }
    try {
        GetThrowTypeError();
        res = false;
    } catch (e) {
        res = res && (e.toString() === new TypeError().toString()) &&
            (e.message === new TypeError().message);
    }
    return res;
}

function testCheckCustomError(): boolean {
    let res: boolean;
    try {
        GetCustomThrowRangeError(CUSTOM_SYNTAX_MESSAGE);
        res = false;
    } catch (err) {
        res = (err instanceof RangeError) && 
            (err.toString() === new RangeError(CUSTOM_SYNTAX_MESSAGE).toString()) &&
            (err.message === CUSTOM_SYNTAX_MESSAGE);
    }
    try {
        GetCustomThrowReferenceError(CUSTOM_SYNTAX_MESSAGE);
        res = false;
    } catch (err) {
        res = res && (err instanceof ReferenceError) && 
            (err.toString() === new ReferenceError(CUSTOM_SYNTAX_MESSAGE).toString()) &&
            (err.message === CUSTOM_SYNTAX_MESSAGE);
    }
    try {
        GetCustomThrowSyntaxError(CUSTOM_SYNTAX_MESSAGE);
        res = false;
    } catch (err) {
        res = res && (err instanceof SyntaxError) && 
            (err.toString() === new SyntaxError(CUSTOM_SYNTAX_MESSAGE).toString()) &&
            (err.message === CUSTOM_SYNTAX_MESSAGE);
    }
    try {
        GetCustomThrowURIError(CUSTOM_SYNTAX_MESSAGE);
        res = false;
    } catch(err) {
        res = res && (err instanceof URIError) && 
            (err.toString() === new URIError(CUSTOM_SYNTAX_MESSAGE).toString()) &&
            (err.message === CUSTOM_SYNTAX_MESSAGE);
    }
    try {
        GetCustomThrowTypeError(CUSTOM_SYNTAX_MESSAGE);
        res = false;
    } catch(err) {
        res = res && (err instanceof TypeError) && 
            (err.toString() === new TypeError(CUSTOM_SYNTAX_MESSAGE).toString()) &&
            (err.message === CUSTOM_SYNTAX_MESSAGE);
    }
    return res;
}

ASSERT_TRUE(testCheckInstanceofRangeError());
ASSERT_TRUE(testCheckCatchError());
ASSERT_TRUE(testCheckCustomError());
