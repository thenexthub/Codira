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
desc: >-
    A casting conversion from union converts an expression of union type
    to one of the types of the union, or to a type that is derived from such one type.
---*/
class Cat {
    sleep () {}
    meow () {}
}
class Dog {
    sleep () {}
    bark () {}
}
class Frog {
    sleep () {}
    leap () {}
}
class Spitz extends Dog {
    override sleep() {
        /* 18-20 hours a day */
    }
}

type Animal = Cat | Dog | Frog | number

function createAnimal(): Animal {
    return new Spitz()
}

function main(): int {
    let animal: Animal = createAnimal()

    if (animal instanceof Frog) {
        let frog: Frog = animal as Frog // Use 'as' conversion here
        frog.leap() // Perform an action specific for the particular union type
    }

    if (animal instanceof Spitz) {
        let dog = animal as Spitz // Use 'as' conversion here
        dog.sleep()
    }

    return 0;
}
