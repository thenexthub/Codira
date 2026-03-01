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
    There are tricky cases related to overloading when a smart type may
    lead to the call of the function or method which suits the smart type
    of an argument rather than the static one.
tags: []
---*/

interface I {
    meth_i(p: int): string
}
interface K extends I {
    meth_k(p: int): string
}
class A implements I {
    meth_i(p: int): string {
        return 'I' + p
    }
    meth_a(p: int): string {
        return 'A' + p
    }
}
class C extends A {
    meth_c(p: int): string {
        return 'C' + p
    }
}
class D extends A implements K {
    meth_k(p: int): string {
        return 'K' + p
    }
    meth_d(p: int): string {
        return 'D' + p
    }
}

function foo_c(p: C): string {
    return 'C'
}
function foo_d(p: D): string {
    return 'D'
}
function foo_a(p: A): string {
    return 'A'
}
function foo_k(p: K): string {
    return 'K'
}
function foo_i(p: I): string {
    return 'I'
}

overload foo { foo_c, foo_d, foo_a, foo_k, foo_i }

function main(): void {
    {{c.code|indent}}
}
{%- endfor %}
