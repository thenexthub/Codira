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

Classes
=======

|

A class declaration introduces a new type and defines its fields, methods,
and constructors.

In the following example, the class ``Person`` is defined that has fields
'name' and 'surname', a constructor, and the method ``fullName``:

.. code-block:: typescript

    class Person {
        name: string = ""
        surname: string = ""
        constructor (n: string, sn: string) {
            this.name = n
            this.surname = sn
        }
        fullName(): string {
            return this.name + " " + this.surname
        }
    }

After the class is defined, its instances can be created by using
the keyword ``new``:

.. code-block:: typescript

    let p = new Person("John", "Smith")
    console.log(p.fullName())

Similarly, an instance can be created by using object literals:

.. code-block:: typescript

    class Point {
        x: number = 0
        y: number = 0
    }
    let p: Point = {x: 42, y: 42}

|

Fields
------

A field is a variable of some type that is declared directly in a class.
Classes can have instance fields, static fields, or both.

|

Instance Fields
~~~~~~~~~~~~~~~

Instance fields exist on every instance of a class. Each instance has its own
set of instance fields:

.. code-block:: typescript

    class Person {
        name: string = ""
        age: number = 0
        constructor(n: string, a: number) {
            this.name = n
            this.age = a
        }
    }

    let p1 = new Person("Alice", 25)
    let p2 = new Person("Bob", 28)

An instance of the class is used to access an instance field:

.. code-block:: typescript

    p1.name 
    this.name

|

``static`` Fields
~~~~~~~~~~~~~~~~~

The keyword ``static`` is used to declare a field as static. ``Static`` fields
belong to the class itself, and all instances of the class share the
same set of ``static`` fields. Class name is used to access a ``static`` field:

.. code-block:: typescript

    class Person {
        static numberOfPersons = 0
        constructor() {
           // ...
           Person.numberOfPersons++
           // ...
        }
    }

    console.log(Person.numberOfPersons)

|

Field Initializers
~~~~~~~~~~~~~~~~~~

|LANG| requires all fields to be explicitly initialized with some values
either when the field is declared, or in the ``constructor``. It is similar
to the ``strictPropertyInitialization`` mode of the standard |TS|. This
behavior is enforced to minimize the number of unexpected runtime errors
and improve performance.

The following code is error-prone and invalid in |LANG|:

.. code-block:: typescript

    class Person {
        name: string 
           /* The TS compiler automatically sets to undefined if no
              strict checks enabled */


        setName(n:string): void {
            this.name = n
        }

        getName(): string {
            // Return type "string" hides from the developers the fact
            // that name can be undefined. The most correct would be
            // to write the return type as "string | undefined". By doing so
            // we tell the users of our API about all possible return values.
            return this.name
        }
    }

    let jack = new Person()
    // Let's assume that the developer forgets to call setName:
    // jack.setName("Jack")
    console.log(jack.getName().length); // runtime exception: name is undefined

It must look as follows in |LANG|:

.. code-block:: typescript

    class Person {
        name: string = "" // The field is always is initialized

        setName(n:string): void {
            this.name = n
        }

        // The type is always string, no other "hidden options".
        getName(): string {
            return this.name
        }
    }

    let jack = new Person()
    // Let's assume that the developer forgets to call setName:
    // jack.setName("Jack")
    console.log(jack.getName().length); // 0, no runtime error

The example below shows how the |LANG| code behaves if the field ``name``
is ``undefined``:

.. code-block:: typescript

    class Person {
        name ?: string // Default field value is undefined
        // More explicit syntax may also be used:
        // name: string | undefined = undefined

        setName(n:string): void {
            this.name = n
        }

        // Compile-time error:
        // name can be "undefined", so we cannot say to those who use this API
        // that it returns only strings:
        getNameWrong(): string {
            return this.name
        }

        getName(): string | undefined { // Return type matches the type of name
            return this.name
        }
    }

    let jack = new Person()
    // Let's assume that the developer forgets to call setName:
    // jack.setName("Jack")

    // Compile-time(!) error: Compiler suspects that we
    // may possibly access something undefined and won't build the code:
    console.log(jack.getName().length); // The code won't build and run

    console.log(jack.getName()?.length); // Builds ok, no runtime error

|

Getters and Setters
~~~~~~~~~~~~~~~~~~~

Setters and getters can be used to provide controlled access to object
properties.

In the following example, a setter is used to forbid setting invalid
values of the 'age' property:

.. code-block:: typescript

    class Person {
        name: string = ""
        private _age: number = 0
        get age(): number { return this._age }
        set age(x: number) {
            if (x < 0) {
                throw Error("Invalid age argument")
            }
            this._age = x
        }
    }

    let p = new Person()
    console.log (p.age) // 0 will be printed out
    p.age = -42 // Error will be thrown as an attempt to set incorrect age

A class can define a getter, a setter, or both.


|

Methods
-------

A method is a function that belongs to a class.
A class can define instance methods, static methods, or both.
A ``static`` method belongs to the class itself. It can access
only ``static`` fields.
An instance method can access both ``static`` (class) fields
and instance fields, including those private to its class.

|

Instance Methods
~~~~~~~~~~~~~~~~

The example below illustrates how the instance methods work.
The ``calculateArea`` method calculates the area of a rectangle by
multiplying the height by the width:

.. code-block:: typescript

    class Rectangle {
        private height: number = 0
        private width: number = 0
        constructor(height: number, width: number) {
            // ...
        }
        calculateArea(): number {
            return this.height * this.width;
        }
    }

In order to be used, an instance method must be called on an instance of
a class:

.. code-block:: typescript

    let square = new Rectangle(10, 10)
    console.log(square.calculateArea()) // output: 100

|

``static`` Methods
~~~~~~~~~~~~~~~~~~

The keyword ``static`` is used to declare a method as static. A ``static``
method belongs to a class itself, and can access ``static`` fields only.

A ``static`` method defines the common behavior of its entire class.
All instances can access ``static`` methods.

Class name is used to call a ``static`` method:

.. code-block:: typescript

    class Cl {
        static staticMethod(): string {
            return "this is a static method."
        }
    }
    console.log(Cl.staticMethod())

|


Method Overload Signatures
~~~~~~~~~~~~~~~~~~~~~~~~~~

Overload signatures can be written to specify that a method can be called
in different ways. Writing an overload signature means that several method
headers have the same name but different signatures, and are immediately
followed by a single implementation method:

.. code-block:: typescript

    class C {
        foo(): void;            /* 1st signature */
        foo(x: string): void;   /* 2nd signature */
        foo(x?: string): void { /* implementation signature */
            console.log(x)
        }
    }
    let c = new C()
    c.foo()     // ok, 1st signature is used
    c.foo("aa") // ok, 2nd signature is used

An error occurs if two overload signatures have the same name and identical
parameter lists.

|

Inheritance
-----------

A class can extend another class. A class that is being extended by another
class is called '*superclass*'.
A class that extends another class is called '*subclass*'.

A subclass can implement several interfaces by using the
following syntax:

.. code-block:: typescript

    class [extends BaseClassName] [implements listOfInterfaces] {
        // ...
    }

A subclass inherits fields and methods, but not constructors,
from its superclass. It can add its own fields and methods, and override
methods defined by the superclass. It is illustrated in the example below:

.. code-block:: typescript

    class Person {
        name: string = ""
        private _age = 0
        get age(): number {
          return this._age
        }
    }
    class Employee extends Person {
        salary: number = 0
        calculateTaxes(): number {
          return this.salary * 0.42
        }
    }

A class containing an ``implements`` clause must implement all methods
defined in all listed interfaces, except the methods defined by the default
implementation:

.. code-block:: typescript

    interface DateInterface {
        now(): string;
    }
    class MyDate implements DateInterface {
        now(): string {
            // implementation is here
            return "now is now"
        }
    }

|

Access to Super
~~~~~~~~~~~~~~~

The keyword ``super`` allows accessing instance methods and constructors
of a superclass. This access is often used to extend basic
functionality of a subclass with the required behavior that can be taken from
the superclass:

.. code-block:: typescript

    class Rectangle {
        protected height: number = 0
        protected width: number = 0

        constructor (h: number, w: number) {
            this.height = h
            this.width = w
        }

        draw() {
            /* draw bounds */
        }
    }
    class FilledRectangle extends Rectangle {
        color = ""
        constructor (h: number, w: number, c: string) {
            super(h, w) // call of super constructor
            this.color = c
        }

        override draw() {
            super.draw() // call of super methods
            // super.height - can be used here
            /* fill rectangle */
        }
    }

|

Override Methods
~~~~~~~~~~~~~~~~

A subclass can override the implementation of a method defined in its
superclass.
An overridden method can be marked with the keyword ``override`` to improve
readability.
An overridden method must have the same types of parameters, and the same, or
derived, return type as the original method:

.. code-block:: typescript

    class Rectangle {
        // ...
        area(): number {
            // implementation
            return 0
        }
    }
    class Square extends Rectangle {
        private side: number = 0
        override area(): number {
            return this.side * this.side
        }
    }

|

Constructors
------------

A class declaration can contain a constructor that is used to initialize
object state. A constructor is defined as follows:

.. code-block:: typescript

    constructor ([parameters]) {
        // ...
    }

If no constructor is defined, then a default constructor with an empty
parameter list is created automatically, for example:

.. code-block:: typescript

    class Point {
        x: number = 0
        y: number = 0
    }
    let p = new Point()

In this case, the default constructor fills default values of field types
in the instance fields.

|

Constructors in Subclass
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The first statement of a constructor body can use the keyword ``super``
to explicitly call a constructor of the direct superclass:

.. code-block:: typescript

    class Rectangle {
        constructor(width: number, height: number) {
            // ...
        }
    }
    class Square extends Rectangle {
        constructor(side: number) { 
            super(side, side)
        }
    }

If a constructor body does not begin with such an explicit call of a
superclass constructor, then it implicitly begins with the superclass
constructor call ``super()``.

|

Constructor Overload Signatures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Overload signatures can be written to specify that a constructor can be called
in different ways. Writing an overload constructor means that several
constructor headers have the same name but different signatures, and are
immediately followed by a single implementation constructor:

.. code-block:: typescript

    class C {
        constructor()             /* 1st signature */
        constructor(x: string)    /* 2nd signature */
        constructor(x?: string) { /* implementation signature */
            console.log(x)
        }
    }
    let c1 = new C()      // ok, 1st signature is used
    let c2 = new C("abc") // ok, 2nd signature is used

An error occurs if two overload signatures have the same name and identical
parameter lists.

|

Visibility Modifiers
--------------------

Both methods and properties of a class can have visibility modifiers.

Visibility modifiers are as follows:

-  ``private``,
-  ``protected``,
-  ``public``.

The default visibility is ``public``.

|

Public Visibility
~~~~~~~~~~~~~~~~~

The ``public`` members (fields, methods, constructors) of a class are
visible in any program part where their class is visible.

|

Private Visibility
~~~~~~~~~~~~~~~~~~

A ``private`` member cannot be accessed outside the class it is declared in,
for example:

.. code-block:: typescript

    class C {
        public x: string = ""
        private y: string = ""
        set_y (new_y: string) {
            this.y = new_y // ok, as y is accessible within the class itself
        }
    }
    let c = new C()
    c.x = "a" // ok, the field is public
    c.y = "b" // compile-time error: 'y' is not visible

|

Protected Visibility
~~~~~~~~~~~~~~~~~~~~

The modifier ``protected`` acts much like the modifier ``private``, but
the ``protected`` members are also accessible in derived classes, for example:

.. code-block:: typescript

    class Base {
        protected x: string = ""
        private y: string = ""
    }
    class Derived extends Base {
        foo() {
            this.x = "a" // ok, access to protected member
            this.y = "b" // compile-time error, 'y' is not visible, as it is private
        }
    }

|

Object Literals
---------------

An object literal is an expression that can be used to create a class instance,
and provide initial values to instance fields. It can be used instead of the
expression ``new`` as it is more convenient in some cases.

A class composite is written as a comma-separated list of name-value pairs
enclosed in '``{``' and '``}``':

.. code-block:: typescript

    class C {
        n: number = 0
        s: string = ""
    }

    let c: C = {n: 42, s: "foo"}

Due to the static typing of |LANG|, object literals can be used in a
context where class or interface type of an object literal can be
inferred as in the example above.

Other valid cases are illustrated below:

.. code-block:: typescript

    class C {
        n: number = 0
        s: string = ""
    }

    function foo(c: C) {}

    let c: C

    c = {n: 42, s: "foo"}  // type of the variable is used
    foo({n: 42, s: "foo"}) // type of the parameter is used

    function bar(): C {
        return {n: 42, s: "foo"} // return type is used
    }

The type of an array element, or of a class field can also be used as follows:

.. code-block:: typescript
    
    class C {
        n: number = 0
        s: string = ""
    }
    let cc: C[] = [{n: 1, s: "a"}, {n: 2, s: "b"}]
    
|

Object Literals of Record Type
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Generic type *Record<K, V>* is used to map the properties of a type
(Key type) to another type (Value type). A special form of object literal
is used to initialize the value of such a type:

.. code-block:: typescript

    let map: Record<string, number> = {
        "John": 25,
        "Mary": 21,
    }
    
    console.log(map["John"]) // prints 25

Type *K* can be either *string* or *number*, while *V* can be any type:

.. code-block:: typescript

    interface PersonInfo {
        age: number
        salary: number
    }
    let map: Record<string, PersonInfo> = {
        "John": { age: 25, salary: 10},
        "Mary": { age: 21, salary: 20}
    }


|
