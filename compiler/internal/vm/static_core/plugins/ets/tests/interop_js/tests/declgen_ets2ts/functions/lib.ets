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

export class Simple {
    constructor(x: double) {
        this.val = x;
    }
    val: double = 322;
};

export function SumDouble(a: double, b: double): double {
    return a + b;
}

export function SumBoxedDouble(a: Double, b: Double): Number {
    return a + b;
}

export function SumString(a: string, b: string): string {
    return a + b;
}

export function ReplaceFirst(arr: FixedArray<Simple>, e: Simple): FixedArray<Simple> {
    arr[0] = e;
    return arr;
}

export function error(msg: string): never {
  throw new Error(msg)
}

export function foo(x: string) {
  console.log(x)
}

export class X { data: number = 0 }
export function foo1(arg: X) {}

export function foo2(a: number = 1) {}

export function foo3(...nums: number[]) {}

export function foo4(a: number) {}

export native function foo5(a: number): int

export async function foo6() {}

export class A {}
export function foo7(this: A) {}

export type Callback = (arg: number) => void
export function foo8(cb : Callback) {
   cb(42)
}

export function foo9(cb: () => void) {}