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
    If some interface properties are of an optional type,
    then such properties can be skipped in an object literal,
    their values are set to undefined
---*/

interface Person {
    name: string
    age: number
    sex?: "male"|"female"
}

function main(): void {
    let b: Person = {name: "Bob", age: 25}
    arktest.assertEQ(b.sex, undefined)
}
