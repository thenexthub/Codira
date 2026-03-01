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

const CheckRangeError = etsVm.getFunction('Ltest_builtinerror/ETSGLOBAL;', 'CheckRangeError');
const CheckReferenceError = etsVm.getFunction('Ltest_builtinerror/ETSGLOBAL;', 'CheckReferenceError');
const CheckSyntaxError = etsVm.getFunction('Ltest_builtinerror/ETSGLOBAL;', 'CheckSyntaxError');
const CheckURIError = etsVm.getFunction('Ltest_builtinerror/ETSGLOBAL;', 'CheckURIError');
const CheckTypeError = etsVm.getFunction('Ltest_builtinerror/ETSGLOBAL;', 'CheckTypeError');
const CheckCatchRangeError = etsVm.getFunction('Ltest_builtinerror/ETSGLOBAL;', 'CheckCatchRangeError');
const CheckCatchReferenceError = etsVm.getFunction('Ltest_builtinerror/ETSGLOBAL;', 'CheckCatchReferenceError');
const CheckCatchSyntaxError = etsVm.getFunction('Ltest_builtinerror/ETSGLOBAL;', 'CheckCatchSyntaxError');
const CheckCatchURIError = etsVm.getFunction('Ltest_builtinerror/ETSGLOBAL;', 'CheckCatchURIError');
const CheckCatchTypeError = etsVm.getFunction('Ltest_builtinerror/ETSGLOBAL;', 'CheckCatchTypeError');

const SYNTAX_MESSAGE = 'Range Error';
const CUSTOM_SYNTAX_MESSAGE = 'cunstom Error!';

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

function testInstanceofError(): boolean {
    return CheckRangeError(new RangeError()) &&
        CheckRangeError(new RangeError() as object) &&
        CheckRangeError(new CustomRangeError(CUSTOM_SYNTAX_MESSAGE) as RangeError) &&
        CheckReferenceError(new ReferenceError()) &&
        CheckReferenceError(new ReferenceError() as object) &&
        CheckReferenceError(new CustomReferenceError(CUSTOM_SYNTAX_MESSAGE) as ReferenceError) &&
        CheckSyntaxError(new SyntaxError()) &&
        CheckSyntaxError(new SyntaxError() as object) &&
        CheckSyntaxError(new CustomSyntaxError(CUSTOM_SYNTAX_MESSAGE) as SyntaxError) &&
        CheckURIError(new URIError()) &&
        CheckURIError(new URIError() as object) &&
        CheckURIError(new CustomURIError(CUSTOM_SYNTAX_MESSAGE) as URIError) &&
        CheckTypeError(new TypeError()) &&
        CheckTypeError(new TypeError() as object) &&
        CheckTypeError(new CustomTypeError(CUSTOM_SYNTAX_MESSAGE) as TypeError);
}
function testCatchRangeError(): boolean {
    let rangeFunc1: () => void = () => { throw new RangeError(); };
    let rangeFunc2: () => void = () => { throw new RangeError(SYNTAX_MESSAGE); };
    let rangeFunc3: () => void = () => { throw new CustomRangeError(CUSTOM_SYNTAX_MESSAGE); };
    return CheckCatchRangeError(rangeFunc1, '') &&
        CheckCatchRangeError(rangeFunc2, SYNTAX_MESSAGE) &&
        CheckCatchRangeError(rangeFunc3, CUSTOM_SYNTAX_MESSAGE);
}
function testCatchReferenceError(): boolean {
    let referenceFunc1: () => void = () => { throw new ReferenceError(); };
    let referenceFunc2: () => void = () => { throw new ReferenceError(SYNTAX_MESSAGE); };
    let referenceFunc3: () => void = () => { throw new CustomReferenceError(CUSTOM_SYNTAX_MESSAGE); };
    return CheckCatchReferenceError(referenceFunc1, '') &&
        CheckCatchReferenceError(referenceFunc2, SYNTAX_MESSAGE) &&
        CheckCatchReferenceError(referenceFunc3, CUSTOM_SYNTAX_MESSAGE);
}
function testCatchSyntaxError(): boolean {
    let syntaxFunc1: () => void = () => { throw new SyntaxError(); };
    let syntaxFunc2: () => void = () => { throw new SyntaxError(SYNTAX_MESSAGE); };
    let syntaxFunc3: () => void = () => { throw new CustomSyntaxError(CUSTOM_SYNTAX_MESSAGE); };
    return CheckCatchSyntaxError(syntaxFunc1, '') &&
        CheckCatchSyntaxError(syntaxFunc2, SYNTAX_MESSAGE) &&
        CheckCatchSyntaxError(syntaxFunc3, CUSTOM_SYNTAX_MESSAGE);
}
function testCatchURIError(): boolean {
    let uriFunc1: () => void = () => { throw new URIError(); };
    let uriFunc2: () => void = () => { throw new URIError(SYNTAX_MESSAGE); };
    let uriFunc3: () => void = () => { throw new CustomURIError(CUSTOM_SYNTAX_MESSAGE); };
    return CheckCatchURIError(uriFunc1, '') &&
        CheckCatchURIError(uriFunc2, SYNTAX_MESSAGE) &&
        CheckCatchURIError(uriFunc3, CUSTOM_SYNTAX_MESSAGE);
}
function testCatchTypeError(): boolean {
    let typeFunc1: () => void = () => { throw new TypeError(); };
    let typeFunc2: () => void = () => { throw new TypeError(SYNTAX_MESSAGE); };
    let typeFunc3: () => void = () => { throw new CustomTypeError(CUSTOM_SYNTAX_MESSAGE); };
    return CheckCatchTypeError(typeFunc1, '') &&
        CheckCatchTypeError(typeFunc2, SYNTAX_MESSAGE) &&
        CheckCatchTypeError(typeFunc3, CUSTOM_SYNTAX_MESSAGE);
}

ASSERT_TRUE(testInstanceofError());
ASSERT_TRUE(testCatchRangeError());
ASSERT_TRUE(testCatchReferenceError());
ASSERT_TRUE(testCatchSyntaxError());
ASSERT_TRUE(testCatchURIError());
ASSERT_TRUE(testCatchTypeError());
