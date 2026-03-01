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
    A method reference refers to a static or instance method of a class or an interface.
    If method reference refers to an instance method, that the named reference is bounded
    with the used instance of that class or interface.
---*/



class Aliases {
    field: int
    constructor (val: int) {
        this.field = val
    }
    foo(): int { return this.field }
}

function main() {
    let a: Aliases = new Aliases(30);
    let ref = a.foo // In TS should be a.foo.bind(a)
    a = new Aliases(100)

    arktest.assertEQ(30, ref())
}
