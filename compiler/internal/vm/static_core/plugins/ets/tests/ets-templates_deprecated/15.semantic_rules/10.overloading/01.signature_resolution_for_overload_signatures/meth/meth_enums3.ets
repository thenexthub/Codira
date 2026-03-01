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

enum RGB: long { Red = 3_000_000_000_000, Green, Blue }
enum YMC: long { Yellow = 4_000_000_000_000, Magenta, Cyan }

class X {
    foo(p: long, x1: int): string
    foo(p: RGB, x1: int, x2: int): string
    foo(p: YMC, x1: int, x2: int, x3: int): string
    foo(p: RGB|YMC, x1: int, x2: int, x3: int, x4: int): string
    foo(p: Object, x1: int, x2: int, x3: int, x4: int, x5: int): string
    foo(...p: Any[]): Any {
        return "" + p[0]
    }
}

let i: long = 5_000_000_000_000
let r: RGB = RGB.Blue
let y: YMC = YMC.Magenta
let x = new X

function main() {
    {{fd.call|indent}}
}
{% endfor %}
