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
desc: Verify the access to constants under complex inheritance.
---*/

interface G {
    value: number;

    getValue(): number {
        return this.value;
    }

    greet(): string {
        return "Hello G";
    }
}

interface F extends G {

}

interface E extends F {
    greet(): string {
        return "Hello E";
    }
}

class A implements E {
    static readonly COMMON_VALUE: number = 100;
    value: number;

    constructor(value: number) {
        this.value = value;
    }

    getValue(): number {
        return this.value;
    }

    greet(): string {
        return "Hello";
    }
}

class B extends A {
    greet(): string {
        return `Hello, ${this.value}`;
    }

    constructor(value: number) {
        super(value);
    }
}

class C extends A {
    constructor(value: number) {
        super(value);
    }

    greet(): string {
        return `Hello, ${this.value}`;
    }

    getValue(): number {
        return this.value + 200;
    }
}

function main(): void {
    const c = new C(200);
    arktest.assertEQ(C.COMMON_VALUE, 100);
    arktest.assertEQ(c.value, 200);
    arktest.assertEQ(c.getValue(), 400);
    arktest.assertEQ(c.greet(), "Hello, 200")
    
    const b = new B(300);
    arktest.assertEQ(B.COMMON_VALUE, 100);
    arktest.assertEQ(b.value, 300);
    arktest.assertEQ(b.getValue(), 300);
    arktest.assertEQ(b.greet(), "Hello, 300")
}