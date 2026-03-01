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

{%- for c in cases %}

/*---
desc: >-
    Overload resolution for an instance method overload (see Class Method Overload Declarations) always uses the type
    of the object reference known at compile time. It can be either the type used in a declaration, or a smart type
---*/

class A {
    a: string = "a"
    foo1(x: A): string { return this.a }
    overload foo {foo1}
}
class B extends A {
    a: string = "b"
    foo2(x: B): string { return this.a }
    overload foo {foo2, foo1}
}

class C extends B {
    a: string = "c"
    foo2(x: C): string { return this.a }
    overload foo {foo2, foo1}
}

function testCompileTime(a: A): string {
    return a.foo(a)
}

function testSmartType(a: A): string {
    if (a instanceof C) {
        return a.foo(a)
    }
    if (a instanceof B) {
        return a.foo(a)
    }
    if (a instanceof A) {
        return a.foo(a)
    } else {
        return ""
    }
}

function main(): void {
    arktest.assertEQ({{c.call}}, {{c.expected}})
}
{%- endfor %}
