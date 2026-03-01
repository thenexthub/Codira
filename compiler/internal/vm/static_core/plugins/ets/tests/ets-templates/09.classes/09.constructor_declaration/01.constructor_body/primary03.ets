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
        if (a == 1) {
            res = 1
            super(a)
        } else {
            if (a <= 2) {
                res = 2
                super(a * 2)
            } else {
                if (a == 3) {
                    res = 3
                    super(a * 3)
                } else {
                    res = 4
                    super(a * 4)
                }
            }
        }
    }
}

let res: int

function main() {
    res = 0

    let b = new B(1)
    arktest.assertEQ(res, 1)
    arktest.assertEQ(b.fldA, 1)

    b = new B(2)
    arktest.assertEQ(res, 2)
    arktest.assertEQ(b.fldA, 4)

    b = new B(3)
    arktest.assertEQ(res, 3)
    arktest.assertEQ(b.fldA, 9)

    b = new B(4)
    arktest.assertEQ(res, 4)
    arktest.assertEQ(b.fldA, 16)
}
