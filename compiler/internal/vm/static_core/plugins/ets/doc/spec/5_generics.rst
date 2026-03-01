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

.. _Generics:

Generics
########

.. meta:
    frontend_status: Partly

Class, interface, type alias, method, and function are program entities
that can be parameterized in |LANG| by one or several types. An entity so
parameterized introduces a *generic declaration* (called *a generic* for
brevity).

Types used as generic parameters in a generic are called *type parameters*
(see :ref:`Type Parameters`).

A *generic* must be instantiated in order to be used. *Generic instantiation*
is the action that transforms a *generic* into a real program entity
(non-generic class, interface, union, array, method, or function), or
into another *generic instantiation*. Instantiation (see
:ref:`Generic Instantiations`) can be performed either explicitly or
implicitly.

|LANG| has special types that look similar to generics syntax-wise, and allow
creating new types during compilation (see :ref:`Utility Types`).

.. index::
   class
   array
   interface
   type alias
   method
   function
   entity
   parameterization
   generic
   generic declaration
   generic instantiation
   explicit instantiation
   instantiation
   program entity
   generic parameter
   type parameter
   generic instantiation
   utility type

|

.. _Type Parameters:

Type Parameters
***************

.. meta:
    frontend_status: Done

*Type parameter* is declared in the type parameter section. It can be used as
an ordinary type inside a *generic*.

Syntax-wise, a *type parameter* is an unqualified identifier with a proper
scope (see :ref:`Scopes` for the scope of type parameters). Each type parameter
can have a *constraint* (see :ref:`Type Parameter Constraint`). A type
parameter can have a default type (see :ref:`Type Parameter Default`), and can
specify its *in-* or *out-* variance (see :ref:`Type Parameter Variance`).

.. index::
   generic parameter
   generic
   class
   interface
   function
   parameterization
   type parameter
   unqualified identifier
   scope
   constraint
   default type
   type parameter
   variance
   out-variance
   in-variance
   syntax

The syntax of *type parameter* is presented below:

.. code-block:: abnf

    typeParameters:
        '<' typeParameterList '>'
        ;

    typeParameterList:
        typeParameter (',' typeParameter)*
        ;

    typeParameter:
        ('in' | 'out')? identifier constraint? typeParameterDefault?
        ;

    constraint:
        'extends' type
        ;

    typeParameterDefault:
        '=' typeReference ('[]')?
        ;

A generic class, interface, type alias, method, or function defines a
set of parameterized classes, interfaces, unions, arrays, methods, or functions
respectively (see :ref:`Generic Instantiations`). A single type argument
can define only one set for each possible parameterization of the type parameter
section.

.. index::
   generic declaration
   generic class
   generic interface
   generic function
   generic instantiation
   class
   interface
   function
   type parameter
   parameterization
   array
   type alias
   method
   syntax

|

.. _Type Parameter Constraint:

Type Parameter Constraint
=========================

.. meta:
    frontend_status: Done

If possible instantiations need to be constrained, then an individual
*constraint* can be set for each type parameter after the keyword ``extends``.
A constraint can have the form of any type.

If no constraint is specified,
then the constraint is :ref:`Type Any`, i.e., the lacking explicit constraint
effectively means ``extends Any``. As a consequence, the type parameter is not
compatible with :ref:`Type Object`, and has neither methods nor fields available
for use.

If type parameter *T* has type constraint *S*, then the actual type of the
generic instantiation must be a subtype of *S* (see :ref:`Subtyping`). If the
constraint *S* is a non-nullish type (see :ref:`Nullish Types`), then *T* is
also non-nullish.

.. index::
   constraint
   instantiation
   type parameter
   extends keyword
   type reference
   union type normalization
   object
   compatibility
   assignability
   nullish-type
   non-nullish-type
   any type
   type argument
   generic instantiation
   instantiation
   subtyping
   subtype


.. code-block:: typescript
   :linenos:

    class Base {}
    class Derived extends Base { }
    class SomeType { }

    class G<T extends Base> { }

    let x = new G<Base>      // OK
    let y = new G<Derived>   // OK
    let z = new G<SomeType>  // Compile-time : SomeType is not compatible with Base

    class H<T extends Base|SomeType> {}
    let h1 = new H<Base>     // OK
    let h2 = new H<Derived>  // OK
    let h3 = new H<SomeType> // OK
    let h4 = new H<Object>   // Compile-time : Object is not compatible with Base|SomeType

    class Exotic<T extends "aa"| "bb"> {}
    let e1 = new Exotic<"aa">   // OK
    let e2 = new Exotic<"cc">  // Compile-time : "cc" is not compatible with "aa"| "bb"

    class A {
      f1: number = 0
      f2: string = ""
      f3: boolean = false
    }
    class B <T extends keyof A> {}
    let b1 = new B<'f1'>    // OK
    let b2 = new B<'f0'>    // Compile-time error as 'f0' does not fit the constraint
    let b3 = new B<keyof A> // OK

A type parameter of a generic can *depend* on another type parameter
of the same generic.

.. index::
   type parameter
   generic

.. code-block:: typescript
   :linenos:

    class G<T, S extends T> {}

    class Base {}
    class Derived extends Base { }
    class SomeType { }

    let x: G<Base, Derived>  // correct: the second argument directly
                             // depends on the first one
    let y: G<Base, SomeType> // error: SomeType does not depend on Base

A :index:`compile-time error` occurs if a type parameter in the type parameter
section depends on itself directly or indirectly:

.. code-block:: typescript
   :linenos:

    class C<T extends T> {} // circular dependency
    class D<T extends R, R extends T> {} // circular dependency
    class E<T extends R, R extends T | undefined> {} // circular dependency

|

.. _Type Parameter Default:

Type Parameter Default
======================

.. meta:
    frontend_status: Done

Type parameters of generic types can have defaults. This situation allows
dropping a type argument when a particular type of instantiation is used.
However, a :index:`compile-time error` occurs if:

- A type parameter without a default type follows a type parameter with a
  default type in the declaration of a generic type;
- Type parameter default refers to a type parameter defined after the current
  type parameter.

The application of this concept to both classes and functions is presented
in the examples below:

.. index::
   type parameter
   default
   generic type
   type argument
   default type
   instantiation
   type instantiation
   class
   function

.. code-block-meta:
    expect-cte:

.. code-block:: typescript
   :linenos:

    class SomeType {}
    interface Interface <T1 = SomeType> { }
    class Base <T2 = SomeType> { }
    class Derived1 extends Base implements Interface { }
    // Derived1 is semantically equivalent to Derived2
    class Derived2 extends Base<SomeType> implements Interface<SomeType> { }

    function foo<T = number>(input: T): T { return input}
    foo(1) // this call is semantically equivalent to next one
    foo<number>(1)

    class C1 <T1, T2 = number, T3> {}
    // That is a compile-time error, as T2 has default but T3 does not

    class C2 <T1, T2 = number, T3 = string> {}
    let c1 = new C2<number>          // equal to C2<number, number, string>
    let c2 = new C2<number, string>  // equal to C2<number, string, string>
    let c3 = new C2<number, Object, number> // all 3 type arguments provided

    function foo <T1 = T2, T2 = T1> () {}
    // That is a compile-time error,
    // as T1's default refers to T2, which is defined after the T1
    // T2's default is valid as it refers to already defined type parameter T1

|

.. _Type Parameter Variance:

Type Parameter Variance
=======================

.. meta:
    frontend_status: Done

Normally, two different instantiations of the same generic class or
interface (like ``Array<number>`` and ``Array<string>``) are handled
as different and unrelated types.
|LANG| supports type parameter variance that allows *subtyping*
relationship between such instantiations (See :ref:`Subtyping`),
depending on the *subtyping* relationship between argument types.

.. index::
   type parameter
   variance
   class
   interface
   generic interface
   generic class
   subtyping
   argument type
   instantiation

When declaring *type parameters* of a generic type, special keywords ``in`` or
``out`` (called *variance modifiers*) are used to specify the variance of the
type parameter (see :ref:`Invariance, Covariance and Contravariance`).

Type parameters with the keyword ``out`` are *covariant*. Covariant type
parameters can be used in the out-position only as follows:

   - Constructors can have ``out`` type parameters as parameters;
   - Methods can have ``out`` type parameters as return types;
   - Fields that have ``out`` type parameters as type must be ``readonly``.
   - Otherwise, a :index:`compile-time error` occurs.

.. index::
   type parameter
   generic type
   in keyword
   out keyword
   variance modifier
   variance
   invariance
   covariance
   covariant
   readonly

Type parameters with the keyword ``in`` are *contravariant*.
Contravariant type parameters can be used in the in-position only as follows:

   - Methods can have ``in`` type parameters as parameter types.
   - Otherwise, a :index:`compile-time error` occurs.

Type parameters with no variance modifier are implicitly *invariant*, and can
occur in any position.

.. index::
   contravariance
   type parameter
   in keyword
   contravariant
   in-position
   invariant
   variance modifier

.. code-block:: typescript
   :linenos:

    class X<in T1, out T2, T3> {

       // T1 can be used in in-position only
       foo (p: T1) {}  // OK
       foo1(p: T1): T1 { return p } // error: T1 in out-position
       fldT1: T1 // error: T1 in invariant position

      constructor (x: T2) { this.fldT2 = x } // OK
      bar(x: T2) : T2 { return x }           // CTE (x in in-position)
      readonly fldT2: T2                     // OK
      bar1() : T2 { return this.fldT2 }      // OK

       // T3 can be used in any position (in-out, write-read)
       fldT3: T3
       method (p: T3): T3 { this.fldT3 = p; return p}  // OK
    }

In case of function types (see :ref:`Function Types`), variance interleaving
occurs.

.. code-block:: typescript
   :linenos:

    class X<in T1, out T2> {
       foo (p: T1): T2 {...}                           // in - out
       foo (p: (p: T2)=> T1) {...}                     // out - in
       foo (p: (p: (p: T1)=>T2)=> T1) {...}            // in - out - in
       foo (p: (p: (p: (p: T2)=> T1)=>T2)=> T1) {...}  // out - in - out - in
       // and further more
    }


.. index::
   function type
   variance interleaving

A :index:`compile-time error` occurs if function or method type parameters
have a variance modifier specified.

.. index::
   function
   method
   type parameter
   variance modifier
   variance

|

.. _Generic Instantiations:

Generic Instantiations
**********************

.. meta:
    frontend_status: Done

As mentioned before, a generic declaration defines a set of corresponding
generic or non-generic entities. The process of instantiation is designed to
do the following:

- Allow producing new generic or non-generic entities;
- Provide every type parameter with a type argument that can be any kind of
  type, including the type argument itself.

As a result of the instantiation process, a new class, interface, union, array,
method, or function is created.

.. code-block:: typescript
   :linenos:

    class A <T> {}
    class B <U, V> extends A<U> { // Here A<U> is a new generic type
        field: A<V>               // Here A<V> is a new generic type
        method (p: A<Object>) {}  // Here A<Object> is a new non-generic type
    }

.. index::
   generic class
   generic instantiation
   interface
   type alias
   method
   function
   instantiation
   generic entity
   non-generic entity
   type parameter
   type argument
   class
   union
   array
   interface

|

.. _Type Arguments:

Type Arguments
==============

.. meta:
    frontend_status: Done

*Type arguments* are non-empty lists of types that are used for instantiation.

The syntax of *type arguments* is presented below:

.. code-block:: abnf

    typeArguments:
        '<' type (',' type)* '>'
        ;

The example below represents instantiations with different forms of type
arguments:

.. code-block:: typescript
   :linenos:

    Array<number>                     // instantiated with type number
    Array<number|string>              // instantiated with union type
    Array<number[]>                   // instantiated with array type
    Array<[number, string, boolean]>  // instantiated with tuple type
    Array<()=>void>                   // instantiated with function type

.. index::
   type argument
   instantiation
   union type
   array type
   tuple type
   function type

|

.. _Explicit Generic Instantiations:

Explicit Generic Instantiations
===============================

.. meta:
    frontend_status: Done

An explicit generic instantiation is a language construct, which provides a
list of *type arguments* (see :ref:`Type Arguments`) that specify real types or
type parameters to substitute corresponding type parameters of a generic:

.. code-block:: typescript
   :linenos:

    class G<T> {}    // Generic class declaration
    let x: G<number> // Explicit class instantiation, type argument provided

    class A {
       method <T> () {}  // Generic method declaration
    }
    let a = new A()
    a.method<string> () // Explicit method instantiation, type argument provided

    function foo <T> () {} // Generic function declaration
    foo <string> () // Explicit function instantiation, type argument provided

    type MyArray<T> = T[] // Generic type declaration
    let array: MyArray<boolean> = [true, false] // Explicit array instantiation, type argument provided

    class X <T1, T2> {}
    // Different forms of explicit instantiations of class X producing new generic entities
    class Y<T> extends X<number, T> { // class Y extends X instantiated with number and T
       f1: X<Object, T> // X instantiated with Object and T
       f2: X<T, string> // X instantiated with T and string
       constructor() {
         this.f1 = new X<Object,T>
         this.f2 = new X<T,string>
       }
    }

.. index::
   instantiation
   generic
   generic instantiation
   type
   type argument
   type parameter
   array
   function
   method
   string

A :index:`compile-time error` occurs if type arguments are provided for
non-generic class, interface, type alias, method, or function.

In the explicit generic instantiation *G* <``T``:sub:`1`, ``...``, ``T``:sub:`n`>,
*G* is the generic declaration, and  <``T``:sub:`1`, ``...``, ``T``:sub:`n`> is
the list of its type arguments.

..
   lines 312, 314, 336 - initially the type was *T*:sub:`1`, ``...``, *T*:sub:`n`
   lines 321, 322 - initially *C*:sub:`1`, ``...``, *C*:sub:`n` and *T*:sub:`1`, ``...``, *T*:sub:`n`

If type parameters *T*:sub:`1`, ``...``, *T*:sub:`n` of a generic
declaration are constrained by the corresponding ``C``:sub:`1`, ``...``,
``C``:sub:`n`, then *T*:sub:`i` is assignable to each constraint type
*C*:sub:`i` (see :ref:`Assignability`). All subtypes of the type listed
in the corresponding constraint have each type argument *T*:sub:`i` of the
parameterized declaration ranging over them.

.. index::
   type argument
   non-generic class
   non-generic interface
   non-generic type alias
   non-generic method
   non-generic function
   generic declaration
   class
   interface
   type alias
   method
   function
   generic
   instantiation
   assignability
   assignable type
   constraint
   subtype
   parameterized declaration

A generic instantiation *G* <``T``:sub:`1`, ``...``, ``T``:sub:`n`> is
*well-formed* if **all** of the following is true:

-  The generic declaration name is *G*;
-  The number of type arguments equals the number of type parameters of *G*; and
-  All type arguments are assignable to the corresponding type parameter
   constraint (see :ref:`Assignability`).

A :index:`compile-time error` occurs if an instantiation is not well-formed.

Unless explicitly stated otherwise in appropriate sections, this specification
discusses generic versions of class type, interface type, or function.

Any two generic instantiations are considered *provably distinct* if:

-  Both are parameterizations of distinct generic declarations; or
-  Any of their type arguments is provably distinct.

.. index::
   generic instantiation
   generic declaration
   type parameter
   type argument
   assignability
   constraint
   instantiation
   well-formed instantiation
   class type
   generic type
   interface type
   function
   type argument
   type parameter
   provably distinct instantiation
   parameterization
   distinct generic declaration
   distinct argument

|

.. _Implicit Generic Instantiations:

Implicit Generic Instantiations
===============================

.. meta:
    frontend_status: Done

In an *implicit* instantiation, type arguments are not specified explicitly.
Such type arguments are inferred (see :ref:`Type Inference`) from the context
in which a generic is referred. If type arguments cannot be inferred, then
a :index:`compile-time error` occurs.

Different cases of type argument inference are represented in the examples below:

.. code-block:: typescript
   :linenos:

    function foo <G> (x: G, y: G) {} // Generic function declaration
    foo (new Object, new Object)     // Implicit generic function instantiation
      // based on argument types: the type argument is inferred

    function process <P, R> (arg: P, cb?: (p: P) => R): P | R {
       // return the data itself or if the processing function provided the
       // result of processing
       return cb != undefined ? cb (arg): arg
    }
    process (123, () => {}) // P is inferred as 'int', while R is 'void'

    function bar <T> (p: number) {}
    bar (1) // Compile-time error: type argument cannot be inferred

Implicit instantiation is only possible for generic functions and methods.

If a method of a generic class or interface
*G* <``T``:sub:`1`, ``...``, ``T``:sub:`n`> has its own type parameter ``U`` with
default type (see :ref:`Type Parameter Default`) that equals ``T``:sub:`i`,
and an implicit generic instantion of this method provides no information
to infer a type argument, then the type argument correspondent to ``T``:sub:`i`
is used as the type argument for ``U``.

This situation is represented in the example below:

.. code-block:: typescript
   :linenos:

    class A <T> {  // T is the class type parameter
        foo<U = T> (p: U) {} // U is own type paramater with default T
        bar<V = T> () {}     // V is own type paramater with default T
    }

    // Assume that X1 and X2 are two distinct types
    let a = new A<X1>

    // implicit instantiation:
    a.foo(new X2) // type argument is inferred from ``new X2``
    a.bar()       // class type argument X1 is used as no other information is provided

    // explicit instantiation:
    a.foo<X2> (new X2) // explicit type argument is used
    a.bar<X2> ()       // explicit type argument is used

.. index::
   instantiation
   type argument
   type inference
   inferred type
   generic
   context
   generic method
   generic function
   method
   function

|

.. _Utility Types:

Utility Types
*************

.. meta:
    frontend_status: Done

|LANG| supports several embedded types, called *utility* types. Utility types
allow constructing new types by adjusting properties of initial types, for
which purpose notations identical to generics are used. If the initial types
are class or interface, then the resultant utility types are also handled as 
class or interface types.
All utility type names are accessible as simple names (see :ref:`Accessible`)
in any module across all its scopes. Using these names as user-defined entities
causes a :index:`compile-time error` in accordance with :ref:`Declarations`. An
alphabetically sorted list of utility types is provided below.

.. index::
   embedded type
   class
   interface
   accessibility
   module
   user-defined entity
   declaration
   utility type

|

.. _Awaited Utility Type:

Awaited Utility Type
====================

.. meta:
    frontend_status: None

Type ``Awaited<T>`` constructs a type which includes no type ``Promise``. It
is similar to ``await`` in ``async`` functions, or to the method ``.then()``
in *Promises*. Any occurrence of type ``Promise`` is recursively removed until
a generic, a function, an array, or a tuple type is detected. If type ``Promise``
is not a part of a type ``T`` declaration, then ``Awaited<T>`` leaves ``T``
intact.

If ``T`` in ``Awaited<T>`` is a type parameter, then subtyping for ``Awaited<T>``
is based on the subtyping for ``T``. In other words, ``Awaited<T>``
is a subtype of ``Awaited<U>`` if ``T`` is a subtype of ``U``. The use of type
``Awaited<T>`` is represented in the example below:

.. code-block:: typescript
   :linenos:

    type A = Awaited<Promise<string>>           // type A is string

    type B = Awaited<Promise<Promise<number>>>  // type B is number

    type C = Awaited<boolean | Promise<number>> // type C is boolean | number

    type D = Awaited <Object>                   // type D is Object

    type E = Awaited<Promise<Promise<number>|Promise<string>|Promise<boolean>>>
                                                // type E is number|string|boolean

    type F = Awaited<Promise<(p: Promise<string>) => Promise<number>>> 
                                                // type F is (p: Promise<string>) => Promise<number>>

    type G = Awaited<Promise<Array<Promise<number>>>> 
                                                // type F is Array<Promise<number>>

    function foo <T extends SuperType> (p: Awaited<T>) {}
    function bar <T extends SubType> (p: Awaited<T>) {
        foo (p) // is a valid call as Awaited<T extends SubType> <: Awaited<T extends SuperType>
    }


.. index::
   utility type
   awaited
   promise
   async function
   method

|

.. _NonNullable Utility Type:

NonNullable Utility Type
========================

.. meta:
    frontend_status: None

Type ``NonNullable<T>`` constructs a type by excluding ``null`` and ``undefined``
types. If type ``T`` contains neither ``null`` nor ``undefined``, then
``NonNullable<T>`` leaves ``T`` intact. The use of type ``NonNullable<T>`` is
represented in the example below:

.. code-block:: typescript
   :linenos:

    type X = Object | null | undefined
    type Y = NonNullable<X> // type of 'Y' is Object

    class A <T> {
      field: NonNullable<T> // This is a non-nullable version of the type parameter
      constructor (field: NonNullable<T>) {
        this.field = field
      }
    }

    const a = new A<Object|null> (new Object)
    a.field // type of field is Object

.. index::
   utility type
   null type
   undefined type
   field

|

.. _Partial Utility Type:

Partial Utility Type
====================

.. meta:
    frontend_status: Done

Type ``Partial<T>`` constructs a type with all fields (see
:ref:`Field Declarations`) and properties in their field form (see
:ref:`Interface Properties`) of ``T`` set to optional. ``T`` must be a class or
an interface type. Otherwise, a :index:`compile-time error` occurs. No method
(not even any getter or setter) of ``T`` is a part of the ``Partial<T>`` type.
The use is represented in the example below:

.. code-block:: typescript
   :linenos:

    interface Issue {
        title: string
        description: string
    }

    function process(issue: Partial<Issue>) {
        if (issue.title != undefined) {
            /* process title */
        }
    }

    process({title: "aa"}) // description is undefined

In the example above, type ``Partial<Issue>`` is transformed to a distinct but
analogous type as follows:

.. code-block:: typescript
   :linenos:

    interface /*some name*/ {
        title?: string
        description?: string
    }

.. index::
   type
   property
   optional property
   class type
   interface type
   method
   getter
   setter
   distinct type

Type ``T`` is not assignable to ``Partial<T>`` (see :ref:`Assignability`),
and variables of ``Partial<T>`` are to be initialized with valid object
literals.

.. note::
   If class ``T`` has a user-defined getter, setter, or both, then none of
   those is called when object literal is used with ``Partial<T>`` variables.
   Object literal has its own built-in getters and setters to modify its
   variables. It is represented in the example below:

.. code-block:: typescript
   :linenos:

    interface I {
        property: number
    }
    class A implements I {
        _property: number
        set property(property: number) {
            console.log ("Setter called")
            this._property = property
        }
        get property(): number {
            console.log ("Getter called");
            return this._property
        }
    }

    function foo (partial: Partial<A>) {
        partial.property = 42 // setter to be called
        console.log(partial.property) // getter to be called
    }

    foo ({property: 1}) // No getter or setter from class A is called
    // 42 is printed as object literal has its own setter and getter

.. index::
   type
   assignability
   assignable type
   variable
   initialization
   object literal
   class
   user-defined getter
   built-in getter
   getter
   setter
   user-defined setter
   built-in setter
   property

|

.. _Required Utility Type:

Required Utility Type
=====================

.. meta:
    frontend_status: Done

Type ``Required<T>`` is opposite to ``Partial<T>``, and constructs a type with
all fields (see :ref:`Field Declarations`) and properties in their field form
(see :ref:`Interface Properties`) of ``T`` set to required (i.e., not optional).
``T`` must be a class or an interface type, otherwise a
:index:`compile-time error` occurs. No method (not even any getter or setter)
of ``T`` is part of the  ``Required<T>`` type. Its usage is represented in the
example below:

.. code-block:: typescript
   :linenos:

    interface Issue {
        title?: string
        description?: string
    }

    let c: Required<Issue> = { // CTE: 'description' should be defined
        title: "aa"
    }

In the example above, type ``Required<Issue>`` is transformed to a distinct
but analogous type as follows:

.. code-block:: typescript
   :linenos:

    interface /*some name*/ {
        title: string
        description: string
    }

Type ``T`` is not assignable (see :ref:`Assignability`) to
``Required<T>``, and variables of ``Required<T>`` are to be initialized with
valid object literals.

.. index::
   type
   interface type
   utility type
   assignability
   assignable type
   property
   required property
   method
   getter
   setter
   type
   object literal
   class type
   interface type
   distinct type
   initialization
   variable

|

.. _Readonly Utility Type:

Readonly Utility Type
=====================

.. meta:
    frontend_status: Done

Type ``Readonly<T>`` constructs a type with all fields (see
:ref:`Field Declarations`) and properties in their field form (see
:ref:`Interface Properties`) of ``T`` set to ``readonly``. It means that such
fields and properties of the constructed type cannot be reassigned. ``T`` must
be a class or an interface type, otherwise a :index:`compile-time error`
occurs. No method (not even any getter or setter) of ``T`` is part of the
``Readonly<T>`` type. Its usage is represented in the example below:

.. code-block:: typescript
   :linenos:

    interface Issue {
        title: string
    }

    const myIssue: Readonly<Issue> = {
        title: "One"
    };

    myIssue.title = "Two" // compile-time error: readonly property

.. index::
   type
   readonly type
   utility type
   type readonly
   constructed value
   method
   reassignment
   assignability
   assignable type
   property
   interface type
   getter
   setter

Type ``T`` is assignable (see :ref:`Assignability`) to ``Readonly<T>``,
and allows assignments as a consequence:

.. code-block:: typescript
   :linenos:

    class A {
       f1: string = ""
       f2: number = 1
       f3: boolean = true
    }
    let x = new A
    let y: Readonly<A> = x // OK

|

.. _Record Utility Type:

Record Utility Type
===================

.. meta:
    frontend_status: Done

Type ``Record<K, V>`` constructs a container that maps keys (of type ``K``)
to values of type ``V``.

Type ``K`` is restricted to numeric types (see :ref:`Numeric Types`), type
``string``, string literal types, and union types constructed from
these types.

A :index:`compile-time error` occurs if any other type, or literal of any other
type is used in place of this type.

Its usage is represented in the example below:

.. index::
   record utility type
   utility type
   value
   container
   restriction
   union type
   numeric type
   string type
   literal
   string literal type
   compile-time error
   type
   key
   union type


.. code-block:: typescript
   :linenos:

    type R1 = Record<number, Object>             // ok
    type R2 = Record<boolean, Object>            // compile-time error
    type R3 = Record<"salary" | "bonus", Object> // ok
    type R4 = Record<"salary" | boolean, Object> // compile-time error
    type R5 = Record<"salary" | number, Object>  // ok
    type R6 = Record<string | number, Object>    // ok

Type ``V`` has no restrictions.

A special form of object literals is supported for instances of type ``Record``
(see :ref:`Object Literal of Record Type`).

Access to ``Record<K, V>`` values is performed by an *indexing expression* like
*r[index]*, where *r* is an instance of type ``Record``, and *index* is the
expression of type ``K`` (see :ref:`Record Indexing Expression` for detail).

Variables of type ``Record<K, V>`` can be initialized by a valid object
literal of Record type (see :ref:`Object Literal of Record Type`) where the
literal is valid if the type of key expression is compatible with key type
``K``, and the type of value expression is compatible with the value type ``V``.

.. code-block:: typescript
   :linenos:

    type Keys = 'key1' | 'key2' | 'key3'

    let x: Record<Keys, number> = {
        'key1': 1,
        'key2': 2,
        'key3': 4,
    }
    console.log(x['key2']) // prints 2
    x['key2'] = 8
    console.log(x['key2']) // prints 8

In the example above, ``K`` is a union of literal types and thus the result of
an indexing expression is of type ``V``. In this case it is ``number``.

.. index::
   restriction
   object literal
   literal
   instance
   Record type
   access
   indexing expression
   index expression
   index
   number
   expression
   variable
   compatibility
   value type
   value

|

.. _ReturnType Utility Type:

ReturnType Utility Type
=======================

.. meta:
    frontend_status: None

Type ``ReturnType<T>`` constructs a new type from the return type of a function
type ``T`` (see :ref:`Function Types`). A :index:`compile-time error` occurs if
a non-function type except type ``never`` is provided. The usage is represented
in the example below:

.. code-block:: typescript
   :linenos:

   type MyString = ReturnType<()=> string> // OK
   type Incorrect = ReturnType<string>     // Compile-time error

   function foo<P extends Function, R = ReturnType<P>>() {} 
   /* OK, default type for the second type parameter is the return type of 
      the function type provided as the first type argument */

   foo<()=>number>()  // R is number

   type anAny = ReturnType<Function>  // anAny is Any
   type aNever = Return Type<never>   // aNever is never


|

.. _Utility Type Private Fields:

Utility Type Private Fields
===========================

.. meta:
    frontend_status: Done

Utility types are built on top of other types. Private fields of the initial
type stay in the utility type but they are not accessible (see
:ref:`Accessible`) and cannot be accessed in any way. It is represented in the
example below:

.. code-block:: typescript
   :linenos:

   function foo(): string {  // Potentially some side effect
      return "private field value"
   }

   class A {
      public_field = 444
      private private_field = foo()
   }

   function bar (part_a: Readonly<A>) {
      console.log (part_a)
   }

   bar ({public_field: 777}) // OK, object literal has no field `private_field`
   bar ({public_field: 777, private_field: ""}) // compile-time error, incorrect field name

   bar (new A) // OK, object of type Readonly<A> has field `private_field`

.. index::
   utility type
   private field
   type
   access
   accessibility
   field name

|

.. _Nesting Utility Types:

Nesting Utility Types
===========================

.. meta:
    frontend_status: Partly

If more than one utility types are required then they can be nested as in example below:

.. code-block:: typescript
   :linenos:

   interface Issue {
     title?: string
   }

   const myIssue: Required<Readonly<Issue>> = {
      title: "One"
   };
   console.log(myIssue.title)  // safe: required property
   myIssue.title = "Two" // compile-time error: readonly property

.. index::
   utility type
   private field
   nesting
   readonly property
   required property
   type
   access
   accessibility


.. raw:: pdf

   PageBreak
