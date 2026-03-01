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
    Mandatory call to a superconstructor if a class has an extension clause
    on all execution paths of the constructor body.
---*/

enum Color { Red, Green, Blue }

abstract class A {
    fldA: string

    constructor(a: string) {
        this.fldA = a
    }
}

final class B extends A {
    constructor(a: string) {  // primary
        super(a)
    }

    constructor(a: Color) {  // secondary
        this('valid color = ' + a.getName())
    }

    constructor(a: int) {  // secondary
        try {
            this(Color.fromValue(a))
        } catch (e) {
            this('invalid color')
        }
    }
}

function main() {
    let b = new B(0)
    arktest.assertEQ(b.fldA, 'valid color = Red')

    b = new B(1)
    arktest.assertEQ(b.fldA, 'valid color = Green')

    b = new B(2)
    arktest.assertEQ(b.fldA, 'valid color = Blue')

    b = new B(3)
    arktest.assertEQ(b.fldA, 'invalid color')
}
