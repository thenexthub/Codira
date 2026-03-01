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

---*/

let x: string

class Aliases {
    static foo(): string {
        return x
    }
}

const aRef = Aliases.foo

function main() {
    x = 'One'
    arktest.assertEQ('One', aRef())
    x = 'Two'
    arktest.assertEQ('Two', aRef())
}
