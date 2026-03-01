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
desc: Generic classes combined with inheritance.
---*/

class A<T> {
    value: T;

    constructor(value: T) {
        this.value = value;
    }

    getValue(): T {
        return this.value;
    }
}

interface G<T> {
    value: T;
    greet(): T {
        return this.value;
    }
}

class B extends A<string> implements G<string> {

    value: string = "";
    constructor(value: string) {
        super(value);
        this.value = value;
    }

    greet(): string {
        return `Hello, ${this.value}`;
    }
}

function main(): void {
    const b: A<string> | G<string> = new B("World");
    arktest.assertEQ((b as A<string>).getValue(), "World");
    arktest.assertEQ((b as G<string>).greet(), "Hello, World");
}
