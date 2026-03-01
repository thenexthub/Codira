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
desc: Access methods with union return types normally in complex inheritance relationships.
---*/

interface G {
    makeSound(): string | number {
        return "interface!";
    }
}

abstract class Animal {
    abstract makeSound(): string | number;
}

class Dog extends Animal {
    makeSound(): string | number {
        return "Woof!";
    }
}

class Cat extends Animal implements G {
    makeSound(): string | number {
        return 123.0;
    }
}

function getSound(animal: Animal): string | number {
    return animal.makeSound();
}

function main(): void {
    const dog = new Dog();
    const cat = new Cat();
    arktest.assertEQ(getSound(dog), "Woof!")
    arktest.assertEQ(getSound(cat), 123.0);
}