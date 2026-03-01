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

class A {
    name: string
    age: int
    constructor(name: string, age: int)
    {
        this.name = name
        this.age = age
    }
}

class B {
    buf: ArrayBuffer
    constructor(b: ArrayBuffer) {
        this.buf = b
    }
}

function main() {
    // Basic values
    console.log("Simple string message");
    console.log(42);
    console.log(true);
    console.log(undefined);
    console.log(null);
    console.log()

    // Multiple arguments
    console.log("User:", "John", "Age:", 30);

    // Object
    console.log(new A("John", 30));

    // Template strings
    const name = "Alice";
    console.log(`Hello, ${name}!`);

    // Styled console output
    console.log(
        "%c Styled text",
        "color: blue;"
    );

    // Error objects
    console.log(new Error("Test error"));

    console.log(null, undefined, 11, "123", new Object())
    let arr = new Array<string | null | undefined>(3)
    console.log(arr)

    // Formatted strings
    console.log("%d")
    console.log("%d", 1)
    console.log("%d", 1, 2)

    console.log("%s %s", "37", 37)
    console.log("%s %s", "19.5", 19.5)
    
    // ArrayBuffer tests
    const buf1 = new ArrayBuffer(3);
    const view1 = new Uint8Array(buf1);
    view1[0] = 1;
    view1[1] = 2;
    view1[2] = 3;

    const buf2 = new ArrayBuffer(2);
    const view2 = new Uint8Array(buf2);
    view2[0] = 4;
    view2[1] = 5;
    
    console.log(buf1); 
    console.log(new ArrayBuffer(0)); 
    console.log([new B(buf1), new B(buf2)]);
    console.log(new B(buf1), new B(buf2));

    const arrayBuffer = new ArrayBuffer(10);
    const arrayBufferValue = reflect.Value.of(arrayBuffer);
    const result = arrayBufferValue.toPrint(0).toString();
    console.log(result);
}