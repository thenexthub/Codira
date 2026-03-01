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

function foo1(inArr: number[]): string {
    return JSON.stringify(inArr);
}

type Typ1 = (x: number[]) => string;
let fun1: Typ1 = foo1;

let var1: int = 2;
let var2: readonly [number, string] = [123, 'abc'];
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('fun1:' + fun1([var1, var2[0]]));

let math1 = (a: number, b: number): number => {
    return a + b;
};

let math2 = (a: number) => a * a;

function math3(a: number, b: number, ...args: number[]) {
    let res = a + b;
    args.forEach((value: number) => {
        res += value;
    });

    return res;
}

// CC-OFFNXT(G.AOD.03) for test purposes
console.log('mathTest', `${math1(1, 2)} ${math2(4)}`);

function buildName(firstName: string, ...restOfName: string[]) {
    return firstName + ' ' + restOfName.join(' ');
}

let buildNameFun: (fname: string, ...rest: string[]) => string = buildName;
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('buildNameFunTest', buildNameFun('abc', '123', 'zxc'));

interface GenericIdentityFn<T> {
    foo: (arg: T) => T;
}

function identity<T>(arg: T): T {
    return arg;
}

let myIdentity: GenericIdentityFn<number> = {
    foo: (arg: number) => identity<number>(arg)
};
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('GenericIdentityFn test', myIdentity.foo(789) === 789);

function memB(): string {
    return 'memBStr';
}

// CC-OFFNXT(G.AOD.03) for test purposes
console.log(`ret ${memB}`);

class C1 {
    public static field1: (x: int, y: int) => int = (x: int, y: int): int => {
        return x + y;
    };
}

// CC-OFFNXT(G.AOD.03) for test purposes
console.log(`C1 ${C1.field1(1, 2)}`);