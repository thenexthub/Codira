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

The Basics
==========

|

Declarations
------------

Declarations in |LANG| introduce the following:

-  Variables,
-  Constants,
-  Functions, and
-  Types.

|

.. _Variable Declaration:

Variable Declaration
~~~~~~~~~~~~~~~~~~~~
A declaration starting with the keyword ``let`` introduces a variable that
can have different values during the program execution:

.. code-block:: typescript

    let hi: string = "hello"
    hi = "hello, world"

|

.. _Constant Declaration:

Constant Declaration
~~~~~~~~~~~~~~~~~~~~

A declaration starting with the keyword ``const`` introduces a read-only
constant that can be assigned only once:

.. code-block:: typescript

    const hello: string = "hello"


A compile-time error occurs if a new value is assigned to a constant.

|

.. _Automatic Type Inference:

Automatic Type Inference
~~~~~~~~~~~~~~~~~~~~~~~~

As |LANG| is a statically-typed language, the types of all entities, like
variables and constants, have to be known at compile time.

However, developers do not need to specify the type of a declared
entity explicitly if a variable or a constant declaration contains an
initial value. All cases that allow inferring type automatically are specified
in the |LANG| Specification.

Both variable declarations shown below are valid, and both variables are of the
type ``string``:

.. code-block:: typescript

    let hi1: string = "hello"
    let hi2 = "hello, world"

|

.. _Types:

Types
-----

``Class``, ``interface``, ``function``, ``enum``, ``union`` types, and type
``aliases`` are discussed in the corresponding sections.

|

.. _Numeric Types:

Numeric Types
~~~~~~~~~~~~~

|LANG| has numeric types ``number`` and ``Number``. Any integer and
floating-point value can be assigned to a variable of these types.

Numeric literals include integer literals and floating-point literals
with the decimal radix.

Integer literals include the following:

* Decimal integers that consist of a sequence of digits. For example:
  ``0``, ``117``, ``-345``;
* Hexadecimal integers that start with 0x (or 0X), and can contain digits
  (0-9) and letters (a-f or A-F). For example: ``0x1123``, ``0x00111``,
  ``-0xF1A7``;
* Octal integers that start with 0o (or 0O), and can only contain digits
  (0-7). For example: ``0o777``;
* Binary integers that start with 0b (or 0B), and can only contain
  digits 0 and 1. For example: ``0b11``, ``0b0011``, ``-0b11``.

A floating-point literal includes the following:

* Decimal integer, optionally signed (i.e., prefixed with '``+``' or '``-``');
* Decimal point ('``.``');
* Fractional part (represented by a string of decimal digits);
* Exponent part that starts with '``e``' or '``E``', followed by an optionally
  signed (i.e., prefixed with '``+``' or '``-``') integer.

For example:

.. code-block:: typescript

    let n1 = 3.14
    let n2 = 3.141592
    let n3 = .5
    let n4 = 1e10

    function factorial(n: number) : number {
        if (n <= 1) {
            return 1
        }
        return n * factorial(n - 1)
    }

|

.. _Boolean:

``boolean`` Type
~~~~~~~~~~~~~~~~

The ``boolean`` type represents logical values that are either ``true``
or ``false``.

The variables of this type are usually used in conditional statements:

.. code-block:: typescript

    let isDone: boolean = false

    // ...

    if (isDone) {
        console.log ("Done!")
    }

|


.. _String:

``string`` Type
~~~~~~~~~~~~~~~

A ``string`` is a sequence of characters; some characters can be set by using
escape sequences.

A ``string`` literal consists of zero or more characters enclosed in single
quote (' \' ') or double quotes (' \" ').

A special form of ``string`` literals are template literals enclosed in
backticks (' \` '):

.. code-block:: typescript

    let s1 = "Hello, world!\n"
    let s2 = 'this is a string'
    let a = 'Success'
    let s3 = `The result is ${a}`

|

.. _Void Type:

``void`` Type
~~~~~~~~~~~~~

Type ``void`` is used to specify that a function returns no value. Type ``void``
as a reference type can be used as type argument for generic types:

.. code-block:: typescript

    class Class<T> {
        //...
    }
    let instance: Class <void>

|

.. _Object Type:

``Object`` Type
~~~~~~~~~~~~~~~

An ``Object`` class type is a base type for all other classes, interfaces,
string, arrays, unions, and function types. Any value, including the
automatically boxed values of primitive types and enum types, can be directly
assigned to variables of type ``Object``.

|

.. _Array Type:

``array`` Type
~~~~~~~~~~~~~~

An ``array`` is an object comprised of data type elements assignable to
the element type specified in the array declaration.
The value of an ``array`` is set by using *array composite literal* that is
a list of zero or more expressions enclosed in square brackets ('``[ ]``').
Each such expression represents an element of the ``array``.
The length of the ``array`` is set by the number of expressions.
The index of the first array element is 0.

The following example creates the ``array`` with three elements:

.. code-block:: typescript

    let names: string[] = ["Alice", "Bob", "Carol"]

|

.. _Enum Type:

``enum`` Type
~~~~~~~~~~~~~

Type ``enum`` is a value type with a defined set of named values called
*enum constants*.
In order to be used, an ``enum`` constant must be prefixed with an ``enum``
type name:

.. code-block:: typescript

    enum Color { Red, Green, Blue }
    let c: Color = Color.Red

A constant expression can be used to explicitly set the value of an ``enum``
constant:

.. code-block:: typescript

    enum Color { White = 0xFF, Grey = 0x7F, Black = 0x00 }
    let c: Color = Color.Black

|

.. _Union Type:

``union`` Type
~~~~~~~~~~~~~~

Type ``union`` is a reference type created as a combination of other types.
Valid values of all types the union is created from can be the values of a
``union`` type:

.. code-block:: typescript

    class Cat {
      // ...
    }
    class Dog {
      // ...
    }
    class Frog {
      // ...
    }
    type Animal = Cat | Dog | Frog | number
    // Cat, Dog, and Frog are some types (class or interface ones)

    let animal: Animal = new Cat()
    animal = new Frog() 
    animal = 42
    // One may assign the variable of the union type with any valid value

Different mechanisms can be used to get a value of a particular type from a
union. For example:

.. code-block:: typescript

    class Cat { sleep () {}; meow () {} }
    class Dog { sleep () {}; bark () {} }
    class Frog { sleep () {}; leap () {} }

    type Animal = Cat | Dog | Frog | number

    let animal: Animal = new Cat()
    if (animal instanceof Frog) {
        let frog: Frog = animal as Frog // animal is of type Frog here
        animal.leap()
        frog.leap()
        // As a result frog leaps twice
    }

    animal.sleep () // Any animal can sleep

|

.. _Type Aliases:

Type Aliases
~~~~~~~~~~~~

Type *aliases* provide names for anonymous types (array, function, object
literal, or union types), or alternative names for the existing types:

.. code-block:: typescript

    type Matrix = number[][]
    type Handler = (s: string, no: number) => string
    type Predicate <T> = (x: T) => Boolean
    type NullableObject = Object | null

|

.. _Operators:

Operators
---------

|

.. _Assignment Operators:

Assignment Operators
~~~~~~~~~~~~~~~~~~~~

Simple assignment operator '``=``' is used as in '``x = y``'.

Compound assignment operators combine an assignment with an operator, where
'``x op = y``' equals '``x = x op y``'.

Compound assignment operators are as follows: '``+=``', '``-=``', '``*=``',
'``/=``', '``%=``', '``<<=``', '``>>=``', '``>>>=``', '``&=``', '``|=``',
and '``^=``'.

|

.. _Comparison Operators:

Comparison Operators
~~~~~~~~~~~~~~~~~~~~

.. table::

    +--------------+-----------------------------------------------------------------------------+
    | Operator     | Description                                                                 |
    +==============+=============================================================================+
    | ``==``       |   returns true if both operands are equal                                   |
    +--------------+-----------------------------------------------------------------------------+
    | ``!=``       |   returns true if both operands are not equal                               |
    +--------------+-----------------------------------------------------------------------------+
    | ``>``        |   returns true if the left operand is greater than the right                |
    +--------------+-----------------------------------------------------------------------------+
    | ``>=``       |   returns true if the left operand is greater than or equal to the right    |
    +--------------+-----------------------------------------------------------------------------+
    | ``<``        |   returns true if the left operand is less than the right                   |
    +--------------+-----------------------------------------------------------------------------+
    | ``<=``       |   returns true if the left operand is less than or equal to the right       |
    +--------------+-----------------------------------------------------------------------------+

|

.. _Arithmetic Operators:

Arithmetic Operators
~~~~~~~~~~~~~~~~~~~~

Unary operators are '``-``', '``+``', '``--``', and '``++``'.

Binary operators are as follows:

.. table::

    +--------------+-------------------------------------+
    | Operator     | Description                         |
    +==============+=====================================+
    | ``+``        |   addition                          |
    +--------------+-------------------------------------+
    | ``-``        |   subtraction                       |
    +--------------+-------------------------------------+
    | ``*``        |   multiplication                    |
    +--------------+-------------------------------------+
    | ``/``        |   division                          |
    +--------------+-------------------------------------+
    | ``%``        |   remainder after division          |
    +--------------+-------------------------------------+


|

.. _Bitwise Operators:

Bitwise Operators
~~~~~~~~~~~~~~~~~

.. csv-table::
   :header: "Operator", "Description"
   :widths: 5, 30

   "``a & b``", "Bitwise AND: sets each bit to 1 if the corresponding bits of both operands are 1, otherwise to 0."
   "``a | b``", "Bitwise OR: sets each bit to 1 if at least one of the corresponding bits of both operands is 1, otherwise to 0."
   "``a ^ b``", "Bitwise XOR: sets each bit to 1 if the corresponding bits of both operands are different, otherwise to 0."
   "``~ a``", "Bitwise NOT: inverts the bits of the operand."
   "``a << b``", "Shift left: shifts the binary representation of *a* to the left by *b* bits."
   "``a >> b``", "Arithmetic right shift: shifts the binary representation of *a* to the right by *b* bits with sign-extension."
   "``a >>> b``", "Logical right shift: shifts the binary representation of *a* to the right by *b* bits with zero-extension."

|

.. _Logical Operators:

Logical Operators
~~~~~~~~~~~~~~~~~

.. table::

    +--------------+---------------------+
    | Operator     | Description         |
    +==============+=====================+
    | ``a && b``   |   logical AND       |
    +--------------+---------------------+
    | ``a || b``   |   logical OR        |
    +--------------+---------------------+
    | ``! a``      |   logical NOT       |
    +--------------+---------------------+

|

.. _Statements:

Statements
----------

|

.. _If Statements:

``if`` Statements
~~~~~~~~~~~~~~~~~

An ``if`` statement is used to execute a sequence of statements when the logical
condition is ``true``. Another set of statements (if provided) is used otherwise.
The ``else`` part can also contain more ``if`` statements.

An ``if`` statement is as follows:

.. code-block:: typescript

    if (condition1) {
        // statements1
    } else if (condition2) {
        // statements2
    } else {
        // else_statements
    }

All conditional expressions must be of type ``boolean`` or other types
(``string``, ``number``, etc.). For types other than ``boolean``, implicit
conversion rules apply as follows:

.. code-block:: typescript

    let s1 = "Hello"
    if (s1) {
        console.log(s1) // prints "Hello"
    }

    let s2 = "World"
    if (s2.length != 0) {
        console.log(s2) // prints "World"
    }

|

.. _Switch Statements:

``switch`` Statements
~~~~~~~~~~~~~~~~~~~~~

A ``switch`` statement is used to execute a sequence of statements that match
the value of a switch expression.

A ``switch`` statement looks as follows:

.. code-block:: typescript

    switch (expression) {
    case label1: // will be executed if label1 is matched
        // ...
        // statements1
        // ...
        break; // Can be omitted
    case label2:
    case label3: // will be executed if label2 or label3 is matched
        // ...
        // statements23
        // ...
        break; // Can be omitted
    default:
        // default_statements
    }

A ``switch`` expression must be of types ``number``, ``enum``, or ``string``.

Each label must be an expression of the same type as the ``switch`` expression.

If the value of a ``switch`` expression equals the value of a label, then
the corresponding statements are executed.

If there is no match, and a ``switch`` has a default clause, then
default statements are executed.

An optional ``break`` statement allows breaking out of a ``switch``, and
then executing the statement that follows the ``switch``.

If there is no ``break``, then the next statement in a ``switch`` is
executed.

|

.. _Conditional Expressions:

Conditional Expressions
~~~~~~~~~~~~~~~~~~~~~~~

The conditional expression '``? :``' uses the ``boolean`` value of the first
expression to decide which of two other expressions to evaluate.

A conditional expression is as follows:

.. code-block:: typescript

    condition ? expression1 : expression2

The condition must be a logical expression. If the logical expression is
``true``, then the first expression is used as the result of the ternary
expression. Otherwise, the second expression is used. For example:

.. code-block:: typescript

    let isValid = Math.random() > 0.5 ? true : false
    let message = isValid ? 'Valid' : 'Failed'

|

.. _For Statements:

``for`` Statements
~~~~~~~~~~~~~~~~~~

A ``for`` statement is executed repeatedly until the specified loop exit
condition is ``false``.

A ``for`` statement looks as follows:

.. code-block:: typescript

    for ([init]; [condition]; [update]) {
        statements
    }

When a ``for`` statement is executed, the following process takes place:


#. An ``init`` expression is executed, if any. This expression usually
   initializes one or more loop counters.

#. The condition is evaluated. If the value of condition is ``true``, or
   the conditional expression is omitted, then the statements in the ``for``
   body are to be executed. If the value of condition is ``false``, then
   the ``for`` loop terminates.

#. The statements of the ``for`` body are executed.

#. An ``update`` expression is executed, if any.

#. Go back to step 2.


The process is illustrated in the example below:

.. code-block:: typescript

    let sum = 0
    for (let i = 0; i < 10; i += 2) {
        sum += i
    }

|

.. _For-of Statements:

``for-of`` Statements
~~~~~~~~~~~~~~~~~~~~~

A ``for-of`` statement is used to iterate over an array or a string. A
``for-of`` statement looks as follows:

.. code-block:: typescript

    for (forVar of expression) {
        statements 
    }

Another example is below:

.. code-block:: typescript

    for (let ch of "a string object") { /* process ch */ }

|

.. _While Statements:

``while`` Statements
~~~~~~~~~~~~~~~~~~~~

A ``while`` statement has its body statements executed as long as the
specified condition evaluates to ``true``. A ``while`` statement is as
follows:

.. code-block:: typescript

    while (condition) {
        statements
    }

The condition must be a logical expression:

.. code-block:: typescript

    let n = 0
    let x = 0
    while (n < 3) {
        n++
        x += n
    }

|

.. _Do-while Statements:

``do-while`` Statements
~~~~~~~~~~~~~~~~~~~~~~~

A ``do-while`` statement is executed repetitively until a specified
condition evaluates to false. A ``do-while`` statement looks as follows:

.. code-block:: typescript

    do {
        statements
    } while (condition)

The condition must be a logical expression:

.. code-block:: typescript

    let i = 0
    do {
        i += 1
    } while (i < 10)

|

.. _Break Statements:

``break`` Statements
~~~~~~~~~~~~~~~~~~~~

A ``break`` statement is used to terminate any ``loop`` or ``switch`` statement:

.. code-block:: typescript

    let x = 0
    while (true) {
        x++;
        if (x > 5) {
            break;
        }
    }

A ``break`` statement with a label identifier transfers control out of the
enclosing statement to the one that has the same label identifier:

.. code-block:: typescript

    let x = 1
    label: while (true) {
        switch (x) {
        case 1: 
            // statements
            break label // breaks the while
        }
    }

|

.. _Continue Statements:

``continue`` Statements
~~~~~~~~~~~~~~~~~~~~~~~

A ``continue`` statement stops the execution of the current loop iteration
and passes control to the next iteration:

.. code-block:: typescript

    let sum = 0
    for (let x = 0; x < 100; x++) {
        if (x % 2 == 0) {
            continue
        }
        sum += x
    }

|

.. _Throw and Try Statements:

``throw`` and ``try`` Statements
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A ``throw`` statement is used to throw an exception or an error:

.. code-block:: typescript

    throw new Error("this error")

A ``try`` statement is used to catch and handle an exception or an error:

.. code-block:: typescript

    try {
        // try block
    } catch (e) {
        // handle the situation
    }

The example below shows the ``throw`` and ``try`` statements used to handle
a zero-division case:

.. code-block:: typescript

    class ZeroDivisor extends Error {}

    function divide (a: number, b: number): number{
        if (b == 0) throw new ZeroDivisor()
        return a / b
    }

    function process (a: number, b: number) {
        try {
            let res = divide(a, b)
            console.log(res)
        } catch (x) { 
            console.log("some error")
        }
    }

The ``finally`` clause is also supported:

.. code-block:: typescript

    function processData(s: string) {
        let error : Error | null = null

        try {
            console.log("Data processed: ", s)
            // ...
            // Throwing operations
            // ...
        } catch (e) {
            error = e as Error
            // ...
            // More error handling
            // ...
        } finally {
            if (error != null) {
                console.log(`Error caught: input='${s}', message='${error.message}'`)
            }
        }
    }

|
