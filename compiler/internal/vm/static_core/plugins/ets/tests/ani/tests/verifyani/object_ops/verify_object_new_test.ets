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

class CheckCtorTypes {
    constructor(z: boolean, c: char, b: byte, s: short, i: int, l: long, f: float, d: double, r: string) {
        this.z = z
        this.c = c
        this.b = b
        this.s = s
        this.i = i
        this.l = l
        this.f = f
        this.d = d
        this.r = r
    }

    constructor(z: boolean, r1: string, s: short, r2: string, l: long, f: float, d: double, r3: string) {
        this.z = z
        this.c = c'a'
        this.b = 1
        this.s = s
        this.i = 1
        this.l = l
        this.f = f
        this.d = d
        this.r = r1
    }

    private z: boolean
    private c: char
    private b: byte
    private s: short
    private i: int
    private l: long
    private f: float
    private d: double
    private r: string

    public voidMethod(): void {
        return;
    }

    public static staticMethod(): void {
        return;
    }
}

class A {
    constructor() {}
}

class B extends A {}

class C {
    constructor(v: A) {
    }
    constructor(v: B) {
    }
}

class CheckUnionType {
    constructor(a: A | C | string) {}
}
