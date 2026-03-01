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
    The result of an instanceof expression is true
    if the actual type of evaluated expr is a subtype of T (see Subtyping).
    Otherwise, the result is false.
---*/

interface I {}

interface J {}

class A implements I {}

class B extends A implements J {}

let a: A
let b: B
let i: I
let j: J

function main() {
    a = new A()
    arktest.assertTrue(a instanceof A)
    arktest.assertTrue(a instanceof I)
    arktest.assertFalse(a instanceof B)
    arktest.assertFalse(a instanceof J)

    a = new B()
    arktest.assertTrue(a instanceof A)
    arktest.assertTrue(a instanceof I)
    arktest.assertTrue(a instanceof B)
    arktest.assertTrue(a instanceof J)

    i = new A()
    arktest.assertTrue(i instanceof A)
    arktest.assertTrue(i instanceof I)
    arktest.assertFalse(i instanceof B)
    arktest.assertFalse(i instanceof J)

    i = new B()
    arktest.assertTrue(i instanceof A)
    arktest.assertTrue(i instanceof I)
    arktest.assertTrue(i instanceof B)
    arktest.assertTrue(i instanceof J)

    b = new B()
    arktest.assertTrue(b instanceof A)
    arktest.assertTrue(b instanceof I)
    arktest.assertTrue(b instanceof B)
    arktest.assertTrue(b instanceof J)

    j = new B()
    arktest.assertTrue(j instanceof A)
    arktest.assertTrue(j instanceof I)
    arktest.assertTrue(j instanceof B)
    arktest.assertTrue(j instanceof J)
}
