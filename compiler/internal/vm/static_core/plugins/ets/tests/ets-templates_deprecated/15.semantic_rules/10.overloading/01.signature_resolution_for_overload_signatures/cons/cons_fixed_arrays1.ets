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

{% for c in cases %}
/*---
desc: >-
    Call arguments are checked at the call site against such multiple signatures
    in their declaration order: the call is considered valid as soon as the first
    signature is found appropriate for the arguments given.
tags: [{{c.tags}}]
---*/

class A {}
class B extends A {}

class X {
    res: string = ""

    constructor(p: Object)
    constructor(p: FixedArray<A>, x1: int)
    constructor(p: FixedArray<B>, x1: int, x2: int)
    constructor(p: FixedArray<A|B>, x1: int, x2: int, x3: int)
    constructor(p: FixedArray<A>|FixedArray<B>, x1: int, x2: int, x3: int, x4: int)
    constructor(p: FixedArray<Object>, x1: int, x2: int, x3: int, x4: int, x5: int)
    constructor(...p: Any[]) {
        let arg = p[0]
        if (arg instanceof FixedArray) {
            if (arg[0] instanceof A) this.res += "A"
            if (arg[0] instanceof B) this.res += "B"
            if (arg[0] instanceof Object) this.res += "Object"
        }
    }
}

let a: FixedArray<A> = [new A]
let b: FixedArray<B> = [new B]
let o: FixedArray<Object> = [new Object]

function main() {
    {{c.call|indent}}
}
{% endfor %}
