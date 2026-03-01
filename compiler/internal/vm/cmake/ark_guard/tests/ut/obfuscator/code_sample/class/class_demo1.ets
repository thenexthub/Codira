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

class ClassA1 {
    constructor(name: string) {
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('name string :' + name);
    }
    constructor(name: number, id: number) {
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('name number :' + name);
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('id number :' + id);
    }

    static ClassA() {
        return 3;
    }

    add() {
        return 1;
    }
}

let obj = new ClassA1('test');
let obj2 = new ClassA1(23, 123);
// CC-OFFNXT(G.AOD.03) for test purposes
console.log(obj.add());
// CC-OFFNXT(G.AOD.03) for test purposes
console.log(ClassA1.ClassA());

// export class
export class ClassA2 {
    public static sFieldA0 = 0;
    public fieldA1 = 1;

    methodA1() {
        return ClassA2.sFieldA0 + this.value1;
    }

    get value1(): number {
        return this.fieldA1;
    }
}

let a = new ClassA2();

// CC-OFFNXT(G.AOD.03) for test purposes
console.log('methodA1 = ' + a.methodA1());

class C {
    foo(): this {
        return this
    }
}
class D extends C {
    foo(): this {
        return this
    }
}
let x = new C().foo() // type of 'x' is 'C'
let y = new D().foo() // type of 'y' is 'D'

// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test1', x instanceof C);
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test1', y instanceof D);

class ClassLong {
    public field1: long = 1000000;
}

class ClassFloat {
    public field1: float = 123;
}

export class BaseClass {
    public field1 = 1;
}
