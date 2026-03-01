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
    A method is defined in a subinterface with the same name as the method
    in the superinterface.
    If signatures are override-compatible, then overriding is used.
    Otherwise, a compile-time error occurs.
tags: [{{c.tags}}]
---*/

class A {}
class B extends A {}

interface I {
    meth(p: B): Object
    meth(p: number): Object
}
interface J extends I {
    // override-compatible signatures
    meth(p: B): Object
    meth(p: number|boolean): Object
    meth(p: string): Object
}
class X implements J {
    // X implements all methods from J
    meth(...p: Any[]): Any {
        return p[0]
    }
}

let a = new A
let b = new B
let i: I = new X
let j: J = new X
let x: X = new X

function main() {
    {{c.use|indent}}
}
{%- endfor %}
