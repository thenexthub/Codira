/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
desc: If class type C is inferred from the context, then type of an object literal is C.
---*/

class Person {
    name: string = ""
    age: number = 0
}

function foo(p: Person) {
    arktest.assertEQ(p.age, 18)
}

function main(): void {
    let p: Person = {name: "Bob", age: 25}  // ok, variable type is used
    arktest.assertTrue(p instanceof Person)
    arktest.assertEQ(p.age, 25)
    foo({name: "Alice", age: 18}) // ok, parameter type is used
}
