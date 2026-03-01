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
desc: Ensure that a field is initialized once
---*/

let res = 0

function foo() {
    return ++res
}

class A {
    fld: int = foo()
}

class B extends A {
    constructor(a: boolean) { // primary
        super()
    }

    constructor() {  // secondary
        this(true)
    }

    constructor(a: number) { // secondary
        this()
    }
}

function main() {
    res = 0
    let b = new B(true)  // primary
    arktest.assertEQ(b.fld, 1)

    res = 0
    b = new B()  // secondary
    arktest.assertEQ(b.fld, 1)

    res = 0
    b = new B(1.0)  // secondary
    arktest.assertEQ(b.fld, 1)
}
