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

.. _|LANG| |TS| compatibility:

|LANG|-|TS| compatibility
=========================

.. meta:
    frontend_status: None

This section discusses the |LANG| compatibility with |TS| and all issues related.

|

.. _Undefined is Not a Universal Value:

Undefined is Not a Universal Value
----------------------------------

.. meta:
    frontend_status: Done

Compile-time or runtime errors occur in |LANG| in many cases where runtime
value ``undefined`` is used in |TS|.

.. code-block-meta:
   expect-cte

.. code-block:: typescript
   :linenos:

    let array = new Array<number>
    let x = array [1234]
       // TypeScript: x will be assigned with value undefined!!!
       // ArkTS: compile-time error if analysis may detect array out of bounds
       //        violation or runtime error ArrayOutOfBounds
    console.log(x)

.. index::
   value
   runtime value
   runtime error

|

.. _Numeric Semantics:

Numeric Semantics
-----------------

.. meta:
    frontend_status: Done

|TS| has a single numeric type ``number`` that handles both integer and real
numbers.

|LANG| interprets ``number`` as a variety of types. Calculations depend
on the context and can produce different results as follows:

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    let n = 1
       // TypeScript: treats 'n' as having type number
       // ArkTS: treats 'n' as having type int to reach max code performance

    console.log(n / 2)
       // TypeScript: will print 0.5 - floating-point division is used
       // ArkTS: will print 0 - integer division is used

.. index::
   numeric type
   context
   semantics

|

.. _Covariant Overriding:

Covariant Overriding
--------------------

.. meta:
    frontend_status: Done

|TS| object runtime model enables |TS| handling situations where a
non-existing property is accessed from an object during program execution.

|LANG| allows generating highly efficient code that relies on the object
layout known at compile time. Covariant overriding violates type-safety and
is prohibited:

.. code-block:: typescript
   :linenos:

    class Base {
       foo (p: Base) {}
    }
    class Derived extends Base {
       override foo (p: Derived)
          // ArkTS will issue a compile-time error - incorrect overriding
       {
           console.log ("p.field unassigned = ", p.field)
              // TypeScript will print 'p.field unassigned =  undefined'
           p.field = 42 // Access the field
           console.log ("p.field assigned   = ", p.field)
              // TypeScript will print 'p.field assigned   =  42'
           p.method() // Call the method
              // TypeScript will generate runtime error: p.method is not a function
       }
       method () {}
       field: number = 0
    }

    let base: Base = new Derived
    base.foo (new Base)

.. index::
   covariant overriding
   runtime model
   object
   property
   access
   compile time
   method
   function
   type safety
   layout

|

.. _Function Types Compatibility:

Function Types Compatibility
----------------------------

.. meta:
    frontend_status: Done

|TS| allows rather relaxed assignment into variables of function type.
|LANG| uses the following stricter rules (see the |LANG| specification for
details):

.. code-block:: typescript
   :linenos:

    type FuncType = (p: string) => void
    let f1: FuncType = (p: string): number => { return 0 } // compile-time error in ArkTS
    let f1: FuncType = (p: string): string => { return "" } // compile-time error in ArkTS

.. index::
   function type
   compatibility
   assignment
   variable
   conversion

|

.. _Utility Type Compatibility:

Utility Type Compatibility
--------------------------

.. meta:
    frontend_status: Done

Utility type ``Partial<T>`` in |LANG| is not assignable to ``T``.
Variables of this type are to be initialized
with object literals only.

.. code-block:: typescript
   :linenos:

    function foo<T>(t: T, part_t: Partial<T>) {
        part_t = t // compile-time error in ArkTS
    }

.. index::
   compatibility
   utility type
   initialization
   object literal

|

.. _TS Overload Signatures:

|TS| Overload Signatures
------------------------

.. meta:
    frontend_status: Done

|LANG| supports no overload signatures like |TS| does. In |TS|, several function
headers can be followed by a single body. In |LANG|, each overloaded function,
method, or constructor must have a separate body. E.g., the following code is
valid in |TS| but causes a compile-time error in |LANG|:

.. code-block-meta:
   expect-cte

.. code-block:: typescript
   :linenos:

    function foo(): void 
    function foo(x: string): void
    function foo(x?: string): void {
        /*body*/
    }

The following code is valid in |LANG|:

.. code-block-meta:
   not-subset

.. code-block:: typescript
   :linenos:

    function foo(): void {
      /*body1*/
    }
    function foo(x: string): void {
      /*body2*/
    }

|

.. _Class Fields While Inheriting:

Class Fields While Inheriting
-----------------------------

.. meta:
    frontend_status: None

|TS| allows overriding a class field with a field in a subclass of invariant
or covariant type.
|LANG| supports overriding a class field with a field in a subclass of invariant
type only.
An overriding field can have a new initial value in both languages.

The two situations are represented by the following examples:

.. code-block-meta:

.. code-block:: typescript
   :linenos:

   // Both TypeScript and ArkTS do the same
   class Base {
     field: number = 123
     foo () {
        console.log (this.field)
     }     
   }
   class Derived extends Base {
     field: number = 456
     foo () {
        console.log (this.field)
     }
   }
   let b: Base = new Derived()
   b.foo()  // 456 is printed


   // That will be a compile-time error in ArkTS as type of 'field' in Child
   // differs from 'field' type in Parent
   class Parent {
       field: Object
   }
   class Child extends Parent {
       field: Number 
   }

.. index::
   class field
   inheritance
   overriding
   subclass
   invariant
   covariant
   shadowing
   semantics
   superclass

|

.. _Type void Compatibility:

Type ``void`` Compatibility
---------------------------

.. meta:
    frontend_status: Done

|TS| allows using type ``void`` in union types. |LANG| allows no ``void``
in union types. This situation is represented by the example below:

.. code-block:: typescript
   :linenos:

   type UnionWithVoid = void | number
     // Such type is OK for Typescript, but leads to a compile-time error for ArkTS

|

.. _Invariant Array Assignment:

Invariant Array Assignment
--------------------------

.. meta:
    frontend_status: None

|TS| allows covariant array assignment.
|LANG| allows invariant array assignment only:

.. code-block:: typescript
   :linenos:

    // Typescript
    let a: Object[] = [1, 2, 3]
    let b = [1, 2, 3] // type of 'b' is inferred as number[]
    a = b // That works well for the Typescript

    // ArkTS
    let a: Object[] = [1, 2, 3]
    let b = [1, 2, 3] // type of 'b' is inferred as double[]
    a = b // compile-time error

    let a: Object[] = ["a", "b", "c"]
    let b: string[] = ["a", "b", "c"]
    a = b // compile-time error

.. index::
   covariant array assignment
   invariant array assignment
   array
   assignment

|

.. _Tuples and Arrays:

Tuples and Arrays
-----------------

.. meta:
    frontend_status: None

|TS| allows assignment of tuples into arrays. |LANG| handles arrays and tuples
as different types. |LANG| allows no assignment of tuples into arrays. This
situation is represented by the following example:

.. code-block:: typescript
   :linenos:

   const tuple: [number, number, boolean] = [1, 3.14, true]

   // Typescript accepts such assignment while ArkTS reports an error
   const array: (number|boolean) [] = tuple


|

.. _Extending Class Object:

Extending Class Object
----------------------

.. meta:
    frontend_status: Done

|TS| forbids using ``super`` and ``override`` if class ``Object`` is not
listed explicitly in the ``extends`` clause of a class. |LANG| allows it because
``Object`` is a superclass for any class without an explicit ``extends`` clause:

.. code-block:: typescript
   :linenos:

    // Typescript reports an error while ArkTS compiles with no issues
    class A {
       override toString() {       // compile-time error
           return super.toString() // compile-time error
       }
    }

    class A extends Object { // That is the form supported by TypeScript
       override toString() {
           return super.toString()
       }
    }

.. index::
   class object
   extends clause

|

.. _Syntax of extends and implements Clauses:

Syntax of ``extends`` and ``implements`` Clauses
------------------------------------------------

.. meta:
    frontend_status: Done

|TS| handles entities listed in ``extends`` and ``implements`` clauses as
expressions.
|LANG| handles such clauses at compile time. |LANG| allows no expressions
but *type references*:

.. code-block:: typescript
   :linenos:

    class B {}
    class A extends (B) {} // compile-time error for ArkTS while accepted by TypeScript


.. index::
   extends clause
   implements clause

|

.. _Uniqueness of Functional Objects:

Uniqueness of Functional Objects
--------------------------------

.. meta:
    frontend_status: Done

|TS| and |LANG| handle function objects differently. The equality test can
perform differently. The difference can be eliminated in the future versions of
|LANG|.

.. code-block:: typescript
   :linenos:

    function foo() {}
    foo == foo  // true in Typescript while may be false in ArkTS
    const f1 = foo
    const f2 = foo
    f1 == f2 // true in Typescript while may be false in ArkTS


.. index::
   function object
   equality test

|

.. _Functional Objects for Methods:

Functional Objects for Methods
------------------------------

.. meta:
    frontend_status: None

|TS| and |LANG| handle function objects differently. The semantics of working
with ``this`` is different. |TS| supports ``undefined`` as a value of ``this``.
In |LANG|, ``this`` is always attached to a valid object. It also affects the
comparison of function objects created from methods. |TS| does not bind ``this``
with a function object, while |LANG| does.

.. code-block:: typescript
   :linenos:

    class A {
      method() { console.log (this) }
    }
    const a = new A
    const method = a.method
    method() // Typescript output: undefined, while ArkTS output: object 'a' content

    const aa = new A
    a.method == aa.method // Typescript: true, while ArkTS: false


.. index::
   function object
   this


|

.. _Differences in Namespaces:

Differences in Namespaces
-------------------------

.. meta:
    frontend_status: Done

|TS| allows having non-exported same-name entities in two or more different
declarations of a namespace, because these entities are local to a
particular declaration of the namespace. |LANG| forbids such situations,
because all declarations in |LANG| are merged into one, and thus declarations
become non-distinguishable:


.. code-block:: typescript
   :linenos:

    // Typescript accepts such code, while ArkTS will report a compile-time error
    // as signatures of foo() from the 1st namespace A is identical to the signature
    // of foo() from the 2nd namespace A
    namespace A {
       function foo() { console.log ("foo() from the 1st namespace A declaration") }
       export function bar () { foo() }
    }
    namespace A {
       function foo() { console.log ("foo() from the 2nd namespace A declaration") }
       export function bar_bar() { foo() }
    }

|

.. _Differences in Math.pow:

Differences in Math.pow
-----------------------

.. meta:
    frontend_status: Done

The function ``Math.pow()`` in |TS| conforms to the outdated 2008 version of the
IEEE 754-2008 standard, with the following exceptions:

.. list-table::
   :widths: 40 20 40
   :header-rows: 1

   * - Function Call
     - IEEE 754-2008 Required Result
     - |TS| Result

   * - ``pow (−1, +/-Infinity)``
     - ``1``
     - ``NaN``

   * - ``pow(+1, y)``
     - ``1`` (for any ``y``)
     - ``1`` for any ``y`` except ``NaN`` or ``Infinity``, and ``NaN``
       for ``NaN`` and ``Infinity``


The function ``Math.pow`` in |LANG| conforms to the latest IEEE 754-2019
standard. It also conforms the outdated *IEEE 754-2008* without exceptions
mentioned for |TS| above.

Both the |TS| and |LANG| give correct results for statements added in *IEEE
754-2019* and listed below:

- *pow(x, +Infinity)* is *+0* for *-1 < x < 1*
- *pow(x, +Infinity)* is *+Infinity* for *x < -1* or for *1 < x* (including *+/-Infinity*)
- *pow(x, -Infinity)* is *+Infinity* for *−1 < x < 1*
- *pow(x, -Infinity)* is *+0* for *x < -1* or for *1 < x* (including *+/-Infinity*)
- *pow(+Infinity, y)* is *+0* for a number *y < 0*
- *pow(+Infinity, y)* is *+Infinity* for a number *y > 0*
- *pow(-Infinity, y)* is *−0* for finite *y < 0* an odd integer
- *pow(-Infinity, y)* is *-Infinity* for finite *y > 0* an odd integer
- *pow(-Infinity, y)* is *+0* for finite *y < 0* and not an odd integer
- *pow(-Infinity, y)* is *+Infinity* for finite *y > 0* and not an odd integer


The difference in behavior is summarized as follows:

.. list-table::
   :widths: 40 20 40
   :header-rows: 1

   * - Function Call
     - |TS| Result
     - |LANG| Result

   * - ``pow (−1, +/-Infinity)``
     - ``NaN``
     - ``1``

   * - ``pow(+1, y)``
     - ``1`` for any ``y`` except ``NaN`` or ``Infinity``, and ``NaN`` for
       ``NaN`` and ``Infinity``
     - ``1`` for any ``y``

.. index::
   IEEE 754
   NaN
   Infinity

|

.. _Differences in Constructor Body:

Differences in Constructor Body
-------------------------------

Call to super() in constructors
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

|LANG| requires a call to ``super()`` in a derived class to be the first
statement in a constructor body. |TS| implies no such restriction.
Therefore, the following code valid in |TS| causes a compile-time error
in |LANG|:

.. code-block:: typescript
   :linenos:

    class Base {
       constructor (p: number) {}
    }

    let some_condition = true

    class Derived extends Base {
       // Next constructor body is OK in TS
       // but causes compile_time error in ArkTS
       constructor (p: number) {
           if (some_condition) { super (1) }
           else { super (2) }
       }
    }

This problem can be bypassed by calling ``super()`` with ``ternary expression``
in some cases. The above chunk of code refactored for |LANG| is presented below:

.. code-block:: typescript
   :linenos:

    class Base {
       constructor (p: number) {}
    }

    let some_condition = true

    class Derived extends Base {
       // Next constructor body is OK in TS and ArkTS
       constructor (p: number) {
           super ( some_condition ? 1 : 2)
       }
    }

**Note** The refactored code is also good for |TS|.


Call to this() in secondary constructors
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

|TS| does not allow multiple constructors while |LANG| does.
By calling ``this()``, a secondary constructor can call another constructor of
the class.

A ``this()`` call if used must be the first statement in a secondary
constructor body. Otherwise, a compile-time error occurs. However, the
ternary expression can be used as an argument of ``this()`` to bypass the
limitation in some cases.

This functionality legal in |LANG| only is represented in the following code:

.. code-block:: typescript
   :linenos:

   let flag: boolean = false

   class A {
      num: number
      constructor (p: number)
      {
         this.num = p
      }
      constructor(p: number, s: string)
      {
         this(flag ? p : -p)
         console.log(this.num, " ", s)
      }
    }

    new A(1, " for flag false") // prints '-1 for flag false'
    flag = true
    new A(1, " for flag true") // prints '1 for flag true'

|

.. _Differences in Static Fields Initialization:

Differences in Static Fields Initialization
-------------------------------------------

The order of initialization of static fields in |TS| and |LANG| is different.
|TS| processes source code 'as is', i.e., in the order it is written. That's
why all static fields are initialized when the module is being loaded. |LANG|
initializes static fields either at compile time or right before the first
use of a field.


.. code-block:: typescript
   :linenos:

    class Base1 {
        static field: number = Base1.init_in_base_1()
        private static init_in_base_1() {
           console.log ("Base1 static field initialization")
           return 321
        }
    }

    class Base2 extends Base1 {
        static override field: number = Base3.init_in_base_2()
        private static init_in_base_2() {
           console.log ("Base2 static field initialization")
           return 777
        }
    }
    console.log (Base1.field, Base3.field)
    /* ArkTS output:
        Base1 static field initialization
        321
        Base3 static field initialization
        777
    */
    /* Typescript output:
        Base1 static field initialization
        Base2 static field initialization
        321
        777
    */

|TS| and |LANG| have different requirements to the presence of initialization.
|TS| allows omitting an initialization expression, while |LANG| ensures that
a static field is initialized.

.. code-block:: typescript
   :linenos:

    class AClass {
        static field: string // ArkTS will report a compile-time error
    }
    console.log (AClass.field) // Typescript will output 'undefined'



|

.. _Differences in Overriding Properties:

Differences in Overriding Properties
------------------------------------

|LANG| handles fields and properties (i.e., pairs of accessors) differently.
Thus, mixing fields and properties is not allowed in |LANG|, while in |TS|
the object model is different and allows such a mix.

.. code-block:: typescript
   :linenos:

    class C {
        num: number = 1
    }
    interface I {
        num: number
    }
    class D extends C implements I {
        num: number = 2 // ArkTS compile-time error, conflict in overriding
        // Typescript accepts such situation
    }


|

.. _Differences in Export:

Differences in Export
---------------------

|LANG| enforces export consistency, i.e., all types used in the signature of
an exported declaration, public class, or interface members are also exported.
|TS| allows such declarations.

.. code-block:: typescript
   :linenos:

    class C {} // Not exported
    export function foo (): C { return new C } // Exported function returns non-exported type
    // ArkTS will produce compile-time error, enforcing C to be exported
    // Typescript accepts such situation


|

.. _Differences in Type Alias Export Status:

Differences in Type Alias Export Status
---------------------------------------


|LANG| enforces export consistency, i.e., non-exported types are kept
non-exported in the case of aliasing. |TS| allows such declarations.

.. code-block:: typescript
   :linenos:

    class B {}
    export type A = B 
    // ArkTS will produce compile-time error, as B is not exported
    // Typescript accepts such situation


|

.. _Differences in Ambient Constant Declarations:

Differences in Ambient Constant Declarations
--------------------------------------------

|LANG| enforces exact constant type annotation rather than inferring the type
from a constant value as is allows in |TS|.

.. code-block:: typescript
   :linenos:

    declare const a = 1
    // ArkTS will produce compile-time error, as type is not specified
    // Typescript accepts such declaration assuming 'a' is of type number

|

.. _TS Definite Assignment Assertion:

|TS| Definite Assignment Assertion
----------------------------------

.. meta:
    frontend_status: None

|TS| supports *definite assignment assertion* ('``!:``') both in classes and
interfaces. |LANG| does not use the term *definite assignment assertions* for
'``!:``' but has a similar notion called *late initialization*. |LANG| allows
it in classes only.

The following code is valid in |TS| for both `class A` and `interface I`.
In |LANG|, `interface I` causes a compile-time error:

.. code-block:: typescript
   :linenos:

   class A {
      f !: number
   }

   interface I {
      field!: number
   }

|

.. _TS Tuple Length Property:

|TS| Tuple Length Property
--------------------------

.. meta:
    frontend_status: Done

Tuples have the property '``length``' in |TS| but not in |LANG|.
The following code is valid in |TS| but causes a compile-time error in |LANG|:

.. code-block:: typescript
   :linenos:

   let tuple : [number, string]  = [1, "" ]

   for (let index = 0; index < tuple.length; index++ ) {
      let element: Object = tuple[index]
      // do something with the element
   }


|

.. _TS Rest Parameter of Union Type Unsupported:

|TS| Rest Parameter of Union Type Unsupported
---------------------------------------------

.. meta:
    frontend_status: Done

|TS| supports *rest parameter* of *union type* while |LANG| does not. The
following example legal in |TS| causes a *compile-time error* in |LANG|:

.. code-block:: typescript
   :linenos:

   type U = Array<number> | Array <string>
   // next declaration causes compile time error - rest cannot have a union type
   function bar (...u: U) {
      console.log (u)
   }
   bar (1, 2, 3)  // create array of numbers from numbers
   bar ("a", "bb", "ccc") // create array of strings

|

.. _Object Literal Property Status:

Object Literal Property Status
------------------------------

.. meta:
    frontend_status: Done

If an interface has only a set accessor for some property, then |TS| allows
getting the value of this property despite no getter is defined.
The code pattern causes a compile-time error in |LANG|:

.. code-block:: typescript
   :linenos:

   interface I {
     set attr (attr: number)
   }

   function foo (i: I) {
     console.log (i.attr) // compile_time error in ArkTS
     i.attr = i.attr + 1  // compile_time error in ArkTS
     console.log (i.attr) // compile_time error in ArkTS
   }
   foo ({attr: 123})

|

.. _TS Record Keys of enum Type Unsupported:

|TS| Record Keys of enum Type Unsupported
-----------------------------------------

.. meta:
    frontend_status: Done

|TS| supports using ``enum`` type as a set of keys for a record in a variety
of formats. |LANG| does not support record keys of ``enum`` type. The following
code sample legal in |TS| causes a compile-time error in |LANG|:

.. code-block:: typescript
   :linenos:

   enum Test { A = 'a', B = 'b' }

   // r1, r2, r3 are OK in TS but cause  compile-time error in ArkTS
   let r1: Record<Test, number> = { a: 0, b: 1 }
   let r2: Record<Test, number> = { 'a': 0, 'b': 1 }
   let r3: Record<Test, number> = { [Test.A] : 0, [Test.B]: 1 }

|

.. Boolean({}) gives different results:

Boolean({}) gives different results
-----------------------------------

.. meta:
    frontend_status: Done

In |TS|, the `Boolean({})` gives `true` value, while in |LANG|  it
is `false`:

.. code-block:: typescript
   :linenos:

   let b = new Boolean({})
   console.log(b) // true in TS, false in ArkTS

|

.. Private members and subclasses

Private members and subclasses
------------------------------

.. meta:
    frontend_status: Done

In |TS|, the name of private member of base class can not be used in subclasses.
In |LANG|, names of private members of base class can be reused in subclasses
without limitations. The following code is illegal in |TS| but compiles without
errors in |LANG|:

.. code-block:: typescript
   :linenos:

   class Base {
     private foo() {}
     private x: number = 1
   }

   // In TS, next gives "Class 'Derived' incorrectly extends base class 'Base'"
   class Derived extends Base {
     x: number = 10
     foo() { console.log(this.x) }
   }

|

