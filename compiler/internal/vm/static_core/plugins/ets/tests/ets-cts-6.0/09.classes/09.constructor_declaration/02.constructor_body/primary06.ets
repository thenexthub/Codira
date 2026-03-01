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

let res = ''

class A {
    x: Object
    constructor() {
        this.x = 'a'
        this.assert()
    }

    assert() {
        arktest.assertTrue(false, 'this method should not be called')
    }
}

class B extends A {
    y: Object
    constructor() {
        super()
        this.y = 'xyz'
    }

    assert() {
        arktest.expectThrow(() => { res = this.y.toString() })
    }
}

function main() {
    let b: B = { y: 'b' }
    arktest.assertEQ(b.x, 'a')
    arktest.assertEQ(b.y, 'b')
    arktest.assertEQ(res, '')
}
