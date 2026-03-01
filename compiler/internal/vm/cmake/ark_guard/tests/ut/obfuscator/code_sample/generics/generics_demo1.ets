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

let nums = [0, 1, 2, 3, 4, 5];
function loggingIdentity1<T>(arg: T[]): T[] {
    return arg;
}
// CC-OFFNXT(G.AOD.03) for test purposes
console.log(loggingIdentity1(nums).length === 6);
// CC-OFFNXT(G.AOD.03) for test purposes
console.log(loggingIdentity1(nums).indexOf(1) === 1);

class ClsA {
    method<T>(value: T) {
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('A ' + typeof value);
    }
}
let instanceA = new ClsA()
instanceA.method<string>('abc');

type MyArray<T> = T[]
let array: MyArray<boolean> = [true, false]
// CC-OFFNXT(G.AOD.03) for test purposes
console.log(loggingIdentity1(array).length === 2);

function foo<G>(x: G, y: G) {
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log('foo ' + x + y);
}
foo(12, 34);

class C2 <T1, T2 = number, T3 = string> {
    foo(value1: T1, value2: T2, value3: T3) {
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('C2', `${typeof value1}, ${typeof value2}, ${typeof value3}`);
    }
}
let c1 = new C2<number>()
c1.foo(1, 2, '');
let c2 = new C2<number, string>
c2.foo(1, '', '');
let c3 = new C2<number, Object, number> // all 3 type arguments provided
c3.foo(1, {}, 2);

class X <T1, T2> {
    tag(value1: T1, value2: T2): string {
        return `X ${typeof value1}, ${typeof value2}`;
    }
}
class Y<T> extends X<number, T> { // class Y extends X instantiated with number and T
    public f1: X<Object, T> // X instantiated with Object and T
    public f2: X<T, string> // X instantiated with T and string
    constructor() {
        this.f1 = new X<Object, T>
        this.f2 = new X<T, string>
    }
}
let varY = new Y<string>;
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test1', varY.f1.tag({}, ''));
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test1', varY.f2.tag('', ''));

class InOutX<in T1, out T2, T3> {
    public readonly fldT2: T2
    public fldT3: T3

    foo(p: T1) {
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('InOutX', p);
    }

    constructor(x: T2) { this.fldT2 = x }
    bar1() : T2 { return this.fldT2 }
    method(p: T3): T3 { this.fldT3 = p; return p}
}
let varInOut = new InOutX<number, string, string>('test');
varInOut.foo(123);
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('InOutX', varInOut.bar1(), varInOut.method('asd'));
