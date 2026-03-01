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
desc: |-
    Only accessible (see Accessible) methods are subjected to override.
    The same rule applies to accessors in case of overriding.
    An overriding member can keep or extend the access modifier (see Access Modifiers)
    of a member. Otherwise, a compile-time error occurs.

    A compile-time error occurs if an attempt is made to do the following:
        • Override a private method of a superclass;

    Protected member can be overridden by the protected or public one.

tags: [compile-only, negative]
---*/

let res: number = 0;

class Base {
    public public_foo() { res = 1; }
    protected protected_foo(i: Number) { res = 2; }
    private private_foo() { res = 4; }
}

interface Interface {
    public_foo() { res = 5; }
}

class Gen<T> {
    protected foo(i: Gen<T>) {}
}

class Gen2<T,S> {
    protected foo(i: T) {}
}

class Derived extends Base {
private override protected_foo/* @@ label */(i: Number) { res = 11; }
}

function main() {
    res = 0
}

/* @@@ label Error TypeError: protected_foo(i: Double): void in Derived cannot override
protected_foo(i: Double): void in Base because overridden method has weaker access privilege. */
/* @@@ label Error Method protected_foo(i: Double): void in Derived not overriding any method */
