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

type FA = (p: string, q: number) => Object // Base type
type FB = (p: string) => Object // Derived, less parameters
type FC = () => Object // Derived, no parameters
type FD = (p: string, q: number) => string // Derived, return type is subtype
type FE = (p: string, q: Object) => Object // Derived, parameter is supertype

class X {
    res: Any

    constructor(p: FA)
    constructor(p: FB, x1: int)
    constructor(p: FC, x1: int, x2: int)
    constructor(p: FD, x1: int, x2: int, x3: int)
    constructor(p: FE, x1: int, x2: int, x3: int, x4: int)
    constructor(p: string)
    constructor(p: number)
    constructor(...p: Any[]) {
        this.res = p[0]
    }
}

let fa: FA = (p: string, q: number): Object => p + q
let fb: FB = (p: string): Object => "FB" + p
let fc: FC = (): Object => "FC"
let fd: FD = (p: string, n: number): string => "FD" + p + n
let fe: FE = (p: string, q: Object): Object => "FE" + p + q

function main() {
    {{fd.call|indent}}
}
{% endfor %}
