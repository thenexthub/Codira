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
    There could be cases when the type of an entity (mostly local variables)
    is a union type and in the particular context of the program the compiler
    can narrow (smart cast) the static type to a more precise type (smart type)
    and allow operations which are specific to such narrowed type.
---*/

function boo(): number | string {
    let a: number | string = 42
    a++
    return a
}

class Base {}
class Derived extends Base {
    method(): int { return 1 }
}

function goo(): int {
    let b: Base = new Derived()
    return b.method()
}

function foo(b: Base, d: Derived): int {
    if (b instanceof Derived) {
        return b.method()
    }
    if (d != null) {
        return d.method()
    }
    return 0
}

function main() {
    arktest.assertEQ(boo(), 43)
    arktest.assertEQ(goo(), 1)
    arktest.assertEQ(foo(new Derived(), new Derived()), 1)
    arktest.assertEQ(foo(new Base(), new Derived()), 1)
}
