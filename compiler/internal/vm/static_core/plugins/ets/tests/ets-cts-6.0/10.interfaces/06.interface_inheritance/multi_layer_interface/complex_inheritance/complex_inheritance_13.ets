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
desc: Testing access to constants in multi-level inheritance.
---*/

interface G {
    readonly CONSTANT: string;
}

class A {
    static readonly CONSTANT: string = "Constant from A";
}

class B extends A {
    static readonly CONSTANT: string = "Constant from B";
}

class C extends B implements G {
    readonly CONSTANT: string = "Constant from interface C";
}

function main(): void {
    arktest.assertEQ(C.CONSTANT, "Constant from B");

    const c: C = new C();
    arktest.assertEQ(c.CONSTANT, "Constant from interface C");
}