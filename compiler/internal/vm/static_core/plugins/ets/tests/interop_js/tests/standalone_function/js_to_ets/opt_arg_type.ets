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

import { functionArgTypeDoubleOpt, functionArgTypeByteOpt, functionArgTypeShortOpt,
    functionArgTypeIntOpt, functionArgTypeLongOpt, functionArgTypeFloatOpt,
    functionArgTypeCharOpt, functionArgTypeBooleanOpt, functionArgTypeStringOpt,
    functionArgTypeObjectOpt, functionArgTypeClassOpt, functionArgTypeArrayOpt,
    functionArgTypeTupleOpt, functionArgTypeCallableOpt }
    from "opt_arg_type"

const functionToCall = () :int => {
    return 1;
}

function TestFunctionArgTypeDoubleOpt(): boolean {
    let arg: double = 1.7976931348623157e+308;
    return functionArgTypeDoubleOpt(arg) as double == functionArgTypeDoubleOpt() as double;
}

function TestFunctionArgTypeByteOpt(): boolean {
    let arg: byte = -128;
    return functionArgTypeByteOpt(arg) as byte == functionArgTypeByteOpt() as byte;
}

function TestFunctionArgTypeShortOpt(): boolean {
    let arg: Short = -32768;
    return functionArgTypeShortOpt(arg) as Short == functionArgTypeShortOpt(arg) as Short;
}

function TestFunctionArgTypeIntOpt(): boolean {
    let arg: int = -2147483648;
    return functionArgTypeIntOpt(arg) as int == functionArgTypeIntOpt() as int;
}

function TestFunctionArgTypeLongOpt(): boolean {
    let arg: long = 9007199254740991; // Max safe integer value in JS (IEEE 754 64-bit)
    return functionArgTypeLongOpt(arg) as long == functionArgTypeLongOpt() as long;
}

function TestFunctionArgTypeFloatOpt(): boolean {
    let arg: float = 3.14;
    return functionArgTypeFloatOpt(arg) as float == functionArgTypeFloatOpt(arg) as float;
}

function TestFunctionArgTypeCharOpt(): boolean {
    let arg: Char = c'c';
    return functionArgTypeCharOpt(arg) as Char == functionArgTypeCharOpt() as Char;
}

function TestFunctionArgTypeBooleanOpt(): boolean {
    return functionArgTypeBooleanOpt(true) as Boolean == functionArgTypeBooleanOpt() as Boolean;
}

function TestFunctionArgTypeStringOpt(): boolean {
    let arg: string = 'test';
    return functionArgTypeStringOpt(arg) as string == functionArgTypeStringOpt() as string;
}

class ArkTSObjectAsParameter {
    public id: number = 1
}

function TestFunctionArgTypeObjectOpt(): boolean {
    let arg: ArkTSObjectAsParameter = new ArkTSObjectAsParameter();
    return functionArgTypeObjectOpt(arg) as int == functionArgTypeObjectOpt() as int;
}

class ArkTSClassAsParameter {
    method(): int {
        return 1;
    }
}

function TestFunctionArgTypeClassOpt(): boolean {
    let arg: ArkTSClassAsParameter = new ArkTSClassAsParameter();
    return functionArgTypeClassOpt(arg) as int == functionArgTypeClassOpt() as int;
}

function TestFunctionArgTypeArrayOpt(): boolean {
    let arg: FixedArray<int> = [1, 2, 3];
    return functionArgTypeArrayOpt(arg) as int == functionArgTypeArrayOpt() as int;
}

function TestFunctionArgTypeTupleOpt(): boolean {
    let arg: [int, string] = [1, 'test'];
    return functionArgTypeTupleOpt(arg) as int == functionArgTypeTupleOpt() as int;
}

function TestFunctionArgTypeCallableOpt(): boolean {
  return functionArgTypeCallableOpt(functionToCall) as int == functionArgTypeCallableOpt() as int;
}
