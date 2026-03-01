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

Generic Types and Functions
===========================

|

Generic types and functions allow creating the code capable to work over a
variety of types rather than a single type.

|

Generic Classes and Interfaces
------------------------------

A class and an interface can be defined as generics that add parameters to
type definition, like the type parameter ``Element`` in the following example:

.. code-block:: typescript

    class Stack<Element> {
        public pop(): Element {
            // ...
        }
        public push(e: Element) {
            // ...
        }
    }

To use type ``Stack``, type argument must be specified for each type parameter:

.. code-block:: typescript

    let s = new Stack<string>
    s.push("hello")

The compiler ensures type safety while working with generic types and functions:

.. code-block:: typescript

    let s = new Stack<string>
    s.push(55) /* That will be a compile-time error as 55 is not compatible
      with type string */

|

Generic Constraints
-------------------

Type parameters of generic types can have constraints. For example, the ``Key``
type parameter in the ``HashMap<Key, Value>`` container must have a hash
method (i.e., it must be hashable):

.. code-block:: typescript

    interface Hashable {
        hash(): number
    }
    class HasMap<Key extends Hashable, Value> {
        public set(k: Key, v: Value) { 
            let h = k.hash()
            // ... other code ...
        }
    }

In the above example, the ``Key`` type extends ``Hashable``, and all methods
of a ``Hashable`` interface can be called for keys.

|

Generic Functions
-----------------

A generic function can be used to create a more universal code. Consideration
can be given to a function that returns the last element of the array:

.. code-block:: typescript

    function last(x: number[]): number {
        return x[x.length -1]
    }
    console.log(last([1, 2, 3])) // output: 3

If the same function needs to be defined for any array, then it can be
defined as a generic with a type parameter:

.. code-block:: typescript

    function last<T>(x: T[]): T {
        return x[x.length - 1]
    }

The function so defined can be used with any array.

In a function call, type argument can be set explicitly or implicitly:

.. code-block:: typescript

    // Explicit type argument
    console.log(last<string>(["aa", "bb"]))
    console.log(last<number>([1, 2, 3]))

    // Implicit type argument:
    // Compiler understands the type argument based on the type of the call arguments
    console.log(last([1, 2, 3]))

|

Generic Defaults
----------------

Type parameters of generic types can have defaults.
Defaults allow simply using a generic type name rather than specifying actual
type arguments. It is illustrated for both classes and functions in the example
below:

.. code-block:: typescript

    class SomeType {}
    interface Interface <T1 = SomeType> { }
    class Base <T2 = SomeType> { }
    class Derived1 extends Base implements Interface { }
    // Derived1 is semantically equivalent to Derived2
    class Derived2 extends Base<SomeType> implements Interface<SomeType> { }

    function foo<T = number>(): T {
        // ...
    }
    foo()
    // such function is semantically equivalent to the call below
    foo<number>()

|


