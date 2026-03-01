..
    Copyright (c) 2021-2024 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

Null Safety
===========

|

All types in |LANG| are by default non-nullable, and the value of a type
cannot be null. It is similar to the |TS| behavior in strict null checking
mode (``strictNullChecks``), but the |LANG| rules are stricter.

In the example below, all lines cause a compile-time error:

.. code-block:: typescript

    let x: number = null    // Compile-time error
    let y: string = null    // ditto
    let z: number[] = null  // ditto

A variable with the null value is defined by a union type ``T | null``:

.. code-block:: typescript

    let x: number | null = null
    x = 1    // ok
    x = null // ok
    if (x != null) { /* do something */ }

|

Non-Null Assertion Operator
-----------------------------

The postfix operator '``!``' can be used to assert that its operand is non-null.
If applied to the null value, the operator throws an error. Otherwise, the
type of the value is changed from ``T | null`` to ``T``:

.. code-block:: typescript

    let x: number | null = 1
    let y: number
    y = x + 1  // compile time error: cannot add to a nullable value
    y = x! + 1 // ok

|

Null-Coalescing Operator
------------------------

The null-coalescing binary operator '``??``' checks whether the evaluation
of the left-hand-side expression equals to null. If so, then the result of
the expression is the right-hand-side expression. Otherwise, it is the
left-hand-side expression.

In other words, '``a ?? b``' equals the ternary operator '``a != null ? a : b``'.

In the following example, the method ``getNick`` returns a nickname if it is
set. Otherwise, an empty string is returned:

.. code-block:: typescript

    class Person {
        // ...
        nick: string | null = null
        getNick(): string {
            return this.nick ?? "" 
        }
    }

|

Optional Chaining
-----------------

The optional chaining operator '``?.``' allows writing code where the evaluation
stops at an expression that partially evaluates to ``null`` or ``undefined``:

.. code-block:: typescript

    class Person {
        nick    : string | null = null
        spouse ?: Person

        setSpouse(spouse: Person) : void {
            this.spouse = spouse
        }

        getSpouseNick(): string | null | undefined {
            return this.spouse?.nick
        }

        constructor(nick: string) {
            this.nick = nick
            this.spouse = undefined
        }
    }

**Note**: The return type of ``getSpouseNick`` must be
``string | null | undefined`` as the method can return ``null`` or
``undefined``.

An optional chain can be of any length and can contain any number of '``?.``'
operators.

In the example below, if a person has a spouse, and the spouse has a
nickname, then the output is the nickname of the person's spouse. Otherwise,
the output is ``undefined``:

.. code-block:: typescript

    class Person {
        nick    : string | null = null
        spouse ?: Person

        constructor(nick: string) {
            this.nick = nick
            this.spouse = undefined
        }
    }

    let p: Person = new Person("Alice")
    console.log(p.spouse?.nick) // print: undefined

|
