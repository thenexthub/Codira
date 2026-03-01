..
    Copyright (c) 2024 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

Features
========

This section provides examples for each feature. Pieces of code cause
ETS-Warnings as the compilation proceeds. The warning messages are included
as comments to the examples.

If the performance requires improvement, then a rewritten version of |LANG| code
appears with specific notes to each line of code that was fixed.


Feature #1: Show Implicit Boxing and Unboxing Conversions
---------------------------------------------------------

|CB_RULE| ``ets-implicit-boxing-unboxing``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Rationale
~~~~~~~~~

|LANG| supports Boxing and Unboxing conversions, though they have performance
limitations. If the design of the application allows avoiding frequent
Boxed/Unboxed type conversions, then the developer is to use the appropriate
type. The performance gain can as much as double in some applications.


|LANG| Performance Challenges
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    // Application contains many arithmetic operations wrapped in a Reference Type.
    // Using Value type is preferable here.
    function complexArithmeticExpression(a: int, b: float, c: long): double {
        if (b == 0 || c == 0) {
            console.log('Division by zero is not allowed');
        }

        const addition = a + b;
        const subtraction = a - c;
        const multiplication = b * c;
        const division = a / (b + c);

        const result = (addition * subtraction) / (multiplication + division) + (Math.pow(a, 2) - Math.sqrt(b)) * Math.sin(c);

        return result;
    }

    function main() {
        let num1: Int = 15; // Warning: Implicit Boxing to Char in Variable Declaration.
        let num2: Float = 5.0; // Warning: Implicit Boxing to Float in Variable Declaration.
        let num3: Long = 5; // Warning: Implicit Boxing to Long in Variable Declaration.
        let result: double = complexArithmeticExpression(num1, num2, num3); // Warning: Implicit Unboxing to float in Call Method/Function.
        console.log(result);
    }

|LANG| is More Efficient
~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    function complexArithmeticExpression(a: int, b: float, c: long): double {
        if (b == 0 || c == 0) {
            console.log('Division by zero is not allowed');
        }

        const addition = a + b;
        const subtraction = a - c;
        const multiplication = b * c;
        const division = a / (b + c);

        const result = (addition * subtraction) / (multiplication + division) + (Math.pow(a, 2) - Math.sqrt(b)) * Math.sin(c);

        return result;
    }

    function main() {
        let num1: int = 15;
        let num2: float = 5.0;
        let num3: long = 5;
        let result: double = complexArithmeticExpression(num1, num2, num3);
        console.log(result);
    }

Feature #2: Boost Equality Expressions
---------------------------------------

|CB_RULE| ``ets-boost-equality-expression``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Rationale
~~~~~~~~~

|LANG| supports reference binary operators. If the left part of an expression
is a nullable type, and the right part is a reference type, then swapping the
sides of the expression causes it work faster, while the result
of the expression in both cases is the same. The change can result in an
approximately 10 % performance improvement.

|LANG| Performance Challenges
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    let x: Int = new Int(5);

    if (x == null) { // Warning: Boost Equality Expression. Change sides of binary expression.
        console.println("Hello!");
    }

    let k: boolean = x == null; // Warning: Boost Equality Expression. Change sides of binary expression.

|LANG| Improvement Implemented
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    let x: Int = new Int(5);

    if (null == x) { // Fixed ETS-Warning
        console.println("Hello!");
    }

    let k: boolean = null == x; // Fixed version - sides of binary expression changed.


Feature #3: Using Coroutines Instead of Async-functions
-----------------------------------------------------------

|CB_RULE| ``ets-remove-async``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Rationale
~~~~~~~~~

|LANG| supports Async-functions and Coroutines. Async-function type is only
supported for the backward |TS| compatibility, and "System |LANG|" suggests
using Coroutines instead.

|LANG| Async Way
~~~~~~~~~~~~~~~~

.. code-block:: typescript

    async function foo(): Promise <int> {
        return 1;
    }

    function main(): void {
        let promise = foo(); // Warning: Replace asynchronous function with coroutine.
    }

|LANG| Coroutines Way
~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    async function foo(): Promise <int> {
        return 1;
    }

    function main(): void {
        let promise = foo(); // Changed to coroutine way - begin
        let i = await promise; // Changed to coroutine way - end
    }

Feature #4: Suggest Final Modifier for Classes and Methods
----------------------------------------------------------

|CB_RULE| ``ets-suggest-final``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Rationale
~~~~~~~~~

By default, all classes in |LANG| can be extended, and all methods can be
overriden. As a consequence, calling a method requires runtime resolution.
If a class or a method is not intended to be used further for inheritance,
then providing the modifier ``final`` is recommended. The usage of ``final``
allows making calls more efficient, and improves the performance significantly.

|LANG| Performance Challenges
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    class A {
        foo(): String {
            return "foo";
        };
    }

    class K extends A { // Warning: Suggest 'final' modifier for class 'K'
        foo_to_suggest(): void {}; // Warning: Suggest 'final' modifier for method 'foo_to_suggest'.
        override foo(): String { // Warning: Suggest 'final' modifier for method 'foo'.
            return "overridden_foo";
        }
    }

|LANG| Improvement Implemented
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    class A {
        foo(): String {
            return "foo";
        };
    }

    final class K extends A {
        final foo_to_suggest(): void {};
        final override foo(): String {
            return "overridden_foo";
        }
    }

Feature #5: Using Function Call Instead of Lambda
-------------------------------------------------

|CB_RULE| ``ets-remove-lambda``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Rationale
~~~~~~~~~

|LANG| supports lambda calls. However, some applications with function calls
are four times as fast as those with lambda. "System |LANG|" suggests using
function calls.

|LANG| Performance Challenges
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    let foo: (i: int) => int
    foo = (i: int): int => {return i + 1} // Warning: Replace the lambda function with a regular function.

|LANG| Improvement Implemented
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    function foo(i: int) : int {
        return i + 1
    }
