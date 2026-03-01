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

class Base {}
class Derived extends Base {}
class SomeType {}


class G1<T extends Base> {
    foo(value: T) {
        let var1 = new Base();
        console.log('G1', `${typeof var1 === typeof value}`);
    }
}
let x1 = new G1<Base>()
x1.foo(new Base());
let y1 = new G1<Derived>()
y1.foo(new Derived());

class G2<T, S extends T> {
    foo(value: S) {
        let var1 = new Base();
        console.log('G2', `${typeof var1 === typeof value}`);
    }
}
let x2 = new G2<Base, Derived>()
x2.foo(new Derived());

class H<T extends Base|SomeType> {
    foo() {
        let var1 = new Base();
        console.log('H', `${var1 instanceof Base}`);
    }
}
let h1 = new H<Base>()
h1.foo();
let h2 = new H<Derived>()
h2.foo();
let h3 = new H<SomeType>()
h3.foo();

class Exotic<T extends "aa"| "bb"> {
    foo(value: T) {
        console.log('Exotic', typeof value);
    }
}
let e1 = new Exotic<"aa">()
e1.foo("aa");

class B1 {
    f1: number = 0
    f2: string = ""
    f3: boolean = false
}
class B2 <T extends keyof B1> {
    foo(value: T) {
        console.log('B2', typeof value);
    }
}
let b1 = new B2<'f1'>()
b1.foo('f1');
let b3 = new B2<keyof B1>()
b3.foo('f3');

class A0<T> {
    data: T
    constructor (p: T) { this.data = p }
    foo () {}
}
class A1<T extends Object> extends A0<T> {
    constructor (p: T) { super(p); this.data = p }
    override foo () {
        let o: Object = this.data
        console.log (this.data.toString())
    }
}
let varA1 = new A1<Base>(new Base());
varA1.foo();

interface Interface <T1 = SomeType> {
    foo(p: T1);
}
class Base2 <T2 = SomeType> {
    bar(p: T2): T2 {
        return p;
    }
}
class Derived1 extends Base2 implements Interface {
    foo(p: SomeType) {
        console.log('Derived1', p === this.bar(p));
    }
}
let varDerived1 = new Derived1();
varDerived1.foo(new SomeType());

