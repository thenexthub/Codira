/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

/*---
desc: >-
    A parameterless constructor is executed to produce an instance a of class A. The execution of the object literal
    completes abruptly if so does the execution of the constructor.
---*/

class A {
    name: string|null = null
    age: number|null = null
    constructor() {
        let x = 0
        let y = 1
        let z = y/x
    }
}

function foo(): void {
    let a: A|null = null;
	a = { name: "Bob", age: 42 } as A
}

let checkException = (e: Error): boolean => { return e instanceof ArithmeticError }

function main() {
    arktest.expectThrow(() => foo(), checkException)
}