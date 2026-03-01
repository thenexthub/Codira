..
    Copyright (c) 2021-2025 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

Simple Calls and Primitive Types
################################

Functions Without Arguments That Return `void`
**********************************************

.. code-block:: typescript
    :linenos:

    // File: lib.ts
    export function greet(): void {
        console.log("Hello!")
    }

.. code-block:: typescript
    :linenos:

    // File: lib.d.ets
    declare export function greet(): void

.. code-block:: typescript
    :linenos:

    // File: app.ets
    import { greet } from "lib"

    greet() // prints: Hello

Passing Numbers, Booleans or Strings to Functions
*************************************************

.. code-block:: typescript
    :linenos:

    // File: lib.ts
    export function greetUser(name: string): void {
        console.log("Hello, `${name}`!")
    }

.. code-block:: typescript
    :linenos:

    // File: lib.d.ets
    declare export function greetUser(name: string): void

.. code-block:: typescript
   :linenos:

    // File: app.ets
    import { greetUser } from "lib"

    greetUser("John") // prints: Hello, John!

    let name = "Jane"
    greetUser(name) // prints: Hello, Jane!

Functions Without Arguments That Return Number, Boolean or String
*****************************************************************

.. code-block:: typescript
   :linenos:

    // File: lib.ts
    export function tryGreetUser(name: string): boolean {
        if (name.length === 0) {
            return false
        }
        console.log("Hello, `${name}`!")
        return true
    }

.. code-block:: typescript
    :linenos:

    // File: lib.d.ets
    declare export function tryGreetUser(name: string): boolean

.. code-block:: typescript
   :linenos:

    // File: app.ets
    import { tryGreetUser } from "lib"

    let greetedBob = tryGreetUser("Bob") // prints: Hello, Bob!
    if (!greetedBob) {
        console.log("Oops, could not greet Bob")
    }

    let greetedNameless = tryGreetUser("") // prints nothing
    if (!greetedNameless) {
        console.log("Oops, could not greet a nameless user")
    }
