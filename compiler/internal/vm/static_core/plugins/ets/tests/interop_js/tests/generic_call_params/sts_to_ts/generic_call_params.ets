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

package generic_call_params.test;

type Union = string | int;
const etsString = 'string';
const etsInt = 1;
const privateEtsInt = 2;
const add:(a: int, b: int) => int;
static {
    add = (a: int, b: int): int => a + b;
}

function applyFunctionGenericUnion<T extends Union>(value1: T, value2: T, func: (a: T, b: T) => T): T {
    return func(value1, value2);
}

function applyFunctionGeneric<T>(value1: T, value2: T, func: (a: T, b: T) => T): T {
    return  func(value1, value2);
}

function applyFunctionGenerics<T, U>(value1: T, value2: U, func: (a: T, b: U) => U): U {
    return func(value1, value2);
}

interface Addable<T> {
    x: T;
    add(): void;
}

class Vector implements Addable<Int> {
    x: Int = 1;

    constructor(x: Int) {
        this.x = x;
    }

    add(): void {
        this.x = this.x + this.x;
    }

    get(): Int {
        return this.x
    }
}

function genericExtendsClass<T extends Addable<Int>>(value1: T): T {
    value1.add();
    return value1;
}

function genericExtendsClassCallFromSts(arg: Vector): Vector {
    return genericExtendsClass(arg);
}


function applyFunctionGenericArray<T>(values: FixedArray<T>, func: (accumulator: T, currentValue: T) => T): T {
    let result = values[0];
    for (let i = 1; i < values.length; i++) {
        result = func(result, values[i]);
    }
    return result;
}

// See #25111. FixedArray<Int> to FixedArray<T> assignability.
// function applyFunctionGenericArrayCallFromSts(array: FixedArray<Int>): int {
//     return applyFunctionGenericArray(array, add);
// }

// NOTE: issue (18007) primitive constrain
// type GNumber<T extends int> = T;
// type GString<T extends string> = T;

// function applyFunctionWithConstraints<GNumber, GString>(value1: GNumber, value2: GString, func: (a: GNumber, b: GString) => string): string {
//     return func(value1, value2);
// }

// function applyFunctionWithConstraintsFromSts(value1: int, value2: string): string {
//     const concat = (a, b) => a + ' ' + b;
//     return applyFunctionWithConstraints(value1, value2, concat);
// }

class Parent<T> {
    get(a: T, b: T, fu: (a: T, b: T) => T): T {
        return fu(a, b);
    }
}

class ClassSubset extends Parent<int> {}

function subsetClassCallFromSts(): int {
    const GClass = new ClassSubset();

    return GClass.get(etsInt, etsInt, add);
}
