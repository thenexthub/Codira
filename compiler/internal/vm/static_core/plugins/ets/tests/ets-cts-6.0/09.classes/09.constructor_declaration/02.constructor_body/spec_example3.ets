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

class Base {
    x: Object
    constructor() {
        this.x = new Object() // Base object is fully initialized
        crash_this(this)
    }
}

class Derived extends Base {
    y: Object
    constructor() {
        super() // mandatory call to base class constructor
        this.y = new Object()
    }
}

function crash_this(b: Base) {
    if (b instanceof Derived) {
        // If b is of type Derived, then
        arktest.assertTrue((b as Derived).y.toString() instanceof string) // Access y field of Derived object
        // Depending on the compilation context, either the compiler reports
        // a compile-time error, or the runtime system is to detect the case
    }
}

function main() {
    let b: Base = new Base()
    arktest.assertTrue(b.x.toString() instanceof string)

    arktest.expectThrow(
        () => { b = new Derived() },
        (e) => e instanceof NullPointerError
    )
}
