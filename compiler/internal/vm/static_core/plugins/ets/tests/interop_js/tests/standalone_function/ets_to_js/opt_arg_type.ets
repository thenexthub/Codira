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

package interopStandaloneFnTest

function functionOptArgTypeAnyEts(arg?: JSValue): JSValue {
    let x: JSValue = 1;
    return arg !== undefined ? arg as JSValue : x;
}

function functionOptArgTypeUndefinedEts(arg?: JSValue): JSValue {
    let x: JSValue = 1;
    return arg !== undefined ? arg as JSValue : x;
}

function functionOptArgTypeUnknownEts(arg?: JSValue): JSValue {
    let x: JSValue = 1;
    return arg !== undefined ? arg as JSValue : x;
}

function functionOptArgTypeNumberEts(arg?: number): number {
    let x: number = 1;
    return arg ?? x;
}

function functionOptArgTypeByteEts(arg?: byte): byte {
    let x: byte = 1;
    return arg ?? x;
}

function functionOptArgTypeShortEts(arg?: short): short {
    let x: short = 1;
    return arg ?? x;
}

function functionOptArgTypeIntEts(arg?: int): int {
    let x: int = 1;
    return arg ?? x;
}

function functionOptArgTypeLongEts(arg?: long): long {
    let x: long = 1;
    return arg ?? x;
}

function functionOptArgTypeFloatEts(arg?: float): float {
    let x: float = 1;
    return arg ?? x;
}

function functionOptArgTypeCharEts(arg?: char): char {
    if (arg == undefined) {
        return c'c' as char
    } else {
        return arg
    }
}

function functionOptArgTypeBooleanEts(arg?: Boolean): Boolean {
    let x: Boolean = true;
    return arg ?? x;
    // NOTE(vladimiralpavlov) the code below causes core to dump with message:
    // Type mismatch after Expression::Compile: Boolean instead of boolean
    // return arg ?? 1;
}

function functionOptArgTypeStringEts(arg?: string): string {
    return arg ?? 'test';
}

function functionOptArgTypeObjectEts(arg?: JSValue): JSValue {
    let x: JSValue = {id: 1};
    return arg ?? x;
}

function functionOptArgTypeClassEts(arg?: ClassWithMethodEts): number {
    return arg?.methodInClass() ?? 1;
}

function functionOptArgTypeArrayEts(arg?: Array<string>): FixedArray<string> | Array<string> {
    return arg ?? ['one', 'two'];
}

type TestTupleType = [number, string];

function functionOptArgTypeTupleEts(arg?: TestTupleType): number {
    const returnValue = arg !== undefined ? arg as TestTupleType : [ 2, 'string' ] as TestTupleType;
    return returnValue[0];
}

function functionOptArgTypeUnionEts(arg?: U): U {
    return arg ?? 'test';
}


function functionOptArgTypeCallableEts(functionToCall?: () => Function<int>): () => int {
    if (functionToCall !== undefined) {
        return functionToCall() as () => int
    }
    return () => 1;
}
