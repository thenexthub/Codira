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
desc: Verify that the GermanShepherd instance is of type Guard.
---*/

interface Animal {
    name: string;
    age: number;
    makeSound(): string;
}

interface Dog extends Animal {
    breed: string;
    bark(): string;
}

interface Guard {
    guard(): string;
}

class GermanShepherd implements Dog, Guard {
    name: string = '';
    age: number = 0;
    breed: string = '';

    constructor(name: string, age: number, breed: string) {
        this.name = name;
        this.age = age;
        this.breed = breed;
    }

    makeSound(): string {
        return 'makeSound';
    }

    bark(): string {
         return 'bark';
    }

    guard(): string {
         return 'guard';
    }
}

function main(): void {
    const dog: Dog = new GermanShepherd("Rex", 3, "German Shepherd");
    const guard: Guard = dog;
    arktest.assertEQ(guard.guard(), 'guard');
}