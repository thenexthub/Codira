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

class Test {
    constructor(a: double, b: double) {
        this.a = a;
        this.b = b;
    }

    public Summ(): double {
        return this.a + this.b;
    }

    @ani.unsafe.Quick
    public static native quickMethod(s: string, l: long): string;

    @ani.unsafe.Direct
    public static native directMethod(b0: boolean, i0: byte, i1: char, i2: short, i3: int, i4: long, f0: float, f1: double): int;

    private a: double;
    private b: double;
};

function exampleFunction(a: double, b: double): double {
    let t = new Test(a, b);
    let res = t.Summ();
    return res;
}

native function nativeExampleFunction(a: long, b: long): long;
