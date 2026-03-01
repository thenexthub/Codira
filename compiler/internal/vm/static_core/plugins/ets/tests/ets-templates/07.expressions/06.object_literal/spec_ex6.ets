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
    If a class or an interface has some method implementation,
    then its object literal can either skip providing a new method implementation,
    or provide its own version of method
---*/

class Base {
    method () { console.log ("method() from Base is called") }
}

interface I {
    method () { console.log ("method() from I is called") }
}

function main(): void {
    const x1: Base = {} // Valid literal of type Base
    x1.method()

    const x2: Base = { // Valid literal of type compatible to Base
        method () { console.log ("method() from object literal is called") }
    }
    x2.method ()

    const x3: I = {} // Valid literal of anonymous class type
    x3.method ()

    const x4: I = { // Valid literal of anonymous class type
        method () { console.log ("method() from object literal is called") }
    }
    x4.method ()

}