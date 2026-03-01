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

class TestObject {
    private _name: string = "Leechy";
    private _age: number = 24;
    private _原產地: string = "中國";

    constructor() {}

    get name(): string {
        return this._name;
    }

    set name(a: string) {
        this._name = a;
    }

    get age(): number {
        return this._age;
    }
}

class Student {
    private _name: string;

    constructor(name: string) {
        this._name = name;
    }

    greet(msg: string): string {
        return `${msg}, I am ${this._name}`;
    }
}

class Person {
    name: string;

    constructor(name: string) {
        this.name = name;
    }
}

class Animal {}

class Dog extends Animal {}

(globalThis as any).tsDogInstance = new Dog();
(globalThis as any).tsAnimalClass = Animal;

(globalThis as any).tsCtor = Person;
(globalThis as any).tsAnyMethodObject = new Student("Leechy");

(globalThis as any).tsAnyObject = new TestObject();
(globalThis as any).tsAnyArray = ["dev", "ts", "ani"];
(globalThis as any).tsAnySetObject = new TestObject();
(globalThis as any).tsAnySetArray = ["dev", "ts", "ani"];

(globalThis as any).tsAnyFunc = (name: string): string => {
    return "Hello, " + name;
};

function main(): void {
    const etsVm = globalThis.gtest.etsVm;

    ASSERT_TRUE(etsVm.getFunction("Lets_any/ETSGLOBAL;", "etsCheckAnyGetPropertyFromTS")());
    ASSERT_TRUE(etsVm.getFunction("Lets_any/ETSGLOBAL;", "etsCheckAnySetPropertyToTS")());
    ASSERT_TRUE(etsVm.getFunction("Lets_any/ETSGLOBAL;", "etsCheckAnyCallMethodFromTS")());
    ASSERT_TRUE(etsVm.getFunction("Lets_any/ETSGLOBAL;", "etsCheckAnyNewFromTS")());
    ASSERT_TRUE(etsVm.getFunction("Lets_any/ETSGLOBAL;", "etsCheckAnyInstanceOfFromTS")());
}

main();