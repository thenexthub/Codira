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

function main() {
    // Basic values
    console.warn("Simple string message");
    console.warn(42);
    console.warn(true);
    console.warn(undefined);
    console.warn(null);
    console.warn()

    // Multiple arguments
    console.warn("User:", "John", "Age:", 30);

    // Objects
    console.warn(new A("John", 30));

    // Template strings
    const name = "Alice";
    console.warn(`Hello, ${name}!`);

    // Styled console output
    console.warn(
        "%c Styled text",
        "color: blue;"
    );

    // Error objects
    console.warn(new Error("Test error"));

    console.warn(null, undefined, 11, "123", new Object())
    let arr = new Array<string | null | undefined>(3)
    console.warn(arr)

    // Formatted strings
    console.warn("%d")
    console.warn("%d", 1)
    console.warn("%d", 1, 2)

    console.warn("%s %s", "37", 37)
    console.warn("%s %s", "19.5", 19.5)
}
