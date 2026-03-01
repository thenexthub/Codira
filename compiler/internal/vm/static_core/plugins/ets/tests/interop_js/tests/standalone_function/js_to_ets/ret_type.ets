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

import { functionReturnTypeAny, functionReturnTypeUnknown, functionReturnTypeUndefined,
    JsClassAsReturnValue, functionReturnTypeDouble, functionReturnTypeByte,
    functionReturnTypeShort, functionReturnTypeInt, functionReturnTypeLong,
    functionReturnTypeFloat, functionReturnTypeChar, functionReturnTypeBoolean,
    functionReturnTypeString, functionReturnTypeObject, functionReturnTypeClass,
    functionReturnTypeArray, functionReturnTypeCallable }
    from "ret_type"

function TestFunctionReturnTypeAny(): boolean {
    return functionReturnTypeAny() as int == 1;
}

function TestFunctionReturnTypeUnknown(): boolean {
    return functionReturnTypeUnknown() as int == 1;
}

function TestFunctionReturnTypeUndefined(): boolean {
    return functionReturnTypeUndefined() as int == 1;
}

function TestFunctionReturnTypeDouble(): boolean {
    return functionReturnTypeDouble() as double == 1.7976931348623157e+308;
}

function TestFunctionReturnTypeByte(): boolean {
    let res: byte = functionReturnTypeByte() as byte;
    return res == -1;
}

// NOTE: (vladimiralpavlov) This test causes a termination with a core dump
// because the JS can return only number (int) value which cannot be casted
// to short type
function TestFunctionReturnTypeShort(): boolean {
    let res: Short = functionReturnTypeShort() as Short;
    return res == -1;
}

function TestFunctionReturnTypeInt(): boolean {
    let res: int = functionReturnTypeInt() as int;
    return res == -1;
}

function TestFunctionReturnTypeLong(): boolean {
    let res: long = functionReturnTypeLong() as long;
    return res == 9007199254740991;
}

function TestFunctionReturnTypeFloat(): boolean {
    return functionReturnTypeFloat() as number == 3.140000104904175;
}

function TestFunctionReturnTypeChar(): boolean {
    return functionReturnTypeChar() as char == c'c';
}

function TestFunctionReturnTypeBoolean(): boolean {
    let res: Boolean = functionReturnTypeBoolean() as Boolean;
    return res;
}

function TestFunctionReturnTypeString(): boolean {
    return functionReturnTypeString() as string == 'string';
}

interface ObjectWithId {
    id: number
}

class ArkTSObjectAsParameter implements ObjectWithId {
    public id: number = 1
}

function TestFunctionReturnTypeObject(): boolean {
  let c = functionReturnTypeObject();
  return c.id as int == 1;
}

function TestFunctionReturnTypeClass(): boolean {
    let c: JsClassAsReturnValue = functionReturnTypeClass() as JsClassAsReturnValue;
    return c.method() as int == 1;
}

function TestFunctionReturnTypeArray(): boolean {
    let arr: FixedArray<int> = functionReturnTypeArray() as FixedArray<int>;
    return arr[0] == 1;
}

// NOTE (vladimiralpavlov) uncomment below after interop is implemented in this direction
function TestFunctionReturnTypeCallable(): boolean {
    console.log(functionReturnTypeCallable()())
    return functionReturnTypeCallable()() as int == 1;
}
