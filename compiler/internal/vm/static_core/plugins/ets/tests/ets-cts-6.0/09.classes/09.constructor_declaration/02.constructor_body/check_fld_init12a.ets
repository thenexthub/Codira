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
desc: Ensure that all fields are initialized in order they appear in a class body
---*/

let cnt = 0

function foo(): int {
    return cnt++
}
class A {
    f00 = foo()
    f01 = foo()
    f02 = foo()
    f03 = foo()
    f04 = foo()
}

class B extends A {
    f00 = foo()  // re-defined fields
    f01 = foo()
    f02 = foo()
    f03 = foo()
    f04 = foo()

    constructor(a: string) { // primary
        super()
    }

    constructor() {  // secondary
        this('')
    }
}

function main() {
    cnt = 0
    let b = new B('x') // call primary constructor
    arktest.assertEQ(b.f00, 0)
    arktest.assertEQ(b.f01, 1)
    arktest.assertEQ(b.f02, 2)
    arktest.assertEQ(b.f03, 3)
    arktest.assertEQ(b.f04, 4)
}
