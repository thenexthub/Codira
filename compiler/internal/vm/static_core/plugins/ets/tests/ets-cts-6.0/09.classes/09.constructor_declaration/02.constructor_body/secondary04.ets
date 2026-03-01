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

    constructor fromInt(a: int) {  // secondary
        if (a == 0) {
            throw new Error('zero')
        } else {
            if (a == 1) {
                this('1')
                return
            } else {
                if (a == 2) {
                    this('2')
                    return
                }
                this('3')
            }
        }
    }
}

function main() {
    let b: B
    arktest.expectError(() => { b = new B.fromInt(0) }, 'zero')

    b = new B.fromInt(1)
    arktest.assertEQ(b.fldA, '1')

    b = new B.fromInt(2)
    arktest.assertEQ(b.fldA, '2')

    b = new B.fromInt(-1)
    arktest.assertEQ(b.fldA, '3')
}
