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
---*/

abstract class A {
    public p1: int|string|boolean = 0;
    public q1: number|number[]|undefined;
    public w1: number[]|undefined;

    protected constructor(p: int)
    protected constructor(p: int|string, q?: number, w?: number[])
    protected constructor(p: boolean, q?: number[])
    protected constructor(...p: Any[]) {
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

class B extends A {
    constructor fromInt(p: int) {
        super(p)
    }

    constructor fromString(p: string) {
        super(p)
    }

    constructor fromArray(p: int, q?: number, w?: number[]) {
        super(p, q, w)
    }

    overload constructor { fromInt, fromString, fromArray }
}

function main() {
    let v = new B(42)
    arktest.assertEQ(v.p1, 42)
    arktest.assertEQ(v.q1, undefined)
    arktest.assertEQ(v.w1, undefined)

    let x = new B("hello")
    arktest.assertEQ(x.p1, "hello")
    arktest.assertEQ(x.q1, undefined)
    arktest.assertEQ(x.w1, undefined)

    let y = new B(33, 3.14, [1.0, 2.0, 3.0])
    arktest.assertEQ(y.p1, 33)
    arktest.assertEQ(y.q1, 3.14)
    let arr = y.w1 as number[]
    arktest.assertEQ(arr[0], 1.0)
    arktest.assertEQ(arr[1], 2.0)
    arktest.assertEQ(arr[2], 3.0)
}
