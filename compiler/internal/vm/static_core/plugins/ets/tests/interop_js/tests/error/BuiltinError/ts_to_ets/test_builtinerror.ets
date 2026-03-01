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

function CheckRangeError(ranErr: Object): boolean {
    return ranErr instanceof RangeError;
}
function CheckReferenceError(refErr: Object): boolean {
    return refErr instanceof ReferenceError;
}
function CheckSyntaxError(synErr: Object): boolean {
    return synErr instanceof SyntaxError;
}
function CheckURIError(uriErr: Object): boolean {
    return uriErr instanceof URIError;
}
function CheckTypeError(uriErr: Object): boolean {
    return uriErr instanceof TypeError;
}

function CheckCatchRangeError(fn: JSValue, message: string): boolean {
    let res: boolean;
    let ranErr = new RangeError(message);
    try {
        ESValue.wrap(fn).invoke();
        res = false; 
    } catch (err) {
        res = ((err).toString() === ranErr.toString()) &&
            (err).toString() === ranErr.toString() &&
            (err as RangeError).message == ranErr.message &&
            (err instanceof RangeError);
    }
    return res;
}

function CheckCatchReferenceError(fn: JSValue, message: string): boolean {
    let res: boolean;
    let ranErr = new ReferenceError(message);
    try {
        ESValue.wrap(fn).invoke();
        res = false; 
    } catch (err) {
        res = ((err).toString() === ranErr.toString()) &&
            (err).toString() === ranErr.toString() &&
            (err as ReferenceError).message == ranErr.message &&
            (err instanceof ReferenceError);
    }
    return res;
}

function CheckCatchSyntaxError(fn: JSValue, message: string): boolean {
    let res: boolean;
    let synErr = new SyntaxError(message);
    try {
        ESValue.wrap(fn).invoke();
        res = false; 
    } catch (err) {
        res = ((err).toString() === synErr.toString()) &&
            (err).toString() === synErr.toString() &&
            (err as SyntaxError).message == synErr.message &&
            (err instanceof SyntaxError);
    }
    return res;
}

function CheckCatchURIError(fn: JSValue, message: string): boolean {
    let res: boolean;
    let uriErr = new URIError(message);
    try {
        ESValue.wrap(fn).invoke();
        res = false; 
    } catch (err) {
        res = ((err).toString() === uriErr.toString()) &&
            (err).toString() === uriErr.toString() &&
            (err as URIError).message == uriErr.message &&
            (err instanceof URIError);
    }
    return res;
}

function CheckCatchTypeError(fn: JSValue, message: string): boolean {
    let res: boolean;
    let uriErr = new TypeError(message);
    try {
        ESValue.wrap(fn).invoke();
        res = false; 
    } catch (err) {
        res = ((err).toString() === uriErr.toString()) &&
            (err).toString() === uriErr.toString() &&
            (err as TypeError).message == uriErr.message &&
            (err instanceof TypeError);
    }
    return res;
}
