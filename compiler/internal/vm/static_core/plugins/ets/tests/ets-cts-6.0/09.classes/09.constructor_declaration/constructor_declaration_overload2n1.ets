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
    A constructor is used in the creation of an object that is an instance of a class.
tags: [compile-only, negative]
---*/

class A {
    public p1: int|string|boolean = 0;
    public q1: number|number[]|undefined;
    public w1: number[]|undefined;

    constructor(p: int)
    constructor(p: int|string, q?: number, w?: number[])
    constructor(p: boolean, q?: number[])
    constructor(...p: Any[]) {
        if (p.length > 0) {
            this.p1 = p[0]
        }
        if (p.length > 1) {
            this.q1 = p[1]
        }
        if (p.length > 2) {
            this.w1 = p[2]
        }
    }
}

final class B extends A {
    constructor(p: int)
    constructor(p: int|string, q?: number, w?: number[])
    constructor(p: boolean, q?: number[])
    constructor()
    constructor(...p: Any[]) {
        switch(p.length) {
            case 0:
                super(false);
                break;
            case 1:
                super(p[0]);
                break;
            case 2:
                super(p[0], p[1]);
                break;
            case 3:
                super(p[0], p[1], p[2]);
                break;
            default:
                throw new Error('Wrong number of arguments')
        }
    }
}

function main() {
    let v = new B("hello", "world")  // CTE: No such constructor
}
