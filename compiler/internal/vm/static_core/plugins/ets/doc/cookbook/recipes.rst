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

.. _Recipes:

Recipes
=======

.. _R001:

|CB_R| Objects with property names that are not identifiers are not supported
-----------------------------------------------------------------------------

|CB_RULE| ``arkts-identifiers-as-prop-names``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: LiteralAsPropertyName, ComputedPropertyName
    :fix: Replace property name with identifier

|CB_ERROR|

|LANG| does not support objects with property names that are numbers,
strings, or computed values.

1. For simple objects:

   * Use classes with proper property declarations;
   * Convert numeric keys to semantic identifiers.


2. For Collection-Like Objects:

   * Use arrays for numerically-indexed data;
   * Use Maps for truly dynamic key-value pairs.


3. For Mixed Cases:

   * Consider splitting into multiple structures;
   * Document access patterns clearly.


|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    var x = {"name": 1, 2: 3}

    console.log(x["name"], x[2])

    enum A { 'red' = '1' }

    // a dictionary with string keys
    const colors = {
        "red": "#FF0000",
        "green": "#00FF00"
    };

    // numeric-keyed lookup
    const users = {
        1001: "Alice",
        1002: "Bob"
    };

    // dynamic keys
    function createConfig(key: string) {
        return {[key]: true};
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class X {
        name: number
        two: number
    }
    let x:X = {name: 1, two: 2}
    console.log(x.name, x.two)

    let y = [1, 2, 3]
    console.log(y[2])

    // If you still need a container to store keys of different types,
    // use Map<Object, some_type>:
    let z = new Map<Object, number>()
    z.set("name", 1)
    z.set(2, 2)
    console.log(z.get("name"))
    console.log(z.get(2))

    enum A { RED = '1' }

    // a dictionary with string keys
    class Colors {
        red: string = "#FF0000";
        green: string = "#00FF00";
    }

    // numeric-keyed lookup
    // ArkTS option 1 (if sequential)
    const users = ["Alice", "Bob"];
    // ArkTS option 2 (if non-sequential)
    class Users {
        id1001: string = "Alice";
        id1002: string = "Bob";
    }

    // dynamic keys
    function createConfig(key: string) {
        const config = new Map<string, boolean>();
        config.set(key, true);
        return config;
    }

|CB_SEE|
~~~~~~~~

* :ref:`R002`
* :ref:`R029`
* :ref:`R059`
* :ref:`R060`
* :ref:`R066`
* :ref:`R144`

.. _R002:

|CB_R| ``Symbol()`` API is not supported
----------------------------------------

|CB_RULE| ``arkts-no-symbol``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SymbolType

|CB_ERROR|

|TS| has ``Symbol()`` API that can be used among other things to generate
unique property names at runtime. |LANG| does not support ``Symbol()`` API
because its most popular use cases make no sense in the statically typed
environment. In particular, the object layout is defined at compile time,
and cannot be changed at runtime.

|LANG| supports the usage of ``Symbol.iterator`` in iterable interfaces.

Migration Strategy

1. Audit Codebase:

   * Find all Symbol() usage;
   * Identify iterator cases vs non-iterator cases.

2. Refactor Patterns:

   * Convert symbol properties to class members;
   * Replace metadata symbols with annotation systems;
   * Preserve iterator symbols where valid.

3. Validation:

   * Verify static type definitions;
   * Ensure no runtime property additions;
   * Test iteration functionality.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    const sym = Symbol()
    let o = {
       [sym]: "value"
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class SomeClass {
        someProperty: string = ""
    }
    let o = new SomeClass()

|CB_SEE|
~~~~~~~~

* :ref:`R001`
* :ref:`R029`
* :ref:`R059`
* :ref:`R060`
* :ref:`R066`
* :ref:`R144`

.. _R003:

|CB_R| Private '``#``' identifiers are not supported
----------------------------------------------------

|CB_RULE| ``arkts-no-private-identifiers``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: PrivateIdentifier

|CB_ERROR|

|LANG| does not use private identifiers starting with the character '``#``'.
Use the keyword ``private`` instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    /*
     * Such notation for private fields is not supported in ArkTS:
    class C {
        #foo: number = 42
    }
    */

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class C {
        private foo: number = 42
    }


.. _R004:

|CB_R| Use unique names for types and namespaces.
-------------------------------------------------

|CB_RULE| ``arkts-unique-names``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: DeclWithDuplicateName

|CB_ERROR|

Names for all types (classes, interfaces, enums) and namespaces must be unique
within the same scope level and distinct from other names, e.g., variable names
and function names.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // Collision between variable and type
    let DataProcessor: string;
    class DataProcessor {} // Compile-time error
                           // Type alias name and variable name are the same

    // Namespace/type conflict
    namespace Utilities {
        export function log() {}
    }
    interface Utilities {} // Error

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // Unique names at same scope
    let dataProcessor: string;
    class DataHandler {}

    // Scoped uniqueness
    namespace Network {
        export function send() {}
    }
    namespace FileSystem {
        export interface Network {} // Allowed in different scope
    }

.. _R005:

|CB_R| Use ``let`` instead of ``var``
-------------------------------------

|CB_RULE| ``arkts-no-var``
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: VarDeclaration

|CB_ERROR|

|LANG| does not support ``var``. Use ``let`` instead.

.. note::
   Scopes of ``var`` and ``let`` are different in |TS| so it is also
   necessary to revise place of variable declaration.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    function f(shouldInitialize: boolean) {
        if (shouldInitialize) {
           var x = 10
        }
        return x
    }

    console.log(f(true))  // 10
    console.log(f(false)) // "undefined"

    let upper_let = 0
    {
        var scoped_var = 0 // same scope as for upper_let
        let scoped_let = 0 // really scoped
        upper_let = 5
    }
    scoped_var = 5 // Visible
    scoped_let = 5 // Compile-time error

|CB_OK|
~~~~~~~

.. code-block:: typescript

    function f(shouldInitialize: boolean): number|undefined {
        let x: number|undefined = undefined
        if (shouldInitialize) {
            x = 10
        }
        return x
    }

    console.log(f(true))  // 10
    console.log(f(false)) // "undefined"

    let upper_let = 0
    let scoped_var = 0  // ``var`` changed to ``let``, placement fixed
    {
        let scoped_let = 0
        upper_let = 5
    }
    scoped_var = 5
    scoped_let = 5 // Compile-time error

.. _R008:

|CB_R| Use explicit types instead of ``any``, ``unknown``
---------------------------------------------------------

|CB_RULE| ``arkts-no-any-unknown``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: AnyType, UnknownType, EsObjectType, EsObjectAssignment, EsObjectAccess

|CB_ERROR|

|LANG| does not support the types ``any`` and ``unknown``. Specify
types explicitly.

If your |LANG| code has to interoperate with standard |TS| or |JS| codes,
and if type information is not available or is impossible to obtain, then you
can use a special ``ESObject`` type to work with dynamic objects. Note that you
must avoid such objects at all cost as they reduce type checking (i.e., the code
is less stable and more error-prone) and cause severe runtime overhead. The
usage of ``ESObject`` still produces a warning message.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let value1: any
    value1 = true
    value1 = 42

    let value2: unknown
    value2 = true
    value2 = 42

    // Let's assume that we have no information for external_function
    // because it is defined in JavaScript code:
    let something: any = external_function()
    console.log("someProperty of something:", something.someProperty)

|CB_OK|
~~~~~~~

.. code-block:: typescript

    let value_b: boolean = true // OR: let value_b = true
    let value_n: number = 42 // OR: let value_n = 42
    let value_o1: Object = true
    let value_o2: Object = 42

    // Let's assume that we have no information for external_function
    // because it is defined in JavaScript code:
    let something: ESObject = external_function()
    console.log("someProperty of something:", something.someProperty)

|CB_SEE|
~~~~~~~~

* :ref:`R145`

.. _R014:

|CB_R| Use ``class`` instead of a type with call signature
----------------------------------------------------------

|CB_RULE| ``arkts-no-call-signatures``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: CallSignature

|CB_ERROR|

|LANG| does not support call signatures in object types. Use classes instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    type DescribableFunction = {
        description: string
        (someArg: number): string // call signature
    }

    function doSomething(fn: DescribableFunction) {
        console.log(fn.description + " returned " + fn(6))
    }

    const f: DescribableFunction = (v:number): string => { return v.toString();}
    f.description = "desc"

    doSomething(f)

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class DescribableFunction {
        description: string
        invoke(someArg: number): string {
            return someArg.toString()
        }
        constructor() {
            this.description = "desc"
        }
    }

    function doSomething(fn: DescribableFunction) {
        console.log(fn.description + " returned " + fn.invoke(6))
    }

    doSomething(new DescribableFunction())

|CB_SEE|
~~~~~~~~

* :ref:`R015`

.. _R015:

|CB_R| Use ``class`` instead of a type with constructor signature
-----------------------------------------------------------------

|CB_RULE| ``arkts-no-ctor-signatures-type``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ConstructorType

|CB_ERROR|

|LANG| does not support constructor signatures in object types. Use classes
instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class SomeObject {
        s: string;
        constructor(s: string) { this.s = s }
    }

    type SomeConstructor = {
        new (s: string): SomeObject
    }

    function fn(ctor: SomeConstructor, s: string) {
        return new ctor(s)
    }

    console.log(fn(SomeObject, "hello").s)

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class SomeObject {
        f: string
        constructor (s: string) {
            this.f = s
        }
    }

    function fn(s: string): SomeObject {
        return new SomeObject(s)
    }

    console.log(fn("hello").f)

|CB_SEE|
~~~~~~~~

* :ref:`R014`

.. _R016:

|CB_R| Only one static block is supported
-----------------------------------------

|CB_RULE| ``arkts-no-multiple-static-blocks``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: MultipleStaticBlocks

|CB_ERROR|

|LANG| does not allow having several static blocks for class initialization.
Combine multiple static block statements into a single static block.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class C {
        static s: string

        static {
            C.s = "aa"
        }
        static {
            C.s = C.s + "bb"
        }
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript


    class C {
        static s: string

        static {
            C.s = "aa"
            C.s = C.s + "bb"
        }
    }


.. _R017:

|CB_R| Indexed signatures are not supported
-------------------------------------------

|CB_RULE| ``arkts-no-indexed-signatures``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: IndexMember

|CB_ERROR|

|LANG| does not allow indexed signatures. Use arrays instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // Interface with an indexed signature:
    interface StringArray {
        [index: number]: string
    }

    function getStringArray(): StringArray {
        return ["a", "b", "c"]
    }

    const myArray: StringArray = getStringArray()
    const secondItem = myArray[1]

    console.log(secondItem)

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class X {
        f: string[] = []
        constructor(p: string[]) { this.f = p }
    }

    let myArray: X = new X(["a", "b", "c"])
    const secondItem = myArray.f[1]

    console.log(secondItem)


    // or

    class StringArray {
        array: string[];
        constructor(arr: string[]) { this.array = arr; }
        $_get(index: number): string { return this.array[index as int]; }
    }

    function getStringArray() : StringArray {
        return new StringArray(["a", "b", "c"])
    }

    const myArray: StringArray = getStringArray()
    const secondItem = myArray[1]
    console.log(secondItem);

.. _R019:

|CB_R| Use inheritance instead of intersection types
----------------------------------------------------

|CB_RULE| ``arkts-no-intersection-types``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: IntersectionType

|CB_ERROR|

Currently, |LANG| does not support intersection types. Use inheritance
as a workaround.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    interface Identity {
        id: number
        name: string
    }

    interface Contact {
        email: string
        phoneNumber: string
    }

    type Employee = Identity & Contact

|CB_OK|
~~~~~~~

.. code-block:: typescript

    interface Identity {
        id: number
        name: string
    }

    interface Contact {
        email: string
        phoneNumber: string
    }

    interface Employee extends Identity,  Contact {}

.. _R021:

|CB_R| ``this`` typing is supported only for methods with explicit return ``this``
----------------------------------------------------------------------------------

|CB_RULE| ``arkts-this-typing``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ThisTyping

|CB_ERROR|

|LANG| allows type notation using the keyword ``this`` only for a return type
of an instance method of a class or struct.
Such methods can only return ``this`` explicitly (``return this``).

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class C {
        n: number = 0

        m(c: this) {
            console.log(c)
        }

        foo(): this {
            return this.bar()
        }

        bar(): this {
            return this
        }
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class C {
        n: number = 0

        m(c: C) {
            console.log(c)
        }

        foo(): this {
            return this
        }

        bar(): this {
            return this
        }
    }

.. _R022:

|CB_R| Conditional types are not supported
------------------------------------------

|CB_RULE| ``arkts-no-conditional-types``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ConditionalType

|CB_ERROR|

|LANG| does not support conditional type aliases. Introduce a new type with
constraints explicitly, or rewrite logic using ``Object``. |LANG| does not
support the keyword ``infer``.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    type X<T> = T extends number ? T : never

    type Y<T> = T extends Array<infer Item> ? Item : never

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // Provide explicit constraints within type alias
    type X1<T extends number> = T

    // Rewrite with Object. Less type control, need more type checks for safety
    type X2<T> = Object

    // Item has to be used as a generic parameter and need to be properly
    // instantiated
    type YI<Item, T extends Array<Item>> = Item

.. _R025:

|CB_R| Declaring fields in ``constructor`` is not supported
-----------------------------------------------------------

|CB_RULE| ``arkts-no-ctor-prop-decls``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ParameterProperties

|CB_ERROR|

|LANG| does not support declaring class fields in the ``constructor``.
Declare class fields inside the ``class`` declaration instead and assign them
explicitly in the constructor body.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class Person {
        constructor(
            protected ssn: string,
            private firstName: string,
            private lastName: string
        ) {}

        getFullName(): string {
            return this.firstName + " " + this.lastName
        }
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class Person {
        protected ssn: string
        private firstName: string
        private lastName: string

        constructor(ssn: string, firstName: string, lastName: string) {
            this.ssn = ssn
            this.firstName = firstName
            this.lastName = lastName
        }

        getFullName(): string {
            return this.firstName + " " + this.lastName
        }
    }

.. _R027:

|CB_R| Construct signatures are not supported in interfaces
-----------------------------------------------------------

|CB_RULE| ``arkts-no-ctor-signatures-iface``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ConstructorIface

|CB_ERROR|

|LANG| does not support construct signatures. Use methods instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    interface I {
        new (s: string): I
    }

    function fn(i: I) {
        return new i("hello")
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    interface I {
        create(s: string): I
    }

    function fn(i: I) {
        return i.create("hello")
    }

|CB_SEE|
~~~~~~~~

* :ref:`R015`

.. _R028:

|CB_R| Indexed access types are not supported
---------------------------------------------

|CB_RULE| ``arkts-no-aliases-by-index``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: IndexedAccessType

|CB_ERROR|

|LANG| does not support indexed access types. Use type names instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    type Point = {x: number, y: number}
    type N = Point["x"] // is equal to number

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class Point {x: number = 0; y: number = 0}
    type N = number

.. _R029:

|CB_R| Indexed access is not supported for fields
-------------------------------------------------

|CB_RULE| ``arkts-no-props-by-index``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: PropertyAccessByIndex
    :fix: Replace with dot notation

|CB_ERROR|

|LANG| does not support dynamic field declaration and access. Declare all
object fields right in a class. Access only those class fields
that are either declared in a class, or accessible by inheritance. Accessing
any other field is prohibited, and causes a compile-time error.

To access a field, use ``obj.field`` syntax. |LANG| does not support indexed
access (``obj["field"]``), except indexed access to all typed arrays found in
the standard library (e.g., ``Int32Array``) that support access to their
elements through ``container[index]`` syntax, tuples, ``Record`` objects,
and enums.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class Point {
        x: number = 0
        y: number = 0
    }
    let p: Point = {x: 1, y: 2}
    console.log(p["x"])

    class Person {
        name: string = ""
        age: number = 0; // semicolon is required here
        [key: string]: string | number
    }

    let person: Person = {
        name: "John",
        age: 30,
        email: "***@example.com",
        phoneNumber: "18*********",
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class Point {
        x: number = 0
        y: number = 0
    }
    let p: Point = {x: 1, y: 2}
    console.log(p.x)

    class Person {
        name: string
        age: number
        email: string
        phoneNumber: string

        constructor(name: string, age: number, email: string,
                    phoneNumber: string) {
            this.name = name
            this.age = age
            this.email = email
            this.phoneNumber = phoneNumber
        }
    }

    let person = new Person("John", 30, "***@example.com", "18*********")
    console.log(person["name"])         // Compile-time error
    console.log(person.unknownProperty) // Compile-time error

    let arr = new Int32Array(1)
    console.log(arr[0])

.. _R030:

|CB_R| Structural typing is not supported
-----------------------------------------

|CB_RULE| ``arkts-no-structural-typing``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: StructuralIdentity

|CB_ERROR|

Currently, |LANG| does not support structural typing. It is recommended to
use other mechanisms (inheritance, interfaces, or type aliases) instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    interface I1 {
        f(): string
    }

    interface I2 { // I2 is structurally compatible to I1
        f(): string
    }

    class X {
        n: number = 0
        s: string = ""
    }

    class Y { // Y is structurally compatible to X
        n: number = 0
        s: string = ""
    }

    let x = new X()
    let y = new Y()

    console.log("Assign X to Y")
    y = x

    console.log("Assign Y to X")
    x = y

    function foo(x: X) {
        console.log(x.n, x.s)
    }

    // As Y is compatible with X the second call is allowed:
    foo(new X())
    foo(new Y())

|CB_OK|
~~~~~~~

.. code-block:: typescript

    interface I1 {
        f(): string
    }

    type I2 = I1 // I2 is an alias for I1

    class B {
        n: number = 0
        s: string = ""
    }

    // D is derived from B, which explicitly set subtype / supertype relations:
    class D extends B {
        constructor() {
            super()
        }
    }

    let b = new B()
    let d = new D()

    console.log("Assign D to B")
    b = d // ok, B is the superclass of D

    // An attempt to assign b to d results in a compile-time error:
    // d = b

    interface Common {
       n: number
       s: string
    }

    // X implements interface Z, which makes relation between X and Y explicit.
    class X implements Common {
        n: number = 0
        s: string = ""
    }

    // Y implements interface Z, which makes relation between X and Y explicit.
    class Y implements Common {
        n: number = 0
        s: string = ""
    }

    let x: Common = new X()
    let y: Common = new Y()

    console.log("Assign X to Y")
    y = x // ok, both are of the same type

    console.log("Assign Y to X")
    x = y // ok, both are of the same type

    function foo(c: Common) {
        console.log(c.n, c.s)
    }

    // X and Y implement the same interface, thus both calls are allowed:
    foo(new X())
    foo(new Y())

.. _R034:

|CB_R| Type inference in case of generic function calls is limited
------------------------------------------------------------------

|CB_RULE| ``arkts-no-inferred-generic-params``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: GenericCallNoTypeArgs

|CB_ERROR|

|LANG| allows omitting generic type parameters if it is possible to infer
a specific type from the parameters passed to a function. A compile-time
error occurs otherwise. In particular, inference of generic type parameters
based on function return types only is prohibited.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    function choose<T>(x: T, y: T): T {
        return Math.random() < 0.5 ? x : y
    }

    let x = choose(10, 20)   // OK, choose<number>(...) is inferred
    let y = choose("10", 20) // Compile-time error

    function greet<T>(): T {
        return "Hello" as T
    }
    let z = greet() // Type of T is inferred as "unknown"

|CB_OK|
~~~~~~~

.. code-block:: typescript

    function choose<T>(x: T, y: T): T {
        return Math.random() < 0.5 ? x : y
    }

    let x = choose(10, 20)   // OK, choose<number>(...) is inferred
    let y = choose("10", 20) // Compile-time error

    function greet<T>(): T {
        return "Hello" as T
    }
    let z = greet<string>()

.. _R037:

|CB_R| RegExp literals are not supported
----------------------------------------

|CB_RULE| ``arkts-no-regexp-literals``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: RegexLiteral

|CB_ERROR|

Currently, |LANG| does not support RegExp literals. Use library call with
string literals instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let regex: RegExp = /bc*d/

|CB_OK|
~~~~~~~

.. code-block:: typescript

    let regex: RegExp = new RegExp("/bc*d/")

.. _R038:

|CB_R| Object literal must correspond to some explicitly declared class or interface
------------------------------------------------------------------------------------

|CB_RULE| ``arkts-no-untyped-obj-literals``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ObjectLiteralNoContextType

|CB_ERROR|

|LANG| supports the usage of object literals if the compiler can infer
what classes or interfaces such literals correspond to.
A compile-time error occurs otherwise. |LANG| does not support using literals
to initialize classes and interfaces, and particularly so for the initialization
of the following:

* Anything with type ``any``, ``Object``, or ``object``;
* Classes or interfaces with methods;
* Classes that declare a ``constructor`` with parameters.

In addition, |LANG| supports the usage of object literals to initialize the
value of special type ``Record<K, V>``. The type ``K`` denotes an object key,
and is restricted to types ``number`` and ``string``, union types constructed
from these types, and literals of these types.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let o1 = {n: 42, s: "foo"}
    let o2: Object = {n: 42, s: "foo"}
    let o3: object = {n: 42, s: "foo"}

    let oo: Object[] = [{n: 1, s: "1"}, {n: 2, s: "2"}]

    class C2 {
        s: string
        constructor(s: string) {
            this.s = "s =" + s
        }
    }
    let o4: C2 = {s: "foo"}

    abstract class A {}
    let o6: A = {}

    class C4 {
        n: number = 0
        s: string = ""
        f() {
            console.log("Hello")
        }
    }
    let o7: C4 = {n: 42, s: "foo", f: () => {}}

    class Point {
        x: number = 0
        y: number = 0
    }

    function id_x_y(o: Point): Point {
        return o
    }

    // Structural typing is used to deduce that p is Point:
    let p = {x: 5, y: 10}
    id_x_y(p)

    // A literal can be contextually (i.e., implicitly) typed as Point:
    id_x_y({x: 5, y: 10})

    let rec: Record<string, number> = { a: 1, b: 2 }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class C1 {
        n: number = 0
        s: string = ""
    }

    let o1: C1 = {n: 42, s: "foo"}
    let o2: C1 = {n: 42, s: "foo"}
    let o3: C1 = {n: 42, s: "foo"}

    let oo: C1[] = [{n: 1, s: "1"}, {n: 2, s: "2"}]

    class C2 {
        s: string
        constructor(s: string) {
            this.s = "s =" + s
        }
    }
    let o4 = new C2("foo")

    abstract class A {}
    class C extends A {}
    let o6: C = {} // or let o6: C = new C()

    class C4 {
        n: number = 0
        s: string = ""
        f() {
            console.log("Hello")
        }
    }
    let o7 = new C4()
    o7.n = 42
    o7.s = "foo"

    class Point {
        x: number = 0
        y: number = 0

        // constructor() is used before literal initialization
        // to create a valid object. Since there is no other Point constructors,
        // constructor() is automatically added by compiler
    }

    function id_x_y(o: Point): Point {
        return o
    }

    // Explicit type is required for literal initialization
    let p: Point = {x: 5, y: 10}
    id_x_y(p)

    // id_x_y expects Point explicitly
    // New instance of Point is initialized with the literal
    id_x_y({x: 5, y: 10})

    let rec: Record<string, number> = { "a": 1, "b": 2 }

|CB_SEE|
~~~~~~~~

* :ref:`R040`
* :ref:`R043`

.. _R040:

|CB_R| Object literals cannot be used as type declarations
----------------------------------------------------------

|CB_RULE| ``arkts-no-obj-literals-as-types``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ObjectTypeLiteral

|CB_ERROR|

|LANG| does not support the usage of object literals to declare
types in place. Declare classes and interfaces explicitly instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let o: {x: number, y: number} = {
        x: 2,
        y: 3
    }

    type S = Set<{x: number, y: number}>

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class O {
        x: number = 0
        y: number = 0
    }

    let o: O = {x: 2, y: 3}

    type S = Set<O>

|CB_SEE|
~~~~~~~~

* :ref:`R038`
* :ref:`R043`

.. _R043:

|CB_R| Array literals must contain elements of only inferable types
-------------------------------------------------------------------

|CB_RULE| ``arkts-no-noninferable-arr-literals``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ArrayLiteralNoContextType

|CB_ERROR|

Basically, |LANG| infers the type of an array literal as union type of its
contents. However, if at least one element has a non-inferable type (e.g.,
untyped object literal), then a compile-time error occurs.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let a = [{n: 1, s: "1"}, {n: 2, s: "2"}]

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class C {
        n: number = 0
        s: string = ""
    }

    let a1 = [{n: 1, s: "1"} as C, {n: 2, s: "2"} as C] // a1 is of type "C[]"
    let a2: C[] = [{n: 1, s: "1"}, {n: 2, s: "2"}]      // ditto

|CB_SEE|
~~~~~~~~

* :ref:`R038`
* :ref:`R040`

.. _R046:

|CB_R| Use arrow functions instead of function expressions
----------------------------------------------------------

|CB_RULE| ``arkts-no-func-expressions``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: FunctionExpression
    :fix: Convert to arrow function

|CB_ERROR|

|LANG| does not support function expressions. Use arrow functions instead
to specify explicitly.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let f = function (s: string) {
        console.log(s)
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    let f = (s: string) => {
        console.log(s)
    }

.. _R050:

|CB_R| Class literals are not supported
---------------------------------------

|CB_RULE| ``arkts-no-class-literals``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ClassExpression

|CB_ERROR|

|LANG| does not support class literals. Introduce new named class types
explicitly.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    const Rectangle = class {
        constructor(height: number, width: number) {
            this.height = height
            this.width = width
        }

        height
        width
    }

    const rectangle = new Rectangle(0.0, 0.0)

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class Rectangle {
        constructor(height: number, width: number) {
            this.height = height
            this.width = width
        }

        height: number
        width: number
    }

    const rectangle = new Rectangle(0.0, 0.0)

.. _R051:

|CB_R| Classes cannot be specified in ``implements`` clause
-----------------------------------------------------------

|CB_RULE| ``arkts-implements-only-iface``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ImplementsClass

|CB_ERROR|

|LANG| does not allow specifying a class in the ``implements`` clause. Only
interfaces can be specified.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class C {
      foo() {}
    }

    class C1 implements C {
      foo() {}
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    interface C {
      foo(): void
    }

    class C1 implements C {
      foo() {}
    }

.. _R052:

|CB_R| Reassigning object methods is not supported
--------------------------------------------------

|CB_RULE| ``arkts-no-method-reassignment``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: MethodReassignment

|CB_ERROR|

|LANG| does not support re-assigning a method for objects. The layout of objects
in statically-typed languages is fixed, and all instances of the same object
must share identical code of each method.

To add specific behavior for certain objects, create separate wrapper functions,
or use inheritance.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class C {
        foo() {
            console.log("foo")
        }
    }

    function bar() {
        console.log("bar")
    }

    let c1 = new C()
    let c2 = new C()
    c2.foo = bar

    c1.foo() // foo
    c2.foo() // bar

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class C {
        foo() {
            console.log("foo")
        }
    }

    class Derived extends C {
        foo() {
            console.log("Extra")
            super.foo()
        }
    }

    function bar() {
        console.log("bar")
    }

    let c1 = new C()
    let c2 = new C()
    c1.foo() // foo
    c2.foo() // foo

    let c3 = new Derived()
    c3.foo() // Extra foo

.. _R053:

|CB_R| Only ``as T`` syntax is supported for type casts
-------------------------------------------------------

|CB_RULE| ``arkts-as-casts``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: TypeAssertion
    :fix: Replace to 'as' expression

|CB_ERROR|

|LANG| supports the keyword ``as`` as the only syntax for type casts.
Incorrect cast causes a compile-time error, or runtime ``ClassCastException``.
|LANG| does not support the ``<type>`` syntax for type casts.

Use the expression ``new ...`` instead of ``as`` to cast a *primitive* type
(e.g., a ``number`` or a ``boolean``) to a reference type.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class Shape {}
    class Circle extends Shape {x: number = 5}
    class Square extends Shape {y: string = "a"}

    function createShape(): Shape {
        return new Circle()
    }

    let c1 = <Circle> createShape()

    let c2 = createShape() as Circle

    // No report is provided during compilation
    // nor during runtime if cast is wrong:
    let c3 = createShape() as Square
    console.log(c3.y) // "undefined"

    // Important corner case for casting primitives to the boxed counterparts:
    // The left operand is not properly boxed here at runtime
    // because "as" has no runtime effect in TypeScript
    let e1 = (5.0 as Number) instanceof Number // false

    // Number object is created, and instanceof acts as expected:
    let e2 = (new Number(5.0)) instanceof Number // true

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class Shape {}
    class Circle extends Shape {x: number = 5}
    class Square extends Shape {y: string = "a"}

    function createShape(): Shape {
        return new Circle()
    }

    let c2 = createShape() as Circle

    // ClassCastException during runtime is thrown:
    let c3 = createShape() as Square

    // Number object is created, and instanceof acts as expected:
    let e2 = (new Number(5.0)) instanceof Number // true

.. _R054:

|CB_R| JSX expressions are not supported
----------------------------------------

|CB_RULE| ``arkts-no-jsx``
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: JsxElement

|CB_ERROR|

Do not use JSX since no alternative is provided to rewrite it.

.. _R055:

|CB_R| Unary operators '``+``', '``-``', and '``~``' work only on numbers
-------------------------------------------------------------------------

|CB_RULE| ``arkts-no-polymorphic-unops``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: UnaryArithmNotNumber

|CB_ERROR|

|LANG| allows unary operators to work on bigint and numeric types only. A
compile-time error occurs if these operators are applied to other types. Unlike
|TS|, |LANG| does not support implicit casting for strings. Use explicit casting
for strings instead  in |LANG|.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let a = +5        // 5 as number
    let b = +"5"      // 5 as number
    let c = -5        // -5 as number
    let d = -"5"      // -5 as number
    let e = ~5        // -6 as number
    let f = ~"5"      // -6 as number
    let g = +"string" // NaN as number
    let h:bigint = -5n // -5n as bigint

    function returnTen(): string {
        return "-10"
    }

    function returnString(): string {
        return "string"
    }

    let x = +returnTen()    // -10 as number
    let y = +returnString() // NaN

|CB_OK|
~~~~~~~

.. code-block:: typescript

    let a = +5        // 5 as number
    let b = +"5"      // Compile-time error
    let c = -5        // -5 as number
    let d = -"5"      // Compile-time error
    let e = ~5        // -6 as number
    let f = ~"5"      // Compile-time error
    let g = +"string" // Compile-time error
    let h:bigint = -5n // -5n as bigint

    function returnTen(): string {
        return "-10"
    }

    function returnString(): string {
        return "string"
    }

    let x = +returnTen()    // Compile-time error
    let y = +returnString() // Compile-time error

.. _R059:

|CB_R| Operator ``delete`` is not supported
-------------------------------------------

|CB_RULE| ``arkts-no-delete``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: DeleteOperator

|CB_ERROR|

|LANG| assumes that object layout is known at compile time, and cannot be
changed at runtime. Thus, deleting a property makes no sense.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class Point {
        x?: number = 0.0
        y?: number = 0.0
    }

    let p = new Point()
    delete p.y

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // To mimic the original semantics, you can declare a nullable type,
    // and assign null to mark value absence:

    class Point {
        x: number | null = 0
        y: number | null = 0
    }

    let p = new Point()
    p.y = null

|CB_SEE|
~~~~~~~~

* :ref:`R001`
* :ref:`R002`
* :ref:`R029`
* :ref:`R060`
* :ref:`R066`

.. _R060:

|CB_R| Operator ``typeof`` is allowed only in expression contexts
-----------------------------------------------------------------

|CB_RULE| ``arkts-no-type-query``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: TypeQuery

|CB_ERROR|

|LANG| supports the operator ``typeof`` only in an *expression* context.
|LANG| does not support using ``typeof`` to specify type notations.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let n1 = 42
    let s1 = "foo"
    console.log(typeof n1) // "number"
    console.log(typeof s1) // "string"
    let n2: typeof n1
    let s2: typeof s1

|CB_OK|
~~~~~~~

.. code-block:: typescript

    let n1 = 42
    let s1 = "foo"
    console.log(typeof n1) // "number"
    console.log(typeof s1) // "string"
    let n2: number
    let s2: string

|CB_SEE|
~~~~~~~~

* :ref:`R001`
* :ref:`R002`
* :ref:`R029`
* :ref:`R059`
* :ref:`R066`
* :ref:`R144`

.. _R065:

|CB_R| Operator ``instanceof`` is partially supported
-----------------------------------------------------

|CB_RULE| ``arkts-instanceof-ref-types``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: InstanceofUnsupported

|CB_ERROR|

In |TS|, the left-hand side of an ``instanceof`` expression must be of type
``any``, an object type, or a type parameter. Otherwise, the result is
``false``.

In |LANG|, the left-hand side expression can be of any reference type, and the
left operand cannot be a type. Otherwise, a compile-time error occurs.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class X {
        // ...
    }

    let a = (new X()) instanceof Object // true
    let b = (new X()) instanceof X      // true

    let c = X instanceof Object // true, left operand is a type
    let d = X instanceof X      // false, left operand is a type

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class X {
        // ...
    }

    let a = (new X()) instanceof Object // true
    let b = (new X()) instanceof X      // true

    let c = X instanceof Object // Compile-time error, left operand is a type
    let d = X instanceof X      // Compile-time error, left operand is a type

.. _R066:

|CB_R| Operator ``in`` is not supported
---------------------------------------

|CB_RULE| ``arkts-no-in``
~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: InOperator

|CB_ERROR|

|LANG| does not support the operator ``in``. This operator makes little
sense since the object layout is already known at compile time, and cannot
be modified at runtime. Use ``instanceof`` as a workaround if you still need
to check whether certain class members exist.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class Person {
        name: string = ""
    }
    let p = new Person()

    let b = "name" in p // true

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class Person {
        name: string = ""
    }
    let p = new Person()

    let b = p instanceof Person // true, and "name" is guaranteed to be present

|CB_SEE|
~~~~~~~~

* :ref:`R001`
* :ref:`R002`
* :ref:`R029`
* :ref:`R059`
* :ref:`R060`
* :ref:`R144`

.. _R069:

|CB_R| Destructuring assignment is partially supported
------------------------------------------------------

|CB_RULE| ``arkts-no-destruct-assignment``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: DestructuringAssignment

|CB_ERROR|

|LANG| supports destructuring assignment for arrays and tuples. The
destructuring assignment in |LANG| does not support the spread operator and
Object destructuring. Use other idioms instead, e.g., a temporary variable
where applicable.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let one, two;
    [one, two] = [1, 2];
    [one, two] = [two, one];

    let head, tail
    [head, ...tail] = [1, 2, 3, 4]

    class Point {
        x: number = 0.0
        y: number = 0.0
    }

    let x: number, y: number;
    ({x, y} = new Point()); // parentheses are required here

|CB_OK|
~~~~~~~

.. code-block:: typescript

    let one: number, two: number;
    [one, two] = [1, 2];
    [one, two] = [two, one];

    let data: Number[] = [1, 2, 3, 4]
    let head = data[0]
    let tail: Number[] = []
    for (let i = 1; i < data.length; ++i) {
        tail.push(data[i])
    }

    class Point {
        x: number = 0.0
        y: number = 0.0
    }

    let p = new Point()
    let x = p.x
    let y = p.y

.. _R071:

|CB_R| The comma operator '``,``' is supported only in ``for`` loops
--------------------------------------------------------------------

|CB_RULE| ``arkts-no-comma-outside-loops``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: CommaOperator

|CB_ERROR|

|TS| has a comma operator '``,``'. The |LANG| supports that functionality for
comma only in the control part of ``for`` loops (not in the body of the loop).
It is useless otherwise as it makes the execution order harder to understand.

.. note::
    This rule is applied to the *comma operator* only. You can use comma in
    other cases to delimit variable declarations or parameters
    of a function call.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    for (let i = 0, j = 0; i < 10; ++i, j += 2) {
        console.log(i)
        console.log(j)
    }

    let x = 0
    x = (++x, x++) // 1

|CB_OK|
~~~~~~~

.. code-block:: typescript

    for (let i = 0, j = 0; i < 10; ++i, j += 2) {
        console.log(i)
        console.log(j)
    }

    // Use explicit execution order instead of the comma operator:
    let x = 0
    ++x
    x = x++

.. _R074:

|CB_R| Destructuring variable declarations are partially supported
------------------------------------------------------------------

|CB_RULE| ``arkts-no-destruct-decls``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: DestructuringDeclaration

|CB_ERROR|

|LANG| does not support destructuring variable declarations.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let [one, two] = [1, 2]

    class Point {
        x: number = 0.0
        y: number = 0.0
    }

    function returnZeroPoint(): Point {
        return new Point()
    }

    let {x, y} = returnZeroPoint()

|CB_OK|
~~~~~~~

.. code-block:: typescript

    let one: number, two: number; // semicolon is required here
    [one, two] = [1, 2]

    class Point {
        x: number = 0.0
        y: number = 0.0
    }

    function returnZeroPoint(): Point {
        return new Point()
    }

    // Create an intermediate object, and work with it field by field
    // without name restrictions:
    let zp = returnZeroPoint()
    let x = zp.x
    let y = zp.y

.. _R079:

|CB_R| Type annotation in catch clause is not supported
-------------------------------------------------------

|CB_RULE| ``arkts-no-types-in-catch``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: CatchWithUnsupportedType
    :fix: Remove type annotation

|CB_ERROR|

In |TS|, catch clause variable type annotation must be ``any`` or ``unknown``
if specified. Omit type annotations as |LANG| does not support these types.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    try {
        // some code
    }
    catch (a: unknown) {
        // handle error
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    try {
        // some code
    }
    catch (a) {
        // handle error
    }

|CB_SEE|
~~~~~~~~

* :ref:`R087`

.. _R080:

|CB_R| ``for .. in`` is not supported
-------------------------------------

|CB_RULE| ``arkts-no-for-in``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ForInStatement

|CB_ERROR|

|LANG| does not support the iteration over object contents by the
``for .. in`` loop. The ``for .. in`` loop in |TS| iterates over all
enumerable properties of an object, including properties inherited through the
prototype chain. This behavior is inherently dynamic and can lead to unexpected
results when object structures change at runtime.

Instead of ``for .. in``:

1. For arrays, use the standard ``for`` loop with numeric indices;
2. For object properties, use ``Object.keys()`` or ``Object.entries()``
   with a ``for .. of`` loop;
3. For iterating over collections, use the ``for .. of`` loop.


|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class CC { f: number = 1; q: string = "q" }
    let a: number[] = [1.0, 2.0, 3.0]

    for (let i in a) {
        console.log(a[i])
    }

    let obj: CC = new CC()
    for (let key in obj) {
        console.log(key)
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class CC { f: number = 1; q: string = "q" }
    let a: number[] = [1.0, 2.0, 3.0]

    for (let i = 0; i < a.length; ++i) {
        console.log(a[i])
    }

    let obj: CC = new CC()
    for (let key: string of  Object.keys(obj)) {
        console.log(key)
    }

.. _R083:

|CB_R| Mapped type expression is not supported
----------------------------------------------

|CB_RULE| ``arkts-no-mapped-types``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: MappedType

|CB_ERROR|

|LANG| does not support mapped types. Use other language idioms and regular
classes to achieve that same behavior.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class C {
        n: number = 0
        s: string = ""
    }

    type OptionsFlags<Type> = {
        [Property in keyof Type]: boolean
    }
    let CFlags: OptionsFlags<C>

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class C {
        n: number = 0
        s: string = ""
    }

    class CFlags {
        n: boolean = false
        s: boolean = false
    }

.. _R084:

|CB_R| ``with`` statement is not supported
------------------------------------------

|CB_RULE| ``arkts-no-with``
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: WithStatement

|CB_ERROR|

|LANG| does not support the ``with`` statement. Use other language idioms
(including fully qualified names of functions) to achieve that same behavior.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    with (Math) { // Compile-time error, but JavaScript code still can be
                  // emitted (depends on TS strict type check settings)
        let r: number = 42
        console.log("Area: ", PI * r * r)
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    let r: number = 42
    console.log("Area: ", Math.PI * r * r)

.. _R087:

|CB_R| ``throw`` statements cannot accept values of arbitrary types
-------------------------------------------------------------------

|CB_RULE| ``arkts-limited-throw``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ThrowStatement
    :fix: Wrap in 'Error'

|CB_ERROR|

|LANG| supports throwing only objects of the class ``Error`` or any
derived class. |LANG| forbids throwing an arbitrary type (i.e., a ``number``
or ``string``).

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    throw 4
    throw ""
    throw new Error()

|CB_OK|
~~~~~~~

.. code-block:: typescript

    throw new Error()

.. _R090:

|CB_R| Function return type inference is limited
------------------------------------------------

|CB_RULE| ``arkts-no-implicit-return-types``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: LimitedReturnTypeInference
    :fix: Annotate return type

|CB_ERROR|

|LANG| supports type inference for function return types, but this functionality
is currently restricted. In particular, a compile-time error occurs if the
expression in the ``return`` statement is a call to a function or method whose
return value type is omitted. In case of any such error, specify the return type
explicitly.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // Compile-time error with noImplicitAny
    function f(x: number) {
        if (x <= 0) {
            return x
        }
        return g(x)
    }

    // Compile-time error with noImplicitAny
    function g(x: number) {
        return f(x - 1)
    }

    function doOperation(x: number, y: number) {
        return x + y
    }

    console.log(f(10))
    console.log(doOperation(2, 3))

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // Explicit return type is required:
    function f(x: number): number {
        if (x <= 0) {
            return x
        }
        return g(x)
    }

    // Return type may be omitted, it is inferred from f's explicit type:
    function g(x: number) {
        return f(x - 1)
    }

    // In this case, return type will be inferred
    function doOperation(x: number, y: number) {
        return x + y
    }

    console.log(f(10))
    console.log(doOperation(2, 3))

.. _R091:

|CB_R| Destructuring parameter declarations are partially supported
-------------------------------------------------------------------

|CB_RULE| ``arkts-no-destruct-params``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: DestructuringParameter

|CB_ERROR|

|LANG| does not support parameter destructuring. Parameters must be passed to
the function directly, and local names must be assigned manually.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    function drawPoint([x, y] = [0, 0]) {
        console.log(x)
        console.log(y)
    }

    drawPoint([1, 2])

    function drawText({ text = "", location: [x, y] = [0, 0], bold = false }) {
        console.log(text)
        console.log(x)
        console.log(y)
        console.log(bold)
    }

    drawText({ text: "Hello, world!", location: [100, 50], bold: true })

|CB_OK|
~~~~~~~

.. code-block:: typescript

    function drawPoint(x: number = 0, y: number = 0) {
        console.log(x)
        console.log(y)
    }

    drawPoint(1, 2)

    function drawText(text: String, location: number[], bold: boolean) {
        let x = location[0]
        let y = location[1]
        console.log(text)
        console.log(x)
        console.log(y)
        console.log(bold)
    }

    function main() {
        drawText("Hello, world!", [100, 50], true)
    }

.. _R092:

|CB_R| Nested functions are not supported
-----------------------------------------

|CB_RULE| ``arkts-no-nested-funcs``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: LocalFunction

|CB_ERROR|

|LANG| does not support nested functions. Use lambdas instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    function addNum(a: number, b: number) {

        // nested function:
        function logToConsole(message: String): void {
            console.log(message)
        }

        let sum = a + b

        // Invoking the nested function:
        logToConsole("result is " + sum)
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    function addNum(a: number, b: number) {
        // Use lambda instead of a nested function:
        const logToConsole: (message: string) => void = (message: string): void => {
            console.log(message)
        }

        let sum = a + b

        logToConsole("result is " + sum)
    }

.. _R093:

|CB_R| Using ``this`` inside stand-alone functions is not supported
-------------------------------------------------------------------

|CB_RULE| ``arkts-no-standalone-this``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: FunctionContainsThis

|CB_ERROR|

|LANG| does not support using ``this`` inside stand-alone functions and
static methods. Use ``this`` in instance methods only.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    function foo(i: number) {
        this.count = i // Compile-time error only with noImplicitThis
    }

    class A {
        count: number = 1
        m = foo
    }

    let a = new A()
    console.log(a.count) // prints "1"
    a.m(2)
    console.log(a.count) // prints "2"


|CB_OK|
~~~~~~~

.. code-block:: typescript

    class A {
        count: number = 1
        m(i: number) {
            this.count = i
        }
    }

    let a = new A()
    console.log(a.count)  // prints "1"
    a.m(2)
    console.log(a.count)  // prints "2"

|CB_SEE|
~~~~~~~~

* :ref:`R140`

.. _R094:

|CB_R| Generator functions are not supported
--------------------------------------------

|CB_RULE| ``arkts-no-generators``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: GeneratorFunction, YieldExpression

|CB_ERROR|

Currently, |LANG| does not support generator functions.
Use the ``async`` / ``await`` mechanism for multitasking.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    function* counter(start: number, end: number) {
        for (let i = start; i <= end; i++) {
            yield i
        }
    }

    for (let num of counter(1, 5)) {
        console.log(num)
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    async function complexNumberProcessing(n: number): Promise<number> {
        // Some complex logic for processing the number here
        return n
    }

    async function foo() {
        for (let i = 1; i <= 5; i++) {
            console.log(await complexNumberProcessing(i))
        }
    }

    foo()

.. _R096:

|CB_R| Type guarding is supported with ``instanceof`` and ``as``
----------------------------------------------------------------

|CB_RULE| ``arkts-no-is``
~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: IsOperator

|CB_ERROR|

|LANG| does not support the ``is`` operator. Replace ``is`` for ``instanceof``
at all times. Use the ``as`` operator to cast the fields of an object to an
appropriate type before use.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class Foo {
        foo: number = 0
        common: string = ""
    }

    class Bar {
        bar: number = 0
        common: string = ""
    }

    function isFoo(arg: any): arg is Foo {
        return arg.foo !== undefined
    }

    function doStuff(arg: Foo | Bar) {
        if (isFoo(arg)) {
            console.log(arg.foo)    // OK
            console.log(arg.bar)    // Compile-time error
        } else {
            console.log(arg.foo)    // Compile-time error
            console.log(arg.bar)    // OK
        }
    }

    doStuff({ foo: 123, common: '123' })
    doStuff({ bar: 123, common: '123' })

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class Foo {
        foo: number = 0
        common: string = ""
    }

    class Bar {
        bar: number = 0
        common: string = ""
    }

    function isFoo(arg: Object): boolean {
        return arg instanceof Foo
    }

    function doStuff(arg: Object) {
        if (isFoo(arg)) {
            let fooArg = arg as Foo
            console.log(fooArg.foo)     // OK
            console.log(arg.bar)        // Compile-time error
        } else {
            let barArg = arg as Bar
            console.log(arg.foo)        // Compile-time error
            console.log(barArg.bar)     // OK
        }
    }

    doStuff(new Foo())
    doStuff(new Bar())

.. _R099:

|CB_R| Only arrays, tuples, or iterable objects can be spread into a rest parameter or array literal
----------------------------------------------------------------------------------------------------

|CB_RULE| ``arkts-no-spread``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SpreadOperator

|CB_ERROR|

|TS| supports a broad range of scenarios in which the *spread operator* can be
used.
A *spread expression* in |LANG| can be used only within an array literal,
or in an argument passing. The expression must be of array type, tuple type, or
a type with an iterator defined. When porting legacy |TS| code to |LANG|,
unsupported scenarios must be reworked, e.g., by unpacking data manually.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // OK, function receiving array of numbers into `rest`
    function sum(...data: number[])  {
        let d : number = 0
        for (let n of data) {
           d += n
        }
        console.log("Sum is ", d )
    }

    let list1: number[] = [1, 2]

    // OK, spread array into literal
    let list2: number[] = [...list1, 3, 4]

    let t: [number, number] = [3, 4]
    // OK, spread array and tuple into array literal
    let list3 = [ ...list1, ...t ]

    // OK, put numbers into `rest` directly
    sum(0, 1)

    // OK, spread arrays into `rest`
    sum(...list1)
    sum(...list2)
    sum(...list3)

    // TS - OK, ArkTS - compile-time error - extend layout of class
    let p2d = {x: 1, y: 2}
    let p3d = {...p2d, z: 3}
    console.log(p3d.x, p3d.y, p3d.z)


|CB_OK|
~~~~~~~

.. code-block:: typescript

    // OK, function receiving array of numbers into `rest`
    function sum(...data: number[])  {
        let d : number = 0
        for (let n of data) {
           d += n
        }
        console.log("Sum is ", d )
    }

    let list1: number[] = [1, 2]

    // OK, spread array into literal
    let list2: number[] = [...list1, 3, 4]

    let t: [number, number] = [3, 4]
    // OK, spread array and tuple into array literal
    let list3 = [ ...list1, ...t ]

    // OK, put numbers into `rest` directly
    sum(0, 1)

    // OK, spread arrays into `rest`
    sum(...list1)
    sum(...list2)
    sum(...list3)

    // TS - OK, ArkTS - compile-time error - extend layout of class
    // let p2d = {x: 1, y: 2}
    // let p3d = {...p2d, z: 3}
    // console.log(p3d.x, p3d.y, p3d.z)

    // TS - OK, ArkTS - OK: extending layout revised using inheritance
    class Point2D {
        x: number = 0; y: number = 0
    }

    class Point3D {
        x: number = 0; y: number = 0; z: number = 0
        constructor(p2d: Point2D, z: number) {
            this.x = p2d.x
            this.y = p2d.y
            this.z = z
        }
    }

    let p3d = new Point3D({x: 1, y: 2} as Point2D, 3)
    console.log(p3d.x, p3d.y, p3d.z)

.. _R102:

|CB_R| Interface cannot extend interfaces with the same method
--------------------------------------------------------------

|CB_RULE| ``arkts-no-extend-same-prop``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: InterfaceExtendDifProps

|CB_ERROR|

In |TS|, an interface that extends two other interfaces with the same method
must declare that method, and have a combined type as a result. It is not
allowed in |LANG| because |LANG| allows no interface to contain two methods
with signatures that are not distinguishable (e.g., two methods cannot have
the same parameter lists but different return types).

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    interface Mover {
        getStatus(): { speed: number }
    }
    interface Shaker {
        getStatus(): { frequency: number }
    }

    interface MoverShaker extends Mover, Shaker {
        getStatus(): {
            speed: number
            frequency: number
        }
    }

    class C implements MoverShaker {
        private speed: number = 0
        private frequency: number = 0

        getStatus() {
            return { speed: this.speed, frequency: this.frequency }
        }
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class MoveStatus {
        speed: number
        constructor() {
            this.speed = 0
        }
    }
    interface Mover {
        getMoveStatus(): MoveStatus
    }

    class ShakeStatus {
        frequency: number
        constructor() {
            this.frequency = 0
        }
    }
    interface Shaker {
        getShakeStatus(): ShakeStatus
    }

    class MoveAndShakeStatus {
        speed: number
        frequency: number
        constructor() {
            this.speed = 0
            this.frequency = 0
        }
    }

    class C implements Mover, Shaker {
        private move_status: MoveStatus
        private shake_status: ShakeStatus

        constructor() {
            this.move_status = new MoveStatus()
            this.shake_status = new ShakeStatus()
        }

        getMoveStatus(): MoveStatus {
            return this.move_status
        }

        getShakeStatus(): ShakeStatus {
            return this.shake_status
        }

        getStatus(): MoveAndShakeStatus {
            return {
                speed: this.move_status.speed,
                frequency: this.shake_status.frequency
            }
        }
    }

.. _R103:

|CB_R| Declaration merging is not supported
-------------------------------------------

|CB_RULE| ``arkts-no-decl-merging``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: InterfaceMerging

|CB_ERROR|

|LANG| does not support merging declarations. Keep all definitions of classes
and interfaces compact in the codebase.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    interface Document {
        createElement(tagName: any): Element
    }

    interface Document {
        createElement(tagName: string): HTMLElement
    }

    interface Document {
        createElement(tagName: number): HTMLDivElement
        createElement(tagName: boolean): HTMLSpanElement
        createElement(tagName: string, value: number): HTMLCanvasElement
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    interface Document {
        createElement(tagName: number): HTMLDivElement
        createElement(tagName: boolean): HTMLSpanElement
        createElement(tagName: string, value: number): HTMLCanvasElement
        createElement(tagName: string): HTMLElement
        createElement(tagName: Object): Element
    }

.. _R104:

|CB_R| Interfaces cannot extend classes
---------------------------------------

|CB_RULE| ``arkts-extends-only-class``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: InterfaceExtendsClass

|CB_ERROR|

|LANG| does not support interfaces that extend classes. Interfaces can only
extend interfaces.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class Control {
        state: number = 0
    }

    interface SelectableControl extends Control {
        select(): void
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    interface Control {
        state: number
    }

    interface SelectableControl extends Control {
        select(): void
    }

.. _R106:

|CB_R| Constructor function type is not supported
-------------------------------------------------

|CB_RULE| ``arkts-no-ctor-signatures-funcs``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ConstructorFuncs

|CB_ERROR|

|LANG| does not support using constructor function types. Use lambdas instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class Person {
        constructor(
            name: string,
            age: number
        ) {}
    }
    type PersonCtor = new (name: string, age: number) => Person

    function createPerson(Ctor: PersonCtor, name: string, age: number): Person
    {
        return new Ctor(name, age)
    }

    const person = createPerson(Person, 'John', 30)

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class Person {
        constructor(
            name: string,
            age: number
        ) {}
    }
    type PersonCtor = (n: string, a: number) => Person

    function createPerson(Ctor: PersonCtor, n: string, a: number): Person {
        return Ctor(n, a)
    }

    let Impersonizer: PersonCtor = (n: string, a: number): Person => {
        return new Person(n, a)
    }

    const person = createPerson(Impersonizer, "John", 30)

.. _R111:

|CB_R| Enumeration members can be initialized only with same-type compile-time expressions
------------------------------------------------------------------------------------------

|CB_RULE| ``arkts-no-enum-mixed-types``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: EnumMemberNonConstInit

|CB_ERROR|

|LANG| does not support initializing members of enumerations with expressions
that are evaluated during program runtime. All explicitly set initializers
must be of the same type.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    enum E1 {
        A = 0xa,
        B = 0xb,
        C = Math.random(),
        D = 0xd,
        E // 0xe inferred
    }

    enum E2 {
        A = 0xa,
        B = "0xb",
        C = 0xc,
        D = "0xd"
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    enum E1 {
        A = 0xa,
        B = 0xb,
        C = 0xc,
        D = 0xd,
        E // 0xe inferred
    }

    enum E2 {
        A = "0xa",
        B = "0xb",
        C = "0xc",
        D = "0xd"
    }

.. _R113:

|CB_R| ``enum`` declaration merging is not supported
----------------------------------------------------

|CB_RULE| ``arkts-no-enum-merging``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: EnumMerging

|CB_ERROR|

|LANG| does not support merging declarations for ``enum``. Keep the
declaration of each ``enum`` compact in the codebase.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    enum Color {
        RED,
        GREEN
    }
    enum Color {
        YELLOW = 2
    }
    enum Color {
        BLACK = 3,
        BLUE
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    enum Color {
        RED,
        GREEN,
        YELLOW,
        BLACK,
        BLUE
    }

.. _R114:

|CB_R| Namespaces cannot be used as objects
-------------------------------------------

|CB_RULE| ``arkts-no-ns-as-obj``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: NamespaceAsObject

|CB_ERROR|

|LANG| does not support the usage of namespaces as objects.
Classes or modules can be interpreted as placeholders of namespaces.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    namespace MyNamespace {
        export let x: number
    }

    let m = MyNamespace
    m.x = 2

|CB_OK|
~~~~~~~

.. code-block:: typescript

    namespace MyNamespace {
        export let x: number
    }

    MyNamespace.x = 2

.. _R121:

|CB_R| ``require`` and ``import`` assignments are not supported
---------------------------------------------------------------

|CB_RULE| ``arkts-no-require``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ImportAssignment

|CB_ERROR|

|LANG| does not support importing by ``require`` and ``import`` assignments.
Use regular ``import`` instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    import m = require("mod")

|CB_OK|
~~~~~~~

.. code-block:: typescript

    import * as m from "mod"

|CB_SEE|
~~~~~~~~

* :ref:`R126`

.. _R126:

|CB_R| ``export = ...`` assignment is not supported
---------------------------------------------------

|CB_RULE| ``arkts-no-export-assignment``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ExportAssignment

|CB_ERROR|

|LANG| does not support the ``export = ...`` syntax.
Use regular ``export`` / ``import`` instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // module1
    export = Point

    class Point {
        constructor(x: number, y: number) {}
        static origin = new Point(0, 0)
    }

    // module2
    import Pt = require("module1")

    let p = Pt.origin

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // module1
    export class Point {
        constructor(x: number, y: number) {}
        static origin = new Point(0, 0)
    }

    // module2
    import * as Pt from "module1"

    let p = Pt.origin

|CB_SEE|
~~~~~~~~

* :ref:`R121`

.. _R128:

|CB_R| Ambient module declaration is not supported
--------------------------------------------------

|CB_RULE| ``arkts-no-ambient-decls``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ShorthandAmbientModuleDecl

|CB_ERROR|

|LANG| does not support ambient module declaration because it has its
own mechanism to interoperate with |JS|.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    declare module "someModule" {
        export function normalize(s: string): string
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // Import what you need from the original module
    import { normalize } from "someModule"

|CB_SEE|
~~~~~~~~

* :ref:`R129`

.. _R129:

|CB_R| Wildcards in module names are not supported
--------------------------------------------------

|CB_RULE| ``arkts-no-module-wildcards``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: WildcardsInModuleName

|CB_ERROR|

|LANG| does not support wildcards in module names because import is not a
runtime but a compile-time feature in the language.
Use ordinary export syntax instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // Declaration:
    declare module "*!text" {
        const content: string
        export default content
    }

    // Consuming code:
    import fileContent from "some.txt!text"

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // Declaration:
    declare namespace N {
        function foo(x: number): number
    }

    // Consuming code:
    import * as m from "module"
    console.log("N.foo called: ", N.foo(42))

|CB_SEE|
~~~~~~~~

* :ref:`R128`
* :ref:`R130`

.. _R130:

|CB_R| Universal module definitions (UMD) are not supported
-----------------------------------------------------------

|CB_RULE| ``arkts-no-umd``
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: UMDModuleDefinition

|CB_ERROR|

|LANG| does not support *universal module definitions* (UMD) because the
concept of *script* (as opposed to *module*) is not provided in the language.
Besides, import is a compile-time rather than a runtime feature in |LANG|.
Use ordinary syntax for ``export`` and ``import`` instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // math-lib.d.ts
    export const isPrime(x: number): boolean
    export as namespace mathLib

    // in script
    mathLib.isPrime(2)

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // math-lib.d.ts
    namespace mathLib {
        export isPrime(x: number): boolean
    }

    // in program
    import { mathLib } from "math-lib"
    mathLib.isPrime(2)

|CB_SEE|
~~~~~~~~

* :ref:`R129`

.. _R132:

|CB_R| ``new.target`` is not supported
--------------------------------------

|CB_RULE| ``arkts-no-new-target``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: NewTarget

|CB_ERROR|

|LANG| does not support ``new.target`` because the concept of runtime prototype
inheritance is not provided in the language.
This feature is considered not applicable to static typing.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class CustomError extends Error {
        constructor(message?: string) {
            // 'Error' breaks prototype chain here:
            super(message)

            // Restore prototype chain:
            Object.setPrototypeOf(this, new.target.prototype)
        }
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class CustomError extends Error {
        constructor(message?: string) {
            // Call parent's constructor, inheritance chain is static and
            // cannot be modified at runtime
            super(message)
            console.log(this instanceof Error) // true
        }
    }
    let ce = new CustomError()

|CB_SEE|
~~~~~~~~

* :ref:`R136`

.. _R134:

|CB_R| Definite assignment assertions are not supported
-------------------------------------------------------

|CB_RULE| ``arkts-no-definite-assignment``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: DefiniteAssignment

|CB_WARNING|

|LANG| does not support definite assignment assertions ``let v!: T`` because
they are considered an excessive compiler hint.
Use declaration with initialization instead.

.. note::
   |LANG| uses ``!:`` to request late initialization of class fields.
   A check of an entity initialization is performed on a ``read`` operation
   at runtime.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let x!: number // Hint: x will be initialized before usage

    initialize()

    function initialize() {
        x = 10
    }

    console.log("x = " + x)

|CB_OK|
~~~~~~~

.. code-block:: typescript

    function initialize(): number {
        return 10
    }

    let x: number = initialize()

    console.log("x = " + x)

.. _R136:

|CB_R| Prototype assignment is not supported
--------------------------------------------

|CB_RULE| ``arkts-no-prototype-assignment``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: Prototype

|CB_ERROR|

|LANG| does not support prototype assignment because the concept of runtime
prototype inheritance is not provided in the language. This feature is
considered not applicable to static typing. Use the classes and / or interfaces
mechanism instead to statically *combine* methods to data.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    var C = function(p: number) {
        this.p = p // Compile-time error only with noImplicitThis
    }

    C.prototype = {
        m() {
            console.log(this.p)
        }
    }

    C.prototype.q = function(r: number) {
        return this.p == r
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class C {
        p: number = 0
        m() {
            console.log(this.p)
        }
        q(r: number) {
            return this.p == r
        }
    }

|CB_SEE|
~~~~~~~~

* :ref:`R132`

.. _R137:

|CB_R| ``globalThis`` is not supported
--------------------------------------

|CB_RULE| ``arkts-no-globalthis``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: GlobalThis

|CB_WARNING|

|LANG| does not support global scope nor ``globalThis`` because untyped
objects with dynamically changed layouts are unsupported.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // in a global file:
    var abc = 100

    // Refers to 'abc' from above.
    let x = globalThis.abc

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // file1
    export let abc: number = 100

    // file2
    import * as M from "file1"

    let x = M.abc

|CB_SEE|
~~~~~~~~

* :ref:`R139`
* :ref:`R144`

.. _R138:

|CB_R| Some utility types are not supported
-------------------------------------------

|CB_RULE| ``arkts-no-utility-types``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: UtilityType

|CB_ERROR|

Currently, |LANG| does not support utility types from |TS| extensions to the
standard library, except ``Partial``, ``Required``, ``Readonly``, and
``Record``. Generic arguments of these types must be a class or interface type.

For type ``Record<K, V>``, an indexing expression *rec[index]* is of type
``V | undefined``.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    type Person = {
        name: string
        age: number
        location: string
    }

    type QuantumPerson = Omit<Person, "location">

    let persons: Record<string, Person> = {
        "Alice": {
            name: "Alice",
            age: 32,
            location: "Shanghai"
        },
        "Bob": {
            name: "Bob",
            age: 48,
            location: "New York"
        }
    }
    console.log(persons["Bob"].age)
    console.log(persons["Rob"].age) // Runtime exception

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class Person {
        name: string = ""
        age: number = 0
        location: string = ""
    }

    class QuantumPerson {
        name: string = ""
        age: number = 0
    }

    type OptionalPerson = Person | undefined
    let persons: Record<string, OptionalPerson> = {
    // Or:
    // let persons: Record<string, Person | undefined> = {
        "Alice": {
            name: "Alice",
            age: 32,
            location: "Shanghai"
        },
        "Bob": {
            name: "Bob",
            age: 48,
            location: "New York"
        }
    }
    console.log(persons["Bob"]!.age)
    if (persons["Rob"]) { // Explicit value check, no runtime exception
        console.log(persons["Rob"]!.age)
    }

.. _R139:

|CB_R| Declaring properties on functions is not supported
---------------------------------------------------------

|CB_RULE| ``arkts-no-func-props``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: PropertyDeclOnFunction

|CB_ERROR|

|LANG| does not support declaring properties on functions because objects
with dynamically changing layouts are unsupported. This rule applies to function
objects, and function object layout cannot be changed at runtime.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class MyImage {
        // ...
    }

    function readImage(
        path: string, callback: (err: any, image: MyImage) => void
    )
    {
        // ...
    }

    function readFileSync(path: string): number[] {
        return []
    }

    function decodeImageSync(contents: number[]) {
        // ...
    }

    readImage.sync = (path: string) => {
        const contents = readFileSync(path)
        return decodeImageSync(contents)
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class MyImage {
        // ...
    }

    async function readImage(
        path: string, callback: (err: Error, image: MyImage) => void
    ): Promise<MyImage>
    {
        // In real world, the implementation is more complex,
        // involving real network / DB logic, etc.
        return await new MyImage()
    }

    function readImageSync(path: string): MyImage {
        return new MyImage()
    }

|CB_SEE|
~~~~~~~~

* :ref:`R137`

.. _R140:

|CB_R| ``Function.bind`` is not supported
-----------------------------------------

|CB_RULE| ``arkts-no-func-bind``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: FunctionBind

|CB_WARNING|

|LANG| does not allow using standard library function ``Function.bind``.
The standard library requires this API to explicitly set the parameter ``this``
for a called function.
The semantics of ``this`` in |LANG| is restricted to the conventional OOP
style. Using ``this`` in stand-alone functions is prohibited. This function
is thus excessive.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    const person = {
        firstName: "aa",

        fullName: function(): string {
            return this.firstName
        }
    }

    const person1 = {
        firstName: "Mary"
    }

    // This will log "Mary":
    const boundFullName = person.fullName.bind(person1)
    console.log(boundFullName())

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class Person {
        firstName: string

        constructor(firstName: string) {
            this.firstName = firstName
        }
        fullName(): string {
            return this.firstName
        }
    }

    let person = new Person("")
    let person1 = new Person("Mary")

    // This will log "Mary":
    console.log(person1.fullName())

|CB_SEE|
~~~~~~~~

* :ref:`R093`

.. _R142:

|CB_R| ``as const`` assertions are not supported
------------------------------------------------

|CB_RULE| ``arkts-no-as-const``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ConstAssertion

|CB_ERROR|

|LANG| does not support ``as const`` assertions because literal types are
unsupported altogether (unlike in standard |TS| where ``as const`` annotates
literals with corresponding literal types).

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // Type 'hello':
    let x = "hello" as const

    // Type 'readonly [10, 20]':
    let y = [10, 20] as const

    // Type '{ readonly text: "hello" }':
    let z = { text: "hello" } as const

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // Type 'hello':
    let x: "hello" = "hello"

    // Type 'number[]':
    let y: number[] = [10, 20]

    class Label {
        text: string = ""
    }

    // Type 'Label':
    let z: Label = {
        text: "hello"
    }

.. _R143:

|CB_R| Import assertions are not supported
------------------------------------------

|CB_RULE| ``arkts-no-import-assertions``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ImportAssertion

|CB_ERROR|

|LANG| does not support import assertions because import is a compile-time
rather than a runtime feature in the language, and asserting correctness of
imported APIs at runtime makes no sense in a statically typed language.
Use the ordinary ``import`` syntax instead.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    import { obj } from "something.json" assert { type: "json" }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // Correctness of importing T will be checked at compile-time:
    import { something } from "module"

|CB_SEE|
~~~~~~~~

* :ref:`R129`
* :ref:`R130`

.. _R144:

|CB_R| Usage of standard library is restricted
----------------------------------------------

|CB_RULE| ``arkts-limited-stdlib``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: LimitedStdlibApi

|CB_ERROR|

|LANG| does not allow using some APIs from the |TS|/|JS| standard libraries.
Most of the restricted APIs are related to handling objects dynamically, which
is not compatible with static typing. |LANG| forbids the usage of the following
APIs:

- Properties and functions of the global object: ``eval``;

- ``Object``: ``__proto__``, ``__defineGetter__``, ``__defineSetter__``,
  ``__lookupGetter__``, ``__lookupSetter__``, ``create``, ``defineProperties``,
  ``defineProperty``, ``freeze``, ``getOwnPropertyDescriptor``,
  ``getOwnPropertyDescriptors``, ``getOwnPropertySymbols``, ``getPrototypeOf``,
  ``hasOwnProperty``, ``is``, ``isExtensible``, ``isFrozen``, ``isPrototypeOf``,
  ``isSealed``, ``preventExtensions``, ``propertyIsEnumerable``, ``seal``,
  ``setPrototypeOf``;

- ``Reflect``: ``apply``, ``construct``, ``defineProperty``, ``deleteProperty``,
  ``getOwnPropertyDescriptor``, ``getPrototypeOf``, ``isExtensible``,
  ``preventExtensions``, ``setPrototypeOf``;

- ``Proxy``: ``handler.apply()``, ``handler.construct()``,
  ``handler.defineProperty()``, ``handler.deleteProperty()``, ``handler.get()``,
  ``handler.getOwnPropertyDescriptor()``, ``handler.getPrototypeOf()``,
  ``handler.has()``, ``handler.isExtensible()``, ``handler.ownKeys()``,
  ``handler.preventExtensions()``, ``handler.set()``,
  ``handler.setPrototypeOf()``.

|LANG| supports the following APIs partially:

``Object.assign(target: Record<string, Object | null | undefined>``,
``...source: Object[]): Record<string, Object | null | undefined>``.

|CB_SEE|
~~~~~~~~

* :ref:`R001`
* :ref:`R002`
* :ref:`R029`
* :ref:`R060`
* :ref:`R066`
* :ref:`R137`

.. _R145:

|CB_R| Strict type checking is enforced
---------------------------------------

|CB_RULE| ``arkts-strict-typing``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: StrictDiagnostic

|CB_ERROR|

Type checker is not optional in |LANG|: the code must be typed explicitly and
correctly to compile and run.
When porting from the standard |TS|, turn on the following flags:

-  ``noImplicitReturns``,
-  ``strictFunctionTypes``,
-  ``strictNullChecks``, and
-  ``strictPropertyInitialization``.


|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class C {
        n: number // Compile-time error only with strictPropertyInitialization
        s: string // Compile-time error only with strictPropertyInitialization
    }

    // Compile-time error only with noImplicitReturns
    function foo(s: string): string {
        if (s != "") {
            console.log(s)
            return s
        } else {
            console.log(s)
        }
    }

    let n: number = null // Compile-time error only with strictNullChecks

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class C {
        n: number = 0
        s: string = ""
    }

    function foo(s: string): string {
        console.log(s)
        return s
    }

    let n1: number | null = null
    let n2: number = 0

|CB_SEE|
~~~~~~~~

* :ref:`R008`
* :ref:`R146`

.. _R146:

|CB_R| Switching off type checks with in-place comments is not allowed
----------------------------------------------------------------------

|CB_RULE| ``arkts-strict-typing-required``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ErrorSuppression

|CB_ERROR|

Type checker in |LANG| is not optional: code must be typed explicitly and
correctly to compile and run. |LANG| does not allow *suppressing* type checker
in place with special comments. |LANG| does not support ``@ts-ignore`` and
``@ts-nocheck`` annotations in particular.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // @ts-nocheck
    // ...
    // Some code with switched off type checker
    // ...

    let s1: string = null // No error, type checker suppressed

    // @ts-ignore
    let s2: string = null // No error, type checker suppressed

|CB_OK|
~~~~~~~

.. code-block:: typescript

    let s1: string | null = null // No error, properly types
    let s2: string = null // Compile-time error

|CB_SEE|
~~~~~~~~

* :ref:`R008`
* :ref:`R145`

.. _R147:

|CB_R| No dependencies on |TS| code are currently allowed
---------------------------------------------------------

|CB_RULE| ``arkts-no-ts-deps``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: NoTypeScriptDeps

|CB_ERROR|

Currently, the codebase implemented in the standard |TS| language must not
depend on |LANG| through importing the |LANG| codebase. |LANG| does not support
imports in the reverse direction.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // app.ets
    export class C {
        // ...
    }

    // lib.ts
    import { C } from "app"


|CB_OK|
~~~~~~~

.. code-block:: typescript

    // lib1.ets
    export class C {
        // ...
    }

    // lib2.ets
    import { C } from "lib1"

.. _R148:

|CB_R| No decorators except ArkUI decorators are currently allowed
------------------------------------------------------------------

|CB_RULE| ``arkts-no-decorators-except-arkui``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: UnsupportedDecorators

|CB_WARNING|

Currently, |LANG| allows ArkUI decorators only.
Any other decorator causes a compile-time error.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    function classDecorator(x: any, y: any): void {
        //
    }

    @classDecorator
    class BugReport {
    }


|CB_OK|
~~~~~~~

.. code-block:: typescript

    function classDecorator(x: any, y: any): void {
        //
    }

    @classDecorator // compile-time error: unsupported decorator
    class BugReport {
    }

.. _R149:

|CB_R| Classes cannot be used as objects
----------------------------------------

|CB_RULE| ``arkts-no-classes-as-obj``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ClassAsObject

|CB_WARNING|

|LANG| does not support using classes as objects (assigning classes to
variables, etc.) because a ``class`` declaration introduces a new type but
not a value in the language.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class C {
        s: string = ""
        n: number = 0
    }

    let c = C

.. _R150:

|CB_R| ``import`` statements after other statements are not allowed
-------------------------------------------------------------------

|CB_RULE| ``arkts-no-misplaced-imports``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ImportAfterStatement

|CB_ERROR|

All ``import`` directives in |LANG| must precede any other directives,
declarations, and statements in a program.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class C {
        s: string = ""
        n: number = 0
    }

    import foo from "module1"

|CB_OK|
~~~~~~~

.. code-block:: typescript

    import foo from "module1"

    class C {
        s: string = ""
        n: number = 0
    }

.. _R151:

|CB_R| Usage of ``ESObject`` type is restricted
-----------------------------------------------

|CB_RULE| ``arkts-limited-esobj``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: EsObjectType

|CB_WARNING|

|LANG| does not allow using ``ESObject`` type in some cases. Most limitations
are put in place because |LANG| does not support dynamic objects. The usage of
``ESObject`` as type specifier is only permitted in the
local variable declaration scenario.
The initialization of ``ESObject`` type variables is also limited. Such
variables can only be initialized with values that originate from interop, i.e.,
other ``ESObject`` typed variables, ``any``, ``unknown``, anonymous type
variables, etc. Initializing an ``ESObject`` typed variable with a statically
typed value is prohibited. An ``ESObject`` typed variable can only be passed
to interop calls and assigned to other variables of type ``ESObject``.

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // lib.d.ts
    declare function foo(): any
    declare function bar(a: any): number

    // main.ets
    let e0: ESObject = foo() // CTE - ``ESObject`` typed variable can only be local

    function f() {
        let e1 = foo()        // CTE - type of e1 is `any`
        let e2: ESObject = 1  // CTE - can't initialize ESObject with not dynamic values
        let e3: ESObject = {} // CTE - can't initialize ESObject with not dynamic values
        let e4: ESObject = [] // CTE - can't initialize ESObject with not dynamic values
        let e5: ESObject = "" // CTE - can't initialize ESObject with not dynamic values
        e5['prop'] // CTE - can't access dynamic properties of ESObject
        e5[1]      // CTE - can't access dynamic properties of ESObject
        e5.prop    // CTE - can't access dynamic properties of ESObject

        let e6: ESObject = foo() // OK - explicitly annotated as ESObject
        let e7 = e6              // OK - initialize ESObject with ESObject
        bar(e7)                  // OK - ESObject is passed to interop call
    }

|CB_SEE|
~~~~~~~~

* :ref:`R001`
* :ref:`R002`
* :ref:`R029`
* :ref:`R060`
* :ref:`R066`
* :ref:`R137`

.. _R152:

|CB_R| ``Function.apply``, ``Function.call`` are not supported
--------------------------------------------------------------

|CB_RULE| ``arkts-no-func-apply-call``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: FunctionApplyCall

|CB_ERROR|

|LANG| does not allow using standard library functions ``Function.apply``
and ``Function.call``. These APIs are needed in the standard library to
explicitly set the parameter ``this`` for a called function.
The semantics of ``this`` in |LANG| is restricted to the conventional OOP style.
Using ``this`` in a stand-alone function is prohibited.
These functions are thus excessive.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    const person = {
        firstName: "aa",

        fullName: function(): string {
            return this.firstName
        }
    }

    const person1 = {
        firstName: "Mary"
    }

    // This will log "Mary":
    console.log(person.fullName.apply(person1))

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class Person {
        firstName: string

        constructor(firstName: string) {
            this.firstName = firstName
        }
        fullName(): string {
            return this.firstName
        }
    }

    let person = new Person("")
    let person1 = new Person("Mary")

    // This will log "Mary":
    console.log(person1.fullName())

|CB_SEE|
~~~~~~~~

* :ref:`R093`

.. _R153:

|CB_R| The inheritance for ``Sendable`` classes is limited
----------------------------------------------------------

|CB_RULE| ``arkts-sendable-class-inheritance``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SendableClassInheritance

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

``Sendable`` classes can inherit only from other ``Sendable`` classes in |LANG|.
``Non-Sendable`` classes are not allowed to inherit from ``Sendable`` classes.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    class A {}

    @Sendable
    class B extends A {
        constructor() {
            super()
        }
    }

    @Sendable
    class C {}

    class D extends C {
        constructor() {
            super()
        }
    }

|CB_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    @Sendable
    class A {}

    @Sendable
    class B extends A {
        constructor() {
            super()
        }
    }

|CB_SEE|
~~~~~~~~

* :ref:`R154`
* :ref:`R155`
* :ref:`R156`
* :ref:`R157`

.. _R154:

|CB_R| Properties in ``Sendable`` classes and interfaces must have a ``Sendable data`` type
-------------------------------------------------------------------------------------------

|CB_RULE| ``arkts-sendable-prop-types``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SendablePropType

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

All properties of ``Sendable`` classes or interfaces must have
``Sendable data`` types in |LANG|.

``Sendable data`` is data of a type that belongs to one of the following
categories:

* Primitive types: ``boolean``, ``number``, ``string``, ``bigint``,
  ``null``, ``undefined``;
* ``Sendable`` class or interface;
* Type parameter of generic type ``Sendable``;
* ``Const enum`` type;
* Union type with elements that are ``Sendable`` data types.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    class A {}

    interface I {}

    @Sendable
    class B {
        a: A = new A()  // Invalid, 'A' is not Sendable
        b: I = {}       // Invalid, 'I' is not Sendable
    }

|CB_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    // a.ets
    @Sendable
    class A {}

    interface I extends ISendable {}

    enum E {
        A,
        B
    }

    // b.ets
    import { A, I, E } from "a"

    @Sendable
    class B {
        a: A = new A()
        b: I = {}
        c: number = 1
        d: string | null | undefined = undefined
        e: E = E.A
    }

|CB_SEE|
~~~~~~~~

* :ref:`R153`
* :ref:`R155`
* :ref:`R156`
* :ref:`R157`

.. _R155:

|CB_R| Definite assignment assertion is not allowed in ``Sendable`` classes
---------------------------------------------------------------------------

|CB_RULE| ``arkts-sendable-definite-assignment``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SendableDefiniteAssignment

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

|LANG| does not allow using definite assignment assertions on properties of
``Sendable`` classes.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    @Sendable
    class A {
        a!: number
    }

|CB_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    @Sendable
    class A {
        a: number = 1
    }

|CB_SEE|
~~~~~~~~

* :ref:`R153`
* :ref:`R154`
* :ref:`R156`
* :ref:`R157`

.. _R156:

|CB_R| Type arguments of generic ``Sendable`` type must be a ``Sendable data`` type
-----------------------------------------------------------------------------------

|CB_RULE| ``arkts-sendable-generic-types``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SendableGenericTypes

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

Only ``Sendable data`` types are allowed as type arguments of generic
``Sendable`` types in |LANG|.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    class A {}

    @Sendable
    class B<T> {
        a: T | undefined
    }

    let b = new B<A>()  // Invalid, 'A' is not Sendable

|CB_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    // a.ets
    @Sendable
    export class A {
        a: number = 1
    }

    @Sendable
    export class B<T> {}

    // b.ets
    import { A, B } from "a"

    @Sendable
    class C<T> {
        a: T | undefined = undefined
        b: B<T> = new B<T>()
        c: B<number | undefined> = new B<number | undefined>()
    }

    let c1 = new B<A>()
    let c2 = new B<string>()

|CB_SEE|
~~~~~~~~

* :ref:`R153`
* :ref:`R154`
* :ref:`R155`
* :ref:`R157`

.. _R157:

|CB_R| Only imported variables can be captured by ``Sendable`` class
--------------------------------------------------------------------

|CB_RULE| ``arkts-sendable-imported-variables``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SendableImportedVariables

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

|LANG| does not support sharing closures at runtime. Therefore, ``Sendable``
classes are allowed neither to capture local variable, nor to use a function or
class from the same module, as otherwise a closure is created. Imported
variables, classes, and functions only can be used inside a ``Sendable`` class
body.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    let foo: number = 1

    function bar() {
        console.log("hello")
    }

    @Sendable
    class A {}

    @Sendable
    class B {
        a: A = new A()     // Invalid, 'A' is not imported
        b: number = foo    // Invalid, 'foo' is not imported

        m(): number {
            bar()          // Invalid, 'bar' is not imported
            return foo     // Invalid, 'foo' is not imported
        }
    }

|CB_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    // a.ets
    export let foo: number = 1

    export function bar() {
        console.log("hello")
    }

    @Sendable
    export class A {}

    // b.ets
    import {foo, bar, A} from "a"

    @Sendable
    class B {
        a: A = new A()
        b: number = foo

        m(): number {
            bar()
            return foo
        }
    }

|CB_SEE|
~~~~~~~~

* :ref:`R153`
* :ref:`R154`
* :ref:`R155`
* :ref:`R156`

.. _R158:

|CB_R| Only ``@Sendable`` decorator can be used on ``Sendable`` class
---------------------------------------------------------------------

|CB_RULE| ``arkts-sendable-class-decorator``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SendableClassDecorator

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

Only ``@Sendable`` decorator is allowed on a ``Sendable`` class in |LANG|.
In addition, decorators cannot be applied to fields, methods, accessors,
or constructor/method parameters of a ``Sendable`` class.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    import { DecoratorA, DecoratorB, DecoratorC, DecoratorD } from "decorator"

    @DecoratorA
    @Sendable
    class A {
        @DecoratorB
        a: number = 1

        @DecoratorC
        m(@DecoratorD p: number) {}
    }

|CB_SEE|
~~~~~~~~

* :ref:`R153`
* :ref:`R154`

.. _R159:

|CB_R| Objects of ``Sendable`` type cannot be initialized by using object literal or array literal
--------------------------------------------------------------------------------------------------

|CB_RULE| ``arkts-sendable-obj-init``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SendableObjectInitialization

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

|LANG| does not support using object literals or array literals to initialize
objects of type ``Sendable``.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    import {collections} from "@arkts.collections.d.ets"

    let a: collections.Array<number> = [1, 2, 3]

    @Sendable
    class A {
        a: number = 1
    }

    let b: A = {a: 2}

|CB_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    import {collections} from "@arkts.collections.d.ets"

    let a: collections.Array<number> = new collections.Array<number>([1, 2, 3])

    @Sendable
    class A {
        a: number

        constructor(a: number) {
            this.a = a
        }
    }

    let b: A = new A(2)

|CB_SEE|
~~~~~~~~

* :ref:`R153`
* :ref:`R154`

.. _R160:

|CB_R| Computed property names are not allowed in ``Sendable`` classes
----------------------------------------------------------------------

|CB_RULE| ``arkts-sendable-computed-prop-name``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SendableComputedPropName

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

|LANG| does not allow using computed values to declare properties in
``Sendable`` classes.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    enum E {
        B = "b"
    }

    @Sendable
    class A {
        ['a']: number = 1
        [E.B]: number = 2
    }

|CB_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    @Sendable
    class A {
        a: number = 1
        b: number = 2
    }

|CB_SEE|
~~~~~~~~

* :ref:`R153`
* :ref:`R154`

.. _R161:

|CB_R| Casting ``Non-sendable`` data to ``Sendable`` type is not allowed
------------------------------------------------------------------------

|CB_RULE| ``arkts-sendable-as-expr``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SendableAsExpr

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

|LANG| does not allow casting *non-sendable* data to ``Sendable`` type.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    class A {}

    @Sendable
    class B {}

    let c: B = new A() as B

|CB_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    @Sendable
    class A {}

    class B {}

    let c: B = new A() as B

|CB_SEE|
~~~~~~~~

* :ref:`R153`
* :ref:`R154`

.. _R162:

|CB_R| Importing a module for side effects only is not supported in shared modules
----------------------------------------------------------------------------------

|CB_RULE| ``arkts-no-side-effects-imports``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SharedNoSideEffectImport

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

|LANG| does not support importing a module for side effects only in a shared
module.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    import 'module' // Error, importing a module for side effects
    'use shared'

|CB_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    import { X, Y } from 'module'
    'use shared'

|CB_SEE|
~~~~~~~~

* :ref:`R163`
* :ref:`R164`

.. _R163:

|CB_R| Only ``Sendable`` entities can be exported in shared modules
-------------------------------------------------------------------

|CB_RULE| ``arkts-shared-module-exports``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SharedModuleExports

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

Only ``Sendable`` entities can be exported in shared modules in |LANG|.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    // a.ets
    'use shared'

    export enum E { A, B } // Error, regular enum is not sendable
    export class C {}      // Error, class C is not sendable
    export let v1: C       // Error, v1 has a non-sendable type

    type T1 = C
    export { T1 } // Error, type T1 is aliasing the non-sendable type
    let v2: T1
    export { v2 } // Error, v2 has a non-sendable type

    export { D } from 'b'  // Error, re-exporting non-sendable class
    export { v3 } from 'b' // Error, re-exporting variable with non-sendable type

    // b.ets
    export class D {}
    export let v3: D

|CB_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    // a.ets
    'use shared'

    export const enum E { A, B }

    @Sendable
    export class C {}

    export let v1: C

    type T1 = C
    let v2: T1
    export { T1, v2 }

    export { D, v3, v4 } from 'b'

    // b.ets
    @Sendable
    export class D {}
    export let v3: D
    export let v4: number

|CB_SEE|
~~~~~~~~

* :ref:`R162`
* :ref:`R164`

.. _R164:

|CB_R| ``export * from ...`` is not allowed in shared modules
-------------------------------------------------------------

|CB_RULE| ``arkts-shared-module-no-wildcard-export``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SharedModuleNoWildcardExport

|CB_ERROR|

**Note: This rule describes the restrictions of an ArkTS-specific feature**

|LANG| does not allow using wildcard exports in shared modules. Specify
all exported entities explicitly.

|CB_NON_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    // a.ets
    @Sendable
    export class C {}
    export let a: number

    // b.ets
    'use shared'
    export * from 'a' // Error, wildcard export in a shared module

|CB_COMPLIANT_CODE|
~~~~~~~~~~~~~~~~~~~

.. code-block:: typescript

    // a.ets
    @Sendable
    export class C {}
    export let a: number

    // b.ets
    'use shared'
    export { C, a } from 'a'

|CB_SEE|
~~~~~~~~

* :ref:`R162`
* :ref:`R163`

.. :comment-begin:
    The following IDs are RESERVED for special rules working only in
    the 'interop' mode (undocumented feature), and used internally
    in ArkTS linter. Therefore, they must NOT be used for regular
    cookbook recipes:

    R165, R166, R167, R168, R169, R170

    See https://gitee.com/openharmony/arkcompiler_ets_frontend/pulls/2397

.. :comment-end:

.. _R183:

|CB_R| Object literal properties can only contain name-value pairs
------------------------------------------------------------------

|CB_RULE| ``arkts-obj-literal-props``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: ObjectLiteralProperty

|CB_ERROR|

In |LANG| an object literal is a comma-separated list of name-value pairs
enclosed in curly braces '``{``' and '``}``'. Each name-value pair
consists of an identifier and an expression.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class C {
        a: string = ""
        b: number = 0
    }
    const a = "Alice"
    const b = 42

    let c: C = {
        a,
        b,
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class C {
        a: string = ""
        b: number = 0
    }
    const a = "Alice"
    const b = 42

    let c: C = {
        a: a,
        b: b,
    }

.. _R184:

|CB_R| Optional methods are not supported
-----------------------------------------

|CB_RULE| ``arkts-no-optional-methods``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: OptionalMethod

|CB_ERROR|

|LANG| does not support optional methods, unlike |TS| where methods with '``?``'
are allowed to be not overridden in method overloading, or to be implemented
in interface implementation.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class X {
        throw?() {}
    }

    interface Y {
        throw?(): void
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class X {
        throw() {}
    }

    interface Y {
        throw(): void
    }



.. _R181:

|CB_R| Local classes and interfaces are not supported
-----------------------------------------------------

|CB_RULE| ``arkts-no-local-classes-ifaces``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: LocalClassesInterfaces

|CB_ERROR|

|LANG| does not support local classes and interfaces, unlike |TS| where one can
define a class or interface in any block of code.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    export function foo () {
        class X {}
        new X
    }


|CB_OK|
~~~~~~~

.. code-block:: typescript

    class X {} // Keep class as non-exported
    export function foo () {
        new X
    }

.. _R189:

|CB_R| Numeric semantics is different for integer values
--------------------------------------------------------

|CB_RULE| ``arkts-numeric-semantic``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: NumericSemantics

|CB_WARNING|

|TS| has a single numeric type ``number`` that handles both integer and real
numbers. |LANG| interprets numbers as a variety of numeric types: ``int``,
``long``, ``float``, ``double``. Calculations depends on the context and can
produce different results.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    let n = 1
    console.log(n / 2) // TypeScript output: 0.5
                       // ArkTS output: 0

|CB_OK|
~~~~~~~

.. code-block:: typescript

    let n: int = 1
    console.log(n / 2) // 0

    // OR
    let n: float = 1
    console.log(n / 2) // 0.5

.. _R190:

|CB_R| Stricter assignments into variables of function type
-----------------------------------------------------------

|CB_RULE| ``arkts-incompatible-func-types``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: IncompationbleFunctionType

|CB_ERROR|

|TS| allows more relaxed assignments into variables of function type, while
|LANG| follows stricter rules stated in `Subtyping for Function Types`
in the Specification.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    type FuncType = (p: string) => void
    let f1: FuncType = (p: string): number => { return 0 } // compile-time error in ArkTS

|CB_OK|
~~~~~~~

.. code-block:: typescript

    type FuncType = (p: string) => void
    let f1: FuncType = (p:string) => {
        ((p: string): number => { return 0 })(p)
        // the function with incompatible type may have some side effects,
        // so it should execute
    }


.. _R193:

|CB_R| Operator ``void`` is not supported
-----------------------------------------

|CB_RULE| ``arkts-no-void-operator``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: VoidOperator

|CB_ERROR|

|LANG| does not support the operator ``void``. Replace some code with a lambda
function call, while retaining all side effects.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    void 2 === "2"    // return false
    void (2 === "2")  // return undefined

|CB_OK|
~~~~~~~

.. code-block:: typescript

    (() => {
        2 === "2";
        return undefined;
    })()

.. _R194:

|CB_R| No ``override`` modifier for field declarations
------------------------------------------------------

|CB_RULE| ``arkts-no-override-fields``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: MethodOverridingField

|CB_WARNING|

There is no keyword ``override`` for field declarations in |LANG|.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class Base1 {
        field: number = init_in_base_1()
        private init_in_base_1() {
            console.log ("Base1 field initialization")
            return 123
        }
    }
    interface Base2 {
        field: number
    }
    class Derived extends Base1 implements Base2 {
        override field = init_in_derived() // overriding 'field' and providing new initial value
        private init_in_derived() {
            console.log ("Derived field initialization")
            return 666
        }
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class Base1 {
        field: number = init_in_base_1()
        private init_in_base_1() {
            console.log ("Base1 field initialization")
            return 123
        }
    }
    interface Base2 {
        field: number
    }
    class Derived extends Base1 implements Base2 {
        field = init_in_derived() // overriding 'field' and providing new initial value
        private init_in_derived() {
            console.log ("Derived field initialization")
            return 666
        }
    }

.. _R221:

|CB_R| Importing/re-exporting itself is prohibited
--------------------------------------------------

|CB_RULE| ``arkts-no-import-itself``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: NoTsReExportEts

|CB_ERROR|

Importing and re-exporting itself is prohibited.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // module.ets
    import {} from 'module'

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // just remove that import

.. _R222:

|CB_R| Import for side effect only is prohibited.
-------------------------------------------------

|CB_RULE| ``arkts-no-side-effect-import``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: NoSideEffectImportEtsToTs

|CB_ERROR|

By prohibiting side effect imports, |LANG| ensures that module initialization
occurs in a predictable manner based on explicitly imported dependencies.

If a module truly has no exports but only side effects (e.g., modifying
global state), then:

- Consider refactoring the dependency to expose explicit exports;
- Move the initialization code to a more appropriate location;
- Consider using a different mechanism like explicit initialization functions
  where the side effect is essential.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    import {} from 'module'
    // or
    import 'module'

|CB_OK|
~~~~~~~

.. code-block:: typescript

    import { X, Y } from 'module'

.. _R223:

|CB_R| Empty list of name bindings is prohibited
------------------------------------------------

|CB_RULE| ``arkts-no-empty-import-binding-list``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SharedNoSideEffectImport

|CB_ERROR|

Each binding adds a declaration or declarations to the scope of a module or
a package (see Scopes). Any declaration added so must be distinguishable
in the declaration scope (see Distinguishable Declarations). A compile-time
error occurs if a declaration added to the scope of a module or a package by
a binding is not distinguishable.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    import {} from 'module'

|CB_OK|
~~~~~~~

.. code-block:: typescript

    import { foo } from 'module'

.. _R239:

|CB_R| Keywords that cannot be used as identifiers
--------------------------------------------------

|CB_RULE| ``arkts-invalid-identifier``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: InvalidIdentifier

|CB_ERROR|

|LANG| enforces strict rules about what names can be used as identifiers to
ensure language consistency, prevent ambiguity, and maintain compatibility with
its runtime environment.

Language syntax keywords, predefined types, and standard library entities like
``number``, ``String``, ``Record``, ``Object``, etc. cannot be used as
identifiers.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    function foo(internal: number) {}

|CB_OK|
~~~~~~~

.. code-block:: typescript

    function foo(internal_param: number) {}


.. _R228:

|CB_R| Only ``loop`` statements are allowed to have a label
-----------------------------------------------------------

|CB_RULE| ``arkts-invalid-label``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: InvalidIdentifier

|CB_ERROR|

Only ``loop`` statements are allowed to have a label. Only one label can be
specified for a certain ``loop`` statement.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // Error: Labels on non-loop statements
    L1: function calculateTotal() {
        // Function body
    }

    START: let count = 0;

    BLOCK: {
        console.log("This is a block");
    }

    CONDITION: if (x > 10) {
        console.log("x is greater than 10");
    }

    // Error: Multiple labels on a single loop
    OUTER: INNER: for (let i = 0; i < 10; i++) {
        console.log(i);
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // Correct: Labels on loop statements
    outer: for (let i = 0; i < 5; i++) {
        inner: for (let j = 0; j < 5; j++) {
            if (i * j > 10) {
                break outer; // Breaking out of outer loop
            }
            if (j === 3) {
                continue inner; // Continue inner loop
            }
            console.log(i, j);
        }
    }

    // Correct: Using labeled while loop
    process: while (hasMoreData()) {
        const data = getData();
        if (isInvalid(data)) {
            break process;
        }
        processData(data);
    }

    // Correct: Using labeled do-while loop
    retry: do {
        const result = attemptOperation();
        if (result.success) {
            break retry;
        }
    } while (shouldRetry());

.. _R229:

|CB_R| No support of ``keyof`` types
------------------------------------

|CB_RULE| ``arkts-no-keyof``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: MappedType

|CB_ERROR|

|LANG| does not support ``keyof`` types.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    type Point = { x: number; y: number };
    type P = keyof Point

|CB_OK|
~~~~~~~

.. code-block:: typescript

    class CPoint { x: number; y: number };
    type Point = CPoint
    type P = "x" | "y"

.. _R230:

|CB_R| The parameter with name ``this`` is not supported
--------------------------------------------------------

|CB_RULE| ``arkts-no-this-param``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: IncompationbleFunctionType

|CB_ERROR|

|LANG| does not support a method parameter with the name ``this``. That
restriction helps maintaining consistency in function parameter lists, and
avoid potential confusion between special and regular parameters.

.. note::

  The keyword ``this`` still can be used in |LANG| in *function* and
  *lambda with receiver*  but only for the name  of the first parameter. It has
  a special meaning of the *receiver parameter* in that case.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // Error: Using 'this' as a parameter name
    interface UIElement {
        addClickListener(onclick: (this: void, e: Event) => void): void;
    }

    class Handler {
        info: string;

        constructor(info: string) {
            this.info = info;
        }

        // Error: 'this' parameter not supported in methods
        onClick(this: Handler, e: Event) {
            console.log('Clicked!', this.info);
        }
    }

    // Error: 'this' parameter in standalone function
    function processEvent(this: Document, event: Event) {
        console.log(this.title, event.type);
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // Correct: Avoid 'this' parameter and use arrow functions to preserve context
    interface UIElement {
        addClickListener(onclick: (e: Event) => void): void;
    }

    class Handler {
        info: string;

        constructor(info: string) {
            this.info = info;
        }

        // Method without 'this' parameter
        onClick(e: Event) {
            console.log('Clicked!', this.info);
        }

        // Use arrow function to preserve 'this' context
        getClickHandler() {
            return (e: Event) => {
                console.log('Clicked!', this.info);
            };
        }
    }

    // Correct: Pass context explicitly
    function processEvent(document: Document, event: Event) {
        console.log(document.title, event.type);
    }

    // Usage
    const doc = document;
    processEvent(doc, new Event('click'));

.. _R231:

|CB_R| No support of ``module.exports``
---------------------------------------

|CB_RULE| ``arkts-no-module-exports``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: SharedModuleExportsWarning

|CB_ERROR|

Key differences between CommonJS and ESM that impact |LANG| compatibility:

#. **Static vs. Dynamic**: ESM supports static analysis at compile time,
   allowing better tree-shaking, type checking, and optimizations. CommonJS is
   inherently dynamic, with exports resolved at runtime.

#. **Synchronous vs. Asynchronous**: CommonJS modules are loaded synchronously,
   while ESM was designed with asynchronous loading in mind, making it more
   suitable for browser environments and modern applications.

#. **Single Export vs. Named Exports**: CommonJS uses a single object
   (``module.exports``)  as its export mechanism, while ESM allows for named
   exports that can be statically analyzed.

When migrating from CommonJS to |LANG|'s ESM, do the following:

- Replace ``module.exports = value`` for ``export default value``;
- Replace ``module.exports.name = value`` for ``export const name = value``;
- Replace ``exports.name = value`` for ``export const name = value``;
- Replace export objects for individual named exports or the default export;
- Group multiple exports by using the ``export { name1, name2 }`` syntax.


|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // Error: Using CommonJS module.exports
    module.exports = function foo() {
        return 'Hello ArkTS';
    };

    // Error: Using CommonJS module.exports with object assignment
    module.exports = {
        add: function(a, b) { return a + b; },
        subtract: function(a, b) { return a - b; },
        name: 'Calculator'
    };

    // Error: Using CommonJS exports property assignments
    exports.multiply = function(a, b) {
        return a * b;
    };

    // Error: Combined CommonJS approach
    const utilities = {
        format: (s) => s.trim(),
        validate: (s) => s.length > 0
    };
    module.exports = utilities;

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // Correct: Using ES module export statements
    export function foo() {
        return 'Hello ArkTS';
    }

    // Correct: Using named exports
    export function add(a: number, b: number): number {
        return a + b;
    }

    export function subtract(a: number, b: number): number {
        return a - b;
    }

    export const name = 'Calculator';

    // Correct: Using export with type
    export type MathFunction = (a: number, b: number) => number;

    // Correct: Using default export (when needed)
    export default class Calculator {
        add(a: number, b: number): number {
            return a + b;
        }

        subtract(a: number, b: number): number {
            return a - b;
        }
    }

    // Correct: Export multiple items at once
    function multiply(a: number, b: number): number {
        return a * b;
    }

    function divide(a: number, b: number): number {
        return a / b;
    }

    export { multiply, divide };

.. _R234:

|CB_R| Decorators are not supported
-----------------------------------

|CB_RULE| ``arkts-no-ts-decorators``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: DecoratorsNotSupported

|CB_ERROR|

|LANG| does not support |TS|-like decorators.
When migrating from |TS| to |LANG|, decorators must be refactored by using
alternative patterns such as:
- Class composition and inheritance;
- Higher-order functions;
- Explicit proxying or wrapper methods;
- Direct implementation of cross-cutting concerns.

Note that while |TS|-style decorators are not supported, |LANG| does provide
its own decorator system specifically for UI development (such as
``@Component``, ``@State``, etc.). These ArkTS UI decorators are fundamentally
different as they are a part of the language's UI framework and operate through
compile-time transformations rather than runtime modifications.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    // Error: Class decorator
    function logClass(constructor: Function) {
        console.log(`Class created: ${constructor.name}`);
    }

    @logClass
    class User {
        name: string;
        constructor(name: string) {
            this.name = name;
        }
    }

    // Error: Method decorator
    function logMethod(target: any, propertyKey: string, descriptor: PropertyDescriptor) {
        const originalMethod = descriptor.value;
        descriptor.value = function(...args: any[]) {
            console.log(`Calling method: ${propertyKey}`);
            return originalMethod.apply(this, args);
        };
        return descriptor;
    }

    class Logger {
        @logMethod
        log(message: string) {
            console.log(message);
        }
    }

    // Error: Property decorator
    function format(formatString: string) {
        return function (target: any, propertyKey: string) {
            let value: string;
            Object.defineProperty(target, propertyKey, {
                get: () => formatString.replace("%s", value),
                set: (newValue: string) => { value = newValue },
            });
        };
    }

    class Greeting {
        @format("Hello, %s!")
        name: string;
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    // Instead of class decorators, use composition or inheritance
    class LoggingCapability {
        logCreation(className: string) {
            console.log(`Class created: ${className}`);
        }
    }

    class User extends LoggingCapability {
        name: string;

        constructor(name: string) {
            super();
            this.name = name;
            this.logCreation("User");
        }
    }

    // Instead of method decorators, use higher-order functions or proxy methods
    class Logger {
        log(message: string) {
            console.log(message);
        }

        // Create a logging wrapper method
        logWithTracking(message: string) {
            console.log("Calling log method");
            this.log(message);
        }
    }

    // Instead of property decorators, use explicit getters/setters
    class Greeting {
        private _name: string = "";

        get name(): string {
            return `Hello, ${this._name}!`;
        }

        set name(value: string) {
            this._name = value;
        }
    }

    // If using ArkTS UI decorators, use the supported syntax
    @Component
    struct MyComponent {
        @State count: number = 0;

        build() {
            // UI building code
        }
    }

.. _R236:

|CB_R| Method cannot override a field in interface implemented
--------------------------------------------------------------

|CB_RULE| ``arkts-no-method-overriding-field``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: MethodOverridingField

|CB_ERROR|

Overriding a field with a functional type and *vice versa* is not allowed.

|LANG| enforces strict type consistency between interfaces and their
implementing classes, particularly regarding the distinction between methods
and fields with function types. While both can represent callable entities,
they have different semantics and are not interchangeable.

1. **Method Declaration**: A method is a function that is a part of a class or
interface definition which is defined by using the syntax
``methodName(parameters): returnType {...}``. Methods are invoked by using
the dot notation, and have a fixed implementation.

2. **Function-Typed Field**: A field with a function type (e.g.,
``fieldName: (parameters) => returnType``) is a property that holds a reference
to a function. This function can be reassigned, unlike methods which have a
fixed implementation.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    interface I {
        callback1: () => void;
        callback2(): void;
    }

    class C implements I {
        callback1(): void {}                // Error
        callback2: () => void = () => {};   // Error
    }

|CB_OK|
~~~~~~~

.. code-block:: typescript

    interface I {
        callback1: () => void;
        callback2(): void;
    }

    class C implements I {
        callback1: () => void = () => {};
        callback2(): void {}
    }


.. _R238:

|CB_R| No types based on type ``this``
--------------------------------------

|CB_RULE| ``arkts-no-types-based-on-this-type``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. meta:
    :keywords: NoThisBasedTypes

|CB_ERROR|

No ``this``-based types are allowed.

|LANG| enforces that type ``this`` can be used only as a method return type.
No other types can use ``this`` as their part.

.. note::
   |LANG| can use ``this`` as a keyword (and not as a type) instead of
   a parameter name in the parameter list. Using ``this`` in a *function
   with receiver* is an example of it.

|CB_BAD|
~~~~~~~~

.. code-block:: typescript

    class C {
        f1: this | number = 5
        f2?: this[]
    }
    const c = new C
    c.f2 = [ new C, new C, new C]

|CB_OK|
~~~~~~~

.. code-block:: typescript


    class C {
        foo (): this { return this }
    }

