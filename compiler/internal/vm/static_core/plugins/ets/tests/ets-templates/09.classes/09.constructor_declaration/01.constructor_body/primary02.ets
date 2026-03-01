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
    fldA: string

    constructor(a: string) {
        this.fldA = a
    }
}

class B extends A {
    constructor(a: string) {
        if (a == '') {
            res = -1
            throw new Error('empty')
        }
        if (a == '1') {
            super('1')
            res = 1
            return
        }
        super('2')
        res = 2
    }
}

let res: int

function main() {
    res = 0

    let b: B
    arktest.expectError(() => { b = new B('') }, 'empty')
    arktest.assertEQ(res, -1)

    b = new B('1')
    arktest.assertEQ(res, 1)
    arktest.assertEQ(b.fldA, '1')

    b = new B('2')
    arktest.assertEQ(res, 2)
    arktest.assertEQ(b.fldA, '2')
}
