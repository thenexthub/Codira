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

// override
class Animal {
    getName() {
        return 'Animal';
    }
}
class Dog extends Animal {
    override getName() {
        return 'Dog';
    }
}
const dog = new Dog();
console.log(dog.getName());

// # private Class member
class C {
    #value: number = 0;
    #add() {                               // private method
      this.#value += 1;
    }

    static #message: string = 'hello';     // private static property
    static #say() {
      return this.#message;
    }

    get #data() {                          // private accessor
      return this.#value;
    }
    set #data(num: number) {               // private accessor
      this.#value = num;
    }

    static get #msg() {                    // private static accessor
      return this.#message;
    }

    static set #msg(msg: string) {         // private static accessor
      this.#message = msg;
    }

    publicMethod() {
        C.#say();
        this.#data = 1
        C.#msg = 'Hello';
        this.#add();
        console.log(this.#data, this.#value, C.#msg);
    }
}
let c: C = new C();
c.publicMethod();

// Static index signature
class Foo {
    hello = 'hello';
    world = 1234;

    [propName: string]: string | number | undefined;
}
let instance = new Foo();
instance['whatever'] = 42;
let x = instance['something'];

// Symbol and template string index signature
interface Colors {
    [sym: symbol]: number;
}

const red = Symbol('red');
const green = Symbol('green');
const blue = Symbol('blue');
 
let colors: Colors = {};
colors[red] = 255;
let redVal = colors[red];
console.log(redVal);

interface OptionsWithDataProps {
    [optName: `option-${string}`]: string;
}
let idx: OptionsWithDataProps = {
    'option-1': 'aaaa',
    'option-2': 'bbbb',
};
console.log(idx['option-1']);


// Static Block in class
class StaticFoo {
    static prop = 1
    static {
        console.log(StaticFoo.prop++);
    }
    static {
        console.log(StaticFoo.prop++);
    }
    static {
        console.log(StaticFoo.prop++);
    }
}
console.log(StaticFoo.prop);


// Instantiate expression
function makeBox<T>(value: T) {
    return { value };
}
const makeStringBox = makeBox<string>;
console.log(makeStringBox('111'));

// Satisfies operator
type S = {
    a: string,
    b: string,
}
const s = {
    a: 'aaa',
    b: 'bbb',
    c: 'ccc'
}
const isSatisfies = s satisfies S;

// Class auto accessor
class D {
    accessor name: string;
    constructor(name:string) {
        this.name = name;
    }
}

