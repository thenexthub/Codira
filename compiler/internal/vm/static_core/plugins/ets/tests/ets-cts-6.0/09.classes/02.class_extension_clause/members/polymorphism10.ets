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

/*---
desc: Testing access to member variables in the case of polymorphism.
      Under testing polymorphism, subclasses override parent class's property accessors.
---*/

class Animal {
    private _age: number;
    constructor(age: number) {
        this._age = age;
    }
    get age(): number {
        return this._age;
    }
    set age(value: number) {
        this._age = value;
    }
}

class Dog extends Animal {
    constructor(age: number) {
        super(age);
    }

    get age(): number {
        return super.age * 7;
    }
}

function main(): void {
    const dog = new Dog(5);
    arktest.assertEQ(dog.age, 35);

    const animal: Animal = new Dog(5);
    arktest.assertEQ((animal as Dog).age, 35);
}