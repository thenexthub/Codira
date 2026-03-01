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

type Union = string | number;
export const tsString = 'string';
export const tsInt = 1;
const add = (a: number, b: number): number => a + b;

export function applyFunctionGenericUnion<T extends Union>(value1: T, value2: T, func: (a: T, b: T) => T): T {
    return func(value1, value2);
}

export function applyFunctionGeneric<T>(value1: T, value2: T, func: (a: T, b: T) => T): T {
    return func(value1, value2);
}

export function applyFunctionGenericTuple<T, U>(value1: T, value2: U, func: (a: T, b: U) => unknown): unknown {
    return func(value1, value2);
}

interface Addable<T> {
    x: T;
    add(): void;
}

export class Vector implements Addable<number> {
    x: number;

    constructor(x: number) {
        this.x = x;
    }

    add(): void {
        this.x = this.x + this.x;
    }

    get(): number {
        return this.x;
    }
}

class Parent<T> {
    get(a: T, b: T, fu: (a: T, b: T) => T): T {
        return fu(a, b);
    }
}

export class ClassSubset extends Parent<number> {}

export function subsetClassCallFromTs(): number {
    const GClass = new ClassSubset();

    return GClass.get(tsInt, tsInt, add);
}

export function genericExtendsClass<T extends Addable<number>>(value1: T): T {
    value1.add();
    return value1;
}

export function genericExtendsClassCallFromTs(arg: Vector): Vector {
    return genericExtendsClass(arg);
}


export function applyFunctionGenericArray<T>(values: T[], func: (accumulator: T, currentValue: T) => T): T {
    let result = values[0];
    for (let i = 1; i < values.length; i++) {
        result = func(result, values[i]);
    }
    return result;
}

export function applyFunctionGenericArrayCallFromTs(array: number[]): number {
    const add = (a: number, b: number): number => a + b;

    return applyFunctionGenericArray(array, add);
}

export function applyFunWithConstraints<T extends number, U extends string>(value1: T, value2: U, func: (a: T, b: U) => string): string {
    return func(value1, value2);
}

export function genericKeyof<T, R extends keyof T>(obj: T, keys: R[]): { obj: T, keys: R[] } {
    return {
        obj,
        keys,
    };
}

