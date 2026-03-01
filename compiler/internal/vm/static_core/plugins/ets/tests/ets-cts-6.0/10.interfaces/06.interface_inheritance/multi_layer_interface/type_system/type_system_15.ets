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
desc: Verify the correctness of the type system through complex generic interfaces and class implementations.
---*/

interface A<T, U> {
    a: T;
    b: U;
}

class B implements A<string, Boolean> {
    a: string = "B";
    b: Boolean = true;
}

class C extends B {
    a: string = "C";
    b: Boolean = false;
}

function main(): void {
    const b: A<string, Boolean> = new B();
     arktest.assertEQ(b.a, "B");
     arktest.assertEQ(b.b, true);

    const c: B = new C();
    arktest.assertEQ(c.a, "C");
    arktest.assertEQ(c.b, false);
}