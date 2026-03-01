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
    The values of intersection type A & B are all values that belong to both A and B.
    Example from the section.
---*/

class C {
    foo() {
        return 30.
    }
}

interface I {
    bar(): number
}

class Derived extends C implements I {
    bar() {
        return 12.
    }
}

function test(i: I): number {
    if (i instanceof C) {
        // smart type of 'i' here is of some subtype of 'C' that implements 'I'
        // type expression for this type is I & subtype of C
        return i.foo() + i.bar()
    }
    return 0
}

function main() {
    arktest.assertEQ(test(new Derived()), 42.)
}
