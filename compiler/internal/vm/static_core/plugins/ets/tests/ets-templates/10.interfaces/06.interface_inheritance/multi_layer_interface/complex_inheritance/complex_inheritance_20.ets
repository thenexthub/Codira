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
desc: Verify the method call order and the correct pointing of super in multi-level inheritance.
---*/

interface G {
    greet(): string {
        return "Hello from interface G";
    }
}

interface F extends G {

}

class A implements F {
    greet() {
        return 'A';
    }
}

class B extends A {
    greet() {
        return `B:${super.greet()}`;
    }
}

class C extends B {
    greet() {
        return `C:${super.greet()}`;
    }
}

function main(): void {
    const c = new C();
    arktest.assertEQ(c.greet(),  "C:B:A");
}