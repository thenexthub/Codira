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

let rangeErr: RangeError = new RangeError();
let referenceErr: ReferenceError = new ReferenceError();
let syntaxErr:SyntaxError = new SyntaxError();
let uriErr: URIError = new URIError();
let typeErr: TypeError = new TypeError();

function GetThrowRangeError(): void {throw rangeErr;}
function GetThrowReferenceError(): void {throw referenceErr;}
function GetThrowSyntaxError(): void {throw syntaxErr;}
function GetThrowURIError(): void {throw uriErr;}
function GetThrowTypeError(): void {throw typeErr;}

function GetRangeError(message: string): RangeError {
    return new RangeError(message);
}
function GetReferenceError(message: string): ReferenceError {
    return new ReferenceError(message);
}
function GetSyntaxError(message: string): SyntaxError {
    return new SyntaxError(message);
}
function GetURIError(message: string): URIError {
    return new URIError(message);
}
function GetTypeError(message: string): TypeError {
    return new TypeError(message);
}

class CustomRangeError extends RangeError {
    constructor(message: string) {
        super(message);
    }
}
class CustomReferenceError extends ReferenceError {
    constructor(message: string) {
        super(message);
    }
}
class CustomSyntaxError extends SyntaxError {
    constructor(message: string) {
        super(message);
    }
}
class CustomURIError extends URIError {
    constructor(message: string) {
        super(message);
    }
}
class CustomTypeError extends TypeError {
    constructor(message: string) {
        super(message);
    }
}

function GetCustomThrowRangeError(message: string): void {
    throw new CustomRangeError(message);
}
function GetCustomThrowReferenceError(message: string): void {
    throw new CustomReferenceError(message);
}
function GetCustomThrowSyntaxError(message: string): void {
    throw new CustomSyntaxError(message);
}
function GetCustomThrowURIError(message: string): void {
    throw new CustomURIError(message);
}
function GetCustomThrowTypeError(message: string): void {
    throw new CustomTypeError(message);
}

let customRangeError: CustomRangeError = new CustomRangeError('cuntomError');
let customReferenceError: CustomReferenceError = new CustomReferenceError('cuntomError');
let customSyntaxError: CustomSyntaxError = new CustomSyntaxError('cuntomError');
let customURIError: CustomURIError = new CustomURIError('cuntomError');
let customTypeError: CustomTypeError = new CustomTypeError('cuntomError');
