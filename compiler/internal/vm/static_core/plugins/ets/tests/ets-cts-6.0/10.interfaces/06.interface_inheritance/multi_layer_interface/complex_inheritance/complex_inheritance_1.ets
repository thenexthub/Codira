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
desc: Verify inheritance of multi-layer interfaces and classes to ensure correct access to constants and methods.
---*/

interface A {
    a: number;
    getA(): number;
}

abstract class B implements A {
    a: number = 1;
    abstract getB(): number;
    getA(): number {
        return this.a;
    }
}

class C extends B {
    getB(): number {
        return 2;
    }
}

function main(): void {
    const c = new C();
    arktest.assertEQ(c.getA(), 1);
    arktest.assertEQ(c.getB(), 2);
}