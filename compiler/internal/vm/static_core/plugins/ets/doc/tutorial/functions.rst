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

Functions
=========

|

Function Declarations
----------------------

A function declaration introduces a named function by specifying its name,
parameters, return type, and body.

A simple function with two string parameters and string return type is
represented in the example below:

.. code-block:: typescript

    function add(x: string, y: string): string {
        let z : string = `${x} ${y}`
        return z
    }

Type annotation of each parameter must be specified. When calling a function,
the corresponding argument of an optional parameter can be omitted. The last
parameter of a function can be a rest parameter (see below).

|

Optional Parameters
-------------------

An optional parameter has the form '``name?: Type``':

.. code-block:: typescript

    function hello(name?: string) {
        if (name == undefined) {
            console.log("Hello!")
        } else {
            console.log(`Hello, ${name}!`)
        }
    }

Another form contains an expression that specifies a default value. If the
corresponding argument to such parameter is omitted in a function call,
then the parameter has the default value:

.. code-block:: typescript

    function multiply(n: number, coeff: number = 2): number {
        return n * coeff
    }
    multiply(2)    // returns 2*2
    multiply(2, 3) // returns 2*3

|

Rest Parameter
--------------

The last parameter of a function can be a rest parameter. As a result,
functions or methods can take an unlimited number of arguments:

.. code-block:: typescript

    function sum(...numbers: number[]): number {
        let res = 0
        for (let n of numbers)
            res += n
        return res
    }

    sum() // returns 0
    sum(1, 2, 3) // returns 6

|

Return Types
------------

If function return type can be inferred from the content of the function body,
then the type can be omitted from the function declaration:

.. code-block:: typescript

    // Explicit return type
    function foo(): string { return "foo" }

    // Implicit return type inferred as string
    function goo() { return "goo" }

If a function requires no return statement, and function return type does not
need to return a value, then the type can be explicitly specified as ``void``
or omitted altogether.

Both notations are valid in the example below:

.. code-block:: typescript

    function hi1() { console.log("hi") }
    function hi2(): void { console.log("hi") }

|

Function Scope
--------------

Variables and other entities defined in a function are local to the function,
and cannot be accessed from the outside.

If the name of a variable defined in the function is equal to the name of an
entity in the outer scope, then the local definition shadows the outer entity.

|

Function Calls
--------------

Calling a function actually leads to the execution of its body, while
arguments of the call are assigned to function parameters.

If the function is defined as follows:

.. code-block:: typescript

    function join(x :string, y :string) :string {
        let z: string = `${x} ${y}`
        return z
    }

---then it is called with two arguments of type ``string``:

.. code-block:: typescript

    let x = join("hello", "world")
    console.log(x)

|

Function Types
--------------

Function types are commonly used as follows to define callbacks:

.. code-block:: typescript

    type trigFunc = (x: number) => number // this is a function type

    function do_action(f: trigFunc) {
         f(3.141592653589) // call the function
    }

    do_action(Math.sin) // pass the function as the parameter

|

Arrow Functions or Lambdas
---------------------------

A function can be defined as an arrow function as in the following
example:

.. code-block:: typescript

    let sum = (x: number, y: number): number => {
        return x + y
    }

If an arrow function return type is omitted, then it is inferred
from the function body.

An expression can be specified as an arrow function to shorten a notation.
For example, the following two notations are semantically equivalent:

.. code-block:: typescript

    let sum1 = (x: number, y: number) => { return x + y }
    let sum2 = (x: number, y: number) => x + y

|

Closure
-------

An arrow function is usually defined inside another function. As an inner
function, it can access all variables defined in the outer function.

To capture the context, an inner function forms a closure of its environment.
The closure allows accessing the inner function from the outside of its own
environment:

.. code-block:: typescript

    function f(): () => number {
        let count = 0
        return (): number => { count++; return count }
    }

    let z = f()
    console.log(z()) // output: 1
    console.log(z()) // output: 2

In the example above, the arrow function closure captures the ``count`` variable.

|

Function Overload Signatures
----------------------------

Overload signatures can be written to specify that a function can be called
in different ways. Writing an overload signature means that several headers
of the function have the same name but different signatures, and are immediately
followed by a single implementation function:

.. code-block:: typescript

    function foo(): void;            /* 1st signature */
    function foo(x: string): void;   /* 2nd signature */
    function foo(x?: string): void { /* implementation signature */
        console.log(x)
    }

    foo()     // ok, 1st signature is used
    foo("aa") // ok, 2nd signature is used

An error occurs if two overload signatures have identical parameter lists.

|

