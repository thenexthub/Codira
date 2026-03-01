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

function functionOptDefaultArgTypeAnyEts(arg: JSValue = 1): JSValue {
    return arg;
}

function functionOptDefaultArgTypeUnknownEts(arg: JSValue = 1): JSValue {
    return arg;
}

function functionOptDefaultArgTypeUndefinedEts(arg: JSValue = 1): JSValue {
    return arg;
}

function functionOptDefaultArgTypeNumberEts(arg: number = 1): number {
    return arg;
}

function functionOptDefaultArgTypeByteEts(arg: byte = 1 as byte): byte {
    return arg;
}

function functionOptDefaultArgTypeShortEts(arg: short = 1 as short): short {
    return arg;
}

function functionOptDefaultArgTypeIntEts(arg: int = 1): int {
    return arg;
}

function functionOptDefaultArgTypeLongEts(arg: long = 1): long {
    return arg;
}

function functionOptDefaultArgTypeFloatEts(arg: float = 1): float {
    return arg;
}

function functionOptDefaultArgTypeCharEts(arg: char = c'c'): char {
    return arg;
}

function functionOptDefaultArgTypeBooleanEts(arg: Boolean = true): Boolean {
    return arg;
}

function functionOptDefaultArgTypeStringEts(arg: string = 'test'): string {
    return arg;
}

function functionOptDefaultArgTypeObjectEts(arg: JSValue = {id: 1}): JSValue {
    return arg;
}

let defaultClass: ClassWithMethodEts;

function functionOptDefaultArgTypeClassEts(arg: ClassWithMethodEts = defaultClass): number {
    return arg.methodInClass();
}

function functionOptDefaultArgTypeArrayEts(arg: FixedArray<string> = ['one', 'two']): FixedArray<string> {
    return arg;
}

 function functionOptDefaultArgTypeTupleEts(arg: [number, string] = [1, 'test'] as [number, string]): number {
     return arg[0];
}

function functionOptDefaultArgTypeUnionEts(arg: U = 'test'): U {
    return arg;
}

function functionOptDefaultArgTypeCallableEts(functionToCall: Function<int>): int {
    return 1;
}
