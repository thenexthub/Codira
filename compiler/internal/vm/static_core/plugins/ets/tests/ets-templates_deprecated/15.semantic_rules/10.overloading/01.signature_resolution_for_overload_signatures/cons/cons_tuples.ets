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

{% for fd in cases %}
/*---
desc: >-
    Call arguments are checked at the call site against such multiple signatures
    in their declaration order: the call is considered valid as soon as the first
    signature is found appropriate for the arguments given.
tags: [{{fd.tags}}]
---*/

class A {}
class B extends A {}

class X {
    res: string = ""

    constructor(p: Object)
    constructor(p: [A, A], x1: int)
    constructor(p: [B, B], x1: int, x2: int)
    constructor(p: [A, A]|[B, B], x1: int, x2: int, x3: int)
    constructor(p: [A|B, A|B], x1: int, x2: int, x3: int, x4: int)
    constructor(...p: Any[]) {
        if (p[0] instanceof [B, B]) this.res += "B"
        if (p[0] instanceof [A, A]) this.res += "A"
    }
}

let ta: [A, A] = [new A, new A]
let tb: [B, B] = [new B, new B]

function main() {
    {{fd.call|indent}}
}
{% endfor %}
