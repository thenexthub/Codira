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

    constructor(a: string) {} // primary

    constructor() {  // secondary
        this('')
    }
}

function main() {
    cnt = 0
    let a = new A() // call secondary constructor
    arktest.assertEQ(a.f00, 0)
    arktest.assertEQ(a.f01, 1)
    arktest.assertEQ(a.f02, 2)
    arktest.assertEQ(a.f03, 3)
    arktest.assertEQ(a.f04, 4)
}
