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

class A {
    fldA: int

    constructor(a: int) {
        this.fldA = a
    }
}

class B extends A {
    constructor(a: int) {
        try {
            if (a == 0) {
                throw new Error('Error thrown')
            }
            super(a)
        } catch (e) {
            super(-1)
        }
    }
}

function main() {
    let b: B

    arktest.expectError(() => { b = new B(0) }, 'Error thrown')
    arktest.assertEQ(b.fldA, -1)

    b = new B(1)
    arktest.assertEQ(b.fldA, 1)
}
