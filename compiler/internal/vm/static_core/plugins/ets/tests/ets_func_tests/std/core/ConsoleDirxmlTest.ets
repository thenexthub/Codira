/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
    console.dirxml("Simple string message");
    console.dirxml(42);
    console.dirxml(true);
    console.dirxml(undefined);
    console.dirxml(null);
    console.dirxml()

    // Multiple arguments
    console.dirxml("User:", "John", "Age:", 30);

    // Objects
    console.dirxml(new A("John", 30));

    // Template strings
    const name = "Alice";
    console.dirxml(`Hello, ${name}!`);

    // Styled console output
    console.dirxml(
        "%c Styled text",
        "color: blue;"
    );

    // Error objects
    console.dirxml(new Error("Test error"));

    console.dirxml(null, undefined, 11, "123", new Object())
    let arr = new Array<string | null | undefined>(3)
    console.dirxml(arr)

    // Formatted strings
    console.dirxml("%d")
    console.dirxml("%d", 1)
    console.dirxml("%d", 1, 2)

    console.dirxml("%s %s", "37", 37)
    console.dirxml("%s %s", "19.5", 19.5)
}