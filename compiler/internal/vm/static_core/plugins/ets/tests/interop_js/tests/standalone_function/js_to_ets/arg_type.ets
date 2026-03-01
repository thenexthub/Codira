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

import { functionArgTypeIdentical, functionArgTypeObject, functionArgTypeClass,
    functionArgTypeArray, functionArgTypeTuple, functionArgTypeCallable }
    from "arg_type"

const functionToCall = () :int => {
    return 1;
}

function TestFunctionArgTypeAny(): boolean {
    let arg: JSValue = 1;
    return functionArgTypeIdentical(arg) as int == 1;
}

function TestFunctionArgTypeUnknown(): boolean {
    let arg: JSValue = 1;
    return functionArgTypeIdentical(arg) as int == 1;
}

function TestFunctionArgTypeUndefined(): boolean {
    let arg: JSValue = 1;
    return functionArgTypeIdentical(arg) as int == 1;
}

function TestFunctionArgTypeDouble(): boolean {
    let arg: double = 1.7976931348623157e+308;
    return functionArgTypeIdentical(arg) as double == 1.7976931348623157e+308;
}

function TestFunctionArgTypeDoubleLit(): boolean {
    return functionArgTypeIdentical(1.7976931348623157e+308) as double == 1.7976931348623157e+308;
}

function TestFunctionArgTypeByte(): boolean {
    let arg: byte = -128;
    return functionArgTypeIdentical(arg) as byte == -128;
}

function TestFunctionArgTypeByteLit(): boolean {
    return functionArgTypeIdentical(-128) as byte == -128;
}

function TestFunctionArgTypeShort(): boolean {
    let arg: Short = -32768;
    return functionArgTypeIdentical(arg) as Short == -32768;
}

function TestFunctionArgTypeShortLit(): boolean {
    return functionArgTypeIdentical(-32768) as Short == -32768;
}

function TestFunctionArgTypeInt(): boolean {
    let arg: int = -2147483648;
    return functionArgTypeIdentical(arg) as int == -2147483648;
}

function TestFunctionArgTypeIntLit(): boolean {
    return functionArgTypeIdentical(-2147483648) as int == -2147483648;
}

function TestFunctionArgTypeLong(): boolean {
    let arg: long = 9007199254740991; // Max safe integer value in JS (IEEE 754 64-bit)
    return functionArgTypeIdentical(arg) as long == 9007199254740991;
}

function TestFunctionArgTypeLongLit(): boolean {
    return functionArgTypeIdentical(9007199254740991) as long == 9007199254740991; // Max safe integer value in JS (IEEE 754 64-bit)
}

function TestFunctionArgTypeFloat(): boolean {
    let arg: float = 3.14f;
    return functionArgTypeIdentical(arg) as float == 3.14f;
}

function TestFunctionArgTypeFloatLit(): boolean {
    return functionArgTypeIdentical(3.14f) as float == 3.14f;
}

function TestFunctionArgTypeChar(): boolean {
    let arg: char = c'c';
    return functionArgTypeIdentical(arg) as char == c'c';
}

function TestFunctionArgTypeCharLit(): boolean {
    return functionArgTypeIdentical(c'c') as char == c'c';
}

function TestFunctionArgTypeBoolean(): boolean {
    let arg: Boolean = true;
    return functionArgTypeIdentical(arg) as Boolean;
}

function TestFunctionArgTypeBooleanLit(): boolean {
    return functionArgTypeIdentical(true) as Boolean;
}

function TestFunctionArgTypeString(): boolean {
    let arg: string = 'string';
    return functionArgTypeIdentical(arg) as string == 'string';
}

function TestFunctionArgTypeStringLit(): boolean {
    return functionArgTypeIdentical('string') as string == 'string';
}

class ArkTSObjectAsParameter {
    public id: number = 1
}

function TestFunctionArgTypeObject(): boolean {
    let arg: ArkTSObjectAsParameter = new ArkTSObjectAsParameter();
    return functionArgTypeObject(arg) as int == 1;
}

class ArkTSClassAsParameter {
    method(): int {
        return 1;
    }
}

function TestFunctionArgTypeClass(): boolean {
    let arg: ArkTSClassAsParameter = new ArkTSClassAsParameter();
    return functionArgTypeClass(arg) as int == 1;
}

function TestFunctionArgTypeArray(): boolean {
    let arg: FixedArray<int> = [1, 2, 3];
    return functionArgTypeArray(arg) as int == 1;
}

function TestFunctionArgTypeTuple(): boolean {
    let arg: [int, string] = [1, "one"] as [int, string];
    return functionArgTypeTuple(arg) as int == 1;
}

function TestFunctionArgTypeCallable(): boolean {
    return functionArgTypeCallable(functionToCall) as int == 1;
}
