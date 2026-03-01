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

class X {
    res: string = ""

    constructor(p: byte, x1: int)
    constructor(p: short, x1: int, x2: int)
    constructor(p: int, x1: int, x2: int, x3: int)
    constructor(p: long, x1: int, x2: int, x3: int, x4: int)
    constructor(p: float, x1: int, x2: int, x3: int, x4: int, x5: int)
    constructor(p: double, x1: int, x2: int, x3: int, x4: int, x5: int, x6: int)
    constructor(...p: Any[]) {
        this.res = "" + p[0]
    }
}

let vb: byte = 127
let vs: short = 32767
let vi: int = 42
let vl: long = 1_000_000
let vf: float = 1.5f
let vd: double = 2.5

function main() {
    {{fd.call|indent}}
}
{% endfor %}
