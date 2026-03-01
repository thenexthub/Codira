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

function functionArgTypeAnyEts(arg: JSValue): JSValue {
    return arg;
}

function functionArgTypeUndefinedEts(arg: JSValue): JSValue {
    return arg;
}

function functionArgTypeUnknownEts(arg: JSValue): JSValue {
    return arg;
}

function functionArgTypeNumberEts(arg: number): number {
    return arg;
}

function functionArgTypeByteEts(arg: byte): byte {
    let x: byte = arg;
    return x;
}

function functionArgTypeShortEts(arg: Short): Short {
    let x: Short = arg;
    return x;
}

function functionArgTypeIntEts(arg: int): int {
    let x: int = arg;
    return x;
}

function functionArgTypeLongEts(arg: long): long {
    let x: long = arg;
    return x;
}

function functionArgTypeFloatEts(arg: float): float {
    let x: float = arg;
    return x;
}

function functionArgTypeCharEts(arg: Char): Char {
    let x: Char = arg;
    return x;
}

function functionArgTypeBooleanEts(arg: Boolean): Boolean {
    let x: Boolean = arg;
    return x;
}

function functionArgTypeStringEts(arg: string): string {
    let x: string = arg;
    return x;
}

function functionArgTypeObjectEts(arg: RefObject): RefObject {
    let x: RefObject = arg;
    return x;
}

function functionArgTypeClassEts(arg: ClassWithMethodEts): number {
    let x: ClassWithMethodEts = arg;
    return x.methodInClass();
}

function functionArgTypeArrayEts(arg: FixedArray<string>): FixedArray<string> {
    let x: FixedArray<string> = arg;
    return x;
}

function functionArgTypeTupleEts(arg: [number, string]): number {
    let x: [number, string] = arg;
    return x[0];
}

function functionArgTypeUnionEts(arg: U): U {
    let x: U = arg;
    return x;
}

function functionArgTypeCallableEts(arg: () => JSValue): JSValue {
    let x: () => JSValue = arg;
    return x();
}

function functionArgTypeInterfaceEts(arg: ImplInterfaceWithMethodEts): number {
    return arg.methodInInterface();
}
