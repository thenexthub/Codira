/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
desc: >-
    Generic type in the form of type name can be used as T operand of an instanceof expression.
    In this case, the check is performed against the type name, and type parameters are ignored.
---*/

interface I<T> {}

interface J<T> {}

class A<T> implements I<T> {}

class B<T> extends A<T> implements J<T> {}

class C<T> {}

class D<T> extends C<T> {}

let a: A<int>
let b: B<int>
let i: I<int>
let j: J<int>
let c: C<int>
let d: D<int>

function main() {
    a = new A<int>()
    arktest.assertTrue(a instanceof A)
    arktest.assertTrue(a instanceof I)
    arktest.assertFalse(a instanceof B)
    arktest.assertFalse(a instanceof J)

    a = new B<int>()
    arktest.assertTrue(a instanceof A)
    arktest.assertTrue(a instanceof I)
    arktest.assertTrue(a instanceof B)
    arktest.assertTrue(a instanceof J)

    i = new A<int>()
    arktest.assertTrue(i instanceof A)
    arktest.assertTrue(i instanceof I)
    arktest.assertFalse(i instanceof B)
    arktest.assertFalse(i instanceof J)

    i = new B<int>()
    arktest.assertTrue(i instanceof A)
    arktest.assertTrue(i instanceof I)
    arktest.assertTrue(i instanceof B)
    arktest.assertTrue(i instanceof J)

    b = new B<int>()
    arktest.assertTrue(b instanceof A)
    arktest.assertTrue(b instanceof I)
    arktest.assertTrue(b instanceof B)
    arktest.assertTrue(b instanceof J)

    j = new B<int>()
    arktest.assertTrue(j instanceof A)
    arktest.assertTrue(j instanceof I)
    arktest.assertTrue(j instanceof B)
    arktest.assertTrue(j instanceof J)

    c = new C<int>()
    d = new D<int>()
    arktest.assertTrue(c instanceof C)
    arktest.assertTrue(d instanceof D)
    arktest.assertTrue(d instanceof C)
    arktest.assertFalse(c instanceof D)
}
