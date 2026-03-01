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

{% for item in btypes %}
/*---
desc: {function: ArrayBackedConstructorWith{{.item.type}}}
name: issue14700.ets
tags: []

---*/


export class TestBuffer {
}

class TestArrayBuffer extends TestBuffer {
    constructor(size:  number) {
    }
}

export final class ArrayBackedConstructor {
    public constructor(buff: Array<{{.item.type}}>) {
        this();
    }

    public static readonly BYTES_PER_ELEMENT: int = 1;

    public constructor() {
        this(new TestArrayBuffer(0), 0, 0);
    }

    public constructor(buf: TestBuffer, offset: number, length: number) {
    }
}


function main(): void {
    let xx: ArrayBackedConstructor  = new ArrayBackedConstructor(new Array<{{.item.type}}>());
    arktest.assertNE(xx, null);
}

{%- endfor %}
