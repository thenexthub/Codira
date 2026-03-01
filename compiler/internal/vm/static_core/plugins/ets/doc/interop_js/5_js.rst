..
    Copyright (c) 2025 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

++++++++++
JavaScript
++++++++++

.. _Features JS:

Features JS
###########

.. _Features JS. Functions:

Functions
*********

Description
^^^^^^^^^^^

To define a function in JS you should use key world ``function``.

.. code-block:: javascript
    :linenos:

    // define the function
    function greet() {
        console.log("Hello World!");
    }

To call a previously declared function, you need to write function name followed by parentheses ``()``.

.. code-block:: javascript
    :linenos:

    // call the function
    greet();

When the greet() function is called, the program's control transfers to the function definition.
All the code inside the function is executed (Hello World! is printed).
Then the program control jumps to the next statement after the function call.

Interop rules
^^^^^^^^^^^^^

- When some JS function is passed through interop to ArkTS, the function object is wrapped to ESObject.

- When some parameters are passed in this function, their values will be converted into JS values.

- The function will be executed in the JS environment, and the return value will be passed through interop to ArkTS.

.. code-block:: javascript
    :linenos:

    // file1.js
    function foo(a) {
        return a;
    }

.. code-block:: typescript
    :linenos:

    // file2.ets
    import { foo } from './file1'

    // foo will be passed to ArkTS world by reference
    // 1 will be passed to JS world by copy
    foo(1)
    let s = foo(1) // `s` is ESObject

    function test1(f: ESObject) {
      return f(1) as number
    }

    // runtime check for foo will delay from the moment it is passed in test1
    // to the moment it is called inside test1
    test1(foo);

- ``Function Expressions`` and ``Arrow function`` will work exactly in the same way. When a variable that contaions ``Function Expressions`` or ``Arrow function`` is passed through interop to ArkTS it works the same way as any function call.

.. code-block:: javascript
    :linenos:

    // file1.js
    let square = function(num) {
        return num * num;
    };

    let mul2 = (num) => {
      return num*2;
    };

.. code-block:: typescript
    :linenos:

    // file2.ets
    import { square, mul2 } from './file1'

    let c = square(5) as Number; // `c` is 25
    let d = mul2(5) as Number; // `d` is 10

Default parameters
******************

Description
^^^^^^^^^^^

Default function parameters allow named parameters to be initialized with default values if no value or ``undefined`` is passed.

.. code-block:: javascript
    :linenos:

    function multiply(a, b = 1) {
        return a * b;
    }

    console.log(multiply(5, 2));
    // Expected output: 10

    console.log(multiply(5));
    // Expected output: 5

Interop rules
^^^^^^^^^^^^^

- Default parameters should be stored on in js-function, so in case of interop there is no special rule.

.. code-block:: javascript
    :linenos:

    // file1.js
    function multiply(a, b = 1) {
        return a * b;
    }


.. code-block:: typescript
    :linenos:

    // file2.ets
    import { multiply } from './file1'

    let c = multiply(5) as Number; // `c` is 5
    let d = multiply(5, 2) as Number; // `d` is 10


Arguments
*********

Description
^^^^^^^^^^^

Since ES6, the ``arguments`` object is no longer the only way how to handle variable parameters count.
ES6 introduced a concept called ``rest parameters``.

Here's how the arguments worked:


.. code-block:: javascript
    :linenos:

    function doSomething() {
        arguments[0]; // "A"
        arguments[1]; // "B"
        arguments[2]; // "C"
        arguments.length; // 3
    }

    doSomething("A","B","C");


Arguments limitations: ``Arguments`` object is array-like, not a full-fledged array. It means that array-specific methods are not available for ``Arguments``. For example, you cannot use methods such as ``arguments.sort()``, ``arguments.map()`` or ``arguments.filter()``.
The only property you have is ``length``.

Interop rules
^^^^^^^^^^^^^

- Arguments will be passed by the default way through napi, so in case of interop there is no special rule.

.. code-block:: javascript
    :linenos:

    // file1.js
    function doSomething() {
        arguments[0]; // "A"
        arguments[1]; // "B"
        arguments[2]; // "C"
        arguments.length; // 3
    }


.. code-block:: typescript
    :linenos:

    // file2.ets
    import { doSomething } from './file1'

    doSomething("A","B","C"); // ok

Rest parameters
***************

Description
^^^^^^^^^^^

Rest parameters means that you can put ``...`` before the last parameter in the function.

.. code-block:: javascript
    :linenos:

    function doSomething(first, second, ...rest) {
        console.log(first); // First argument passed to the function
        console.log(second); // Second argument passed to the function
        console.log(rest[0]); // Third argument passed to the function
        console.log(rest[1]); // Fourth argument passed to the function
        // Etc.
    }

You can access the first two named parameters as usual.
However, all the other arguments passed to the function starting with third are automatically collected to an array called as the last parameter (``rest`` here).

If you pass less than three parameters, ``rest`` will be just empty array.

Unlike ``arguments`` object, ``rest parameters`` give you a real array so that you can use all the array-specific methods.
Moreover, unlike ``arguments``, they do work in ``arrow functions``.

.. code-block:: javascript
    :linenos:

    let doSomething = (...rest) => {
        rest[0]; // Can access the first argument
    };

    let doSomething = () => {
        arguments[0]; // Arrow functions don't have arguments
    };

In addition to the advantages above, ``rest parameters`` are part of the function signature. That means that just from the function "header" you can immediately recognize that it uses ``rest parameters`` and therefore accepts variable number of arguments. With ``arguments`` object, there is no such hint.


Rest parameters limitations

1. You can use them max once in a function, multiple rest parameters are not allowed.

.. code-block:: javascript
    :linenos:

    // This is not valid, multiple rest parameters
    function doSomething(first, ...second, ...third) {}

2. You can use rest parameters only as a last parameter of a function:

.. code-block:: javascript
    :linenos:

    // This is not valid, rest parameters not last
    function doSomething(first, ...second, third) {}

Interop rules
^^^^^^^^^^^^^

- Arguments will be passed by the default way through napi, so in case of interop there is no special rule.

.. code-block:: javascript
    :linenos:

    // file1.js
    function doSomething(first, second, ...rest) {
        first; // "A"
        second; // "B"
        rest[0]; // "C"
        rest[1]; // "D"
    }

.. code-block:: typescript
    :linenos:

    // file2.ets
    import { doSomething } from './file1'

    doSomething("A","B","C","D"); // ok

Spread operator(empty)
**********************

Description
^^^^^^^^^^^

Syntactically ``spread operator`` is the same as ``rest parameters ...``, but it works opposite to the ``rest parameters``.
Instead of collecting multiple values in one array, it lets you expand one existing array (or other iterable) into multiple values.

.. code-block:: javascript
    :linenos:

    let numbers = [1, 2, 3];

    // equivalent to
    // console.log(numbers[0], numbers[1], numbers[2])
    console.log(...numbers);

    // Output: 1 2 3


.. code-block:: javascript
    :linenos:

    // Spread operator inside arrays

    // to expand the elements of another array
    let fruits = ["Apple", "Banana", "Cherry"];

    // add fruits array to moreFruits1
    // without using the ... operator
    let moreFruits1 = ["Dragonfruit", fruits, "Elderberry"];

    // spread fruits array within moreFruits2 array
    let moreFruits2 = ["Dragonfruit", ...fruits, "Elderberry"];

    console.log(moreFruits1);
    console.log(moreFruits2);

    // Output: [ 'Dragonfruit', [ 'Apple', 'Banana', 'Cherry' ], 'Elderberry' ]
    //         [ 'Dragonfruit', 'Apple', 'Banana', 'Cherry', 'Elderberry' ]


You can also use the spread operator with object literals:

.. code-block:: javascript
    :linenos:

    // Spread operator with object
    let obj1 = { x : 1, y : 2 };
    let obj2 = { z : 3 };

    // use the spread operator to add
    // members of obj1 and obj2 to obj3
    let obj3 = {...obj1, ...obj2};

    // add obj1 and obj2 without spread operator
    let obj4 = {obj1, obj2};

    console.log("obj3 =", obj3);
    console.log("obj4 =", obj4);

    // Output:
    // obj3 = { x: 1, y: 2, z: 3 }
    // obj4 = { obj1: { x: 1, y: 2 }, obj2: { z: 3 } }


What happens though when you introduce a property with the ``spread operator`` which already exists in the object:

.. code-block:: javascript
    :linenos:

    // Property conflicts
    let firstObject = {a: 1};
    let secondObject = {a: 2};

    let mergedObject = {...firstObject, ...secondObject};
    // a: 2 is after a: 1 so it wins
    console.log(mergedObject); // { a: 2 }


.. code-block:: javascript
    :linenos:

    // Updating immutable objects
    let original = {
        someProperty: "oldValue",
        someOtherProperty: 42
    };

    let updated = {...original, someProperty: "newValue"};
    // updated is now { someProperty: "newValue", someOtherProperty: 42 }



.. code-block:: javascript
    :linenos:

    // Object destructuring
    let myObject = { a: 1, b: 2, c: 3, d: 4};
    let {b, d, ...remaining} = myObject;

    console.log(b); // 2
    console.log(d); // 4
    console.log(remaining); // { a: 1, c: 3 }


.. code-block:: javascript
    :linenos:

    // Spread operator with functions
    let myArray = [1, 2, 3];

    function doSomething(first, second, third) {}

    doSomething(...myArray);
    // Is equivalent to
    doSomething(myArray[0], myArray[1], myArray[2]);

It works with any iterable, not just arrays. For example, using the spread operator with string will disassemble it to the individual characters.

You can combine this with passing individual parameters. Unlike rest parameters, you can use multiple spread operators in the same function call and it does not need to be the last item.

.. code-block:: javascript
    :linenos:

    // All of this is possible
    doSomething(1, ...myArray);
    doSomething(1, ...myArray, 2);
    doSomething(...myArray, ...otherArray);
    doSomething(2, ...myArray, ...otherArray, 3, 7);

Destructing assignment  (empty)
*******************************

Description
^^^^^^^^^^^

Destructuring Assignment is a syntax that allows you to extract data from arrays and objects.

.. code-block:: javascript
    :linenos:

    const user = {firstName: 'Adrian', lastName: 'Mejia'};

    function getFullName({ firstName, lastName }) {
    return `${firstName} ${lastName}`;
    }

    console.log(getFullName(user));
    // Adrian Mejia

Array destructuring  (empty)
****************************

Description
^^^^^^^^^^^

Obtain values from an array and assign values to other variables

.. code-block:: javascript
    :linenos:


    let myArray = [1, 2, 3, 4, 5];
    let [a, b, c, ...d] = myArray;

    console.log(a); // 1
    console.log(b); // 2
    console.log(c); // 3
    console.log(d); // [4, 5]

Exceptions(empty)
*****************

Description
^^^^^^^^^^^

``throw``
``try catch`` statement

Error Handling:

- Native Error Types Used

    - RangeError

    - ReferenceError

    - SyntaxError

    - TypeError

    - URIError

    - AggregateError

    - EvalError

    - InternalError

Including:

- the cause property on Error objects, which can be used to record a causation chain in errors

Interop rules
^^^^^^^^^^^^^

- JS Error and escompat Error classes are mapped as reference proxy-classes
- If JS throws a value which is not an Error instance, the Error is boxed into ESError 2.0 internal class

.. code-block:: javascript
    :linenos:

    // file1.js
    function foo(a) {
      throw new Error();
      return a;
    }

.. code-block:: typescript
    :linenos:

    // file2.ets  ArkTS
    import { foo } from './file1'

    try {
        foo();
    } catch (e) {
        e.message; // ok
    }

Limitations & Solutions
"""""""""""""""""""""""

- If JS throws a value which is not an Error instance, the Error is boxed into ESError 2.0 internal class

.. code-block:: javascript
    :linenos:

    // file1.js
    function foo(a) {
      throw 123;
      return a;
    }

.. code-block:: typescript
    :linenos:

    // file2.ets  ArkTS
    import { foo } from './file1'

    try {
        foo();
    } catch (e) {
        if (e instanceof RewrappedESObjectError) {
          let v = e.getValue() as number; // ok, obtain what's actually thrown
        }
    }


Getter/Setter
*************

Description
^^^^^^^^^^^

Getters and setters allow you to define Object Accessors (Computed Properties).

This example uses a lang property to get the value of the language property:

.. code-block:: javascript
    :linenos:

    // Create an object:
    const person = {
      firstName: "John",
      lastName: "Doe",
      language: "en",
      get lang() {
        return this.language;
      }
    };

    // Display data from the object using a getter:
    document.getElementById("demo").innerHTML = person.lang;

This example uses a lang property to set the value of the language property:

.. code-block:: javascript
    :linenos:

    const person = {
      firstName: "John",
      lastName: "Doe",
      language: "",
      set lang(lang) {
        this.language = lang;
      }
    };

    // Set an object property using a setter:
    person.lang = "en";

    // Display data from the object:
    document.getElementById("demo").innerHTML = person.language;


Interop rules
^^^^^^^^^^^^^

- Accesing to getter/setter will do on JS side, so here should not be any additional side effects or limitations, just the same as for functions.

.. code-block:: javascript
    :linenos:

    // file1.js
    class A {
      get val() { return 42};
      set val(val) { console.log(val)};
    }

    export let a = new A();

.. code-block:: typescript
    :linenos:

    // file2.ets
    import { a } from './file1'

    a.val = 35; // ok

Objects (empty)
***************

Description
^^^^^^^^^^^

JavaScript object is a variable that can store multiple data in key-value pairs.

Syntax to create object:

    .. code-block:: javascript
        :linenos:

        const objectName = {
            key1: value1,
            key2: value2,
            ...,
            keyN: valueN
        };

Access object property:

    * Using dot notation
    * Using braket notation

Modify Object Properties:

    .. code-block:: javascript
        :linenos:

        const person = {
            name: "Bobby",
            hobby: "Dancing",
        };

        // modify property
        person.hobby = "Singing";

        // display the object
        console.log(person);

        // Output: { name: 'Bobby', hobby: 'Singing' }

Add Object Properties:

    .. code-block:: javascript
        :linenos:

        const student = {
            name: "John",
            age: 20,
        };

        // add properties
        student.rollNo = 14;
        student.faculty = "Science";

        // display the object
        console.log(student);

        // Output: { name: 'John', age: 20, rollNo: 14, faculty: 'Science' }

Delete Object Properties:

    .. code-block:: javascript
        :linenos:

        const employee = {
            name: "Tony",
            position: "Officer",
            salary: 30000,
        };

        // delete object property
        delete employee.salary

        // display the object
        console.log(employee);

        // Output: { name: 'Tony', position: 'Officer' }

JS Method  (empty)
******************

Description
^^^^^^^^^^^

A JavaScript method is a function defined within an object.

Using object literal Syntax

.. code-block:: typescript
    :linenos:

    const person = {
        name: 'John',
        age: 25,
        add(a, b) {
            return a + b;
        }
    };

    person.add();

Defining a method in a Class

.. code-block:: typescript
    :linenos:

    class Person {
        constructor(name, age) {
            this.name = name;
            this.age = age;
        }

        add(a, b) {
            return a + b;
        }
    }

    const person = new Person('John', 25);
    person.add();

``this`` key word (empty)
*************************

Description
^^^^^^^^^^^

The ``this`` keyword refers to the context where a piece of code, such as a function's body, is supposed to run. Most typically, it is used in object methods, where this refers to the object that the method is attached to, thus allowing the same method to be reused on different objects.

The value of ``this`` in JavaScript depends on how a function is invoked (runtime binding), not how it is defined. When a regular function is invoked as a method of an object (``obj.method()``), ``this`` points to that object. When invoked as a standalone function (not attached to an object: ``func()``), ``this`` typically refers to the ``global object`` (in ``non-strict mode``) or ``undefined`` (in ``strict mode``).

Constructor (empty)
*******************

Description
^^^^^^^^^^^

Objects are not fundamentally class-based.
Objects may be created in various ways including via a literal notation or via constructors which create objects and then execute code that initializes all or part of them by assigning initial values to their properties. Each constructor is a function that has a property named "prototype" that is used to implement prototype-based inheritance and shared properties.
Objects are created by using constructors in new expressions.

.. code-block:: javascript
    :linenos:

    // constructor function
    function Person () {
      this.name = 'John',
      this.age = 23
    }

    // create an object
    const person = new Person();


Every object created by a constructor has an implicit reference (called the object's prototype) to the value of its constructor's "prototype" property.

Interop rules
^^^^^^^^^^^^^

- Constructor will be passed by the default way through napi, so in case of interop there is no special rule.

.. code-block:: javascript
    :linenos:

    // file1.js
    exprot class Person {
      constructor(name, age) {
        this.name = name;
        this.age = age;
      }
    }
   

.. code-block:: typescript
    :linenos:

    // file2.ets
    import { Person } from './file1'

    let a: Person = new Person('Alice', 30) // ok

Prototype (empty)
*****************

Description
^^^^^^^^^^^

The state and methods are carried by objects, while structure, behaviour, and state are all inherited.

.. code-block:: javascript
    :linenos:

    //prototyping
    let animal = {
        who: () => console.log("animal"),
        say: () => console.log("arr")
    };

    let dog = {
        __proto__: animal,
        say: () => console.log("woof")
    };

    let puppy = {
        __proto__: dog,
        say: () => console.log("wf")
    };

    dog.who(); /* animal */
    dog.say(); /* woof */

    puppy.who(); /* animal */
    puppy.say(); /* wf */

Every object has prototype, if the object does not have the required property, then the search is performed in the object's prototype. If it is not there either, then in the prototype of the prototype, etc.
The function call can be delegated to prototypes located "above".

Including:

- Object.hasOwn, a convenient alternative to Object.prototype.hasOwnProperty.

Import module (empty)
*********************

Description
^^^^^^^^^^^

In JavaScript, you can import modules using the import statement.  This allows you to include functionality from external files or libraries into your code.

.. code-block:: javascript
    :linenos:

    import defaultExport from "module-name";
    import * as name from "module-name";
    import * from 'module-name';
    import { namedExport } from 'module-name';
    import "module-name";

Dynamic import (empty)
**********************

Description
^^^^^^^^^^^

The import declaration syntax (import something from "somewhere") is static and will always result in the imported module being evaluated at load time.
Dynamic imports allow one to circumvent the syntactic rigidity of import declarations and load a module conditionally or on demand.

.. code-block:: javascript
    :linenos:

    import(moduleName)
    import(moduleName, options)

Export (empty)
**************

Description
^^^^^^^^^^^

.. code-block:: javascript
    :linenos:

    export { name1, name2, ..., nameN };
    export default <expression>;
    export * from ...;
    export default function (...) { ... };

.. _Features JS. Classes:

Classes (empty)
***************

Description
^^^^^^^^^^^

Constructor - special method ``constructor``, which is called when the class is initialized with new.

The body of a class is executed in strict mode even without the ``"use strict"`` directive.

A class element can be characterized by three aspects:

    * Kind: Getter, setter, method, or field

    * Location: Static or instance

    * Visibility: Public or private

Parental constructor inherited automatically if the descendant does not have its own method constructor. If the descendant has his own constructor, then to inherit the parent's constructor you need to use ``super()`` with arguments for parent.
The ``extends`` keyword is used in class declarations or class expressions to create a class as a child of another constructor (either a class or a function).
If there is a constructor present in the subclass, it needs to first call super() before using this. The ``super`` keyword can also be used to call corresponding methods of super class.

.. code-block:: javascript
    :linenos:

    class ChildClass extends ParentClass { /* … */ }

Including fetures:
- public and private instance fields
- public and private static fields
- private instance methods and accessors
- private static methods and accessors
- static blocks inside classes, to perform per-class evaluation initialization
- the #x in obj syntax, to test for presence of private fields on objects

Interop rules
^^^^^^^^^^^^^

- Proxing JS class with ESObject.

.. code-block:: javascript
    :linenos:

    // file1.js
    export class A {
      v = 123;
    }

.. code-block:: typescript
    :linenos:

    // file2.ets ArkTS
    import { A } from './file1'

    let val = new A(); // ok, val is ESObject

Iterations (empty)
******************

Description
^^^^^^^^^^^

An ``Iterator`` object is an object that conforms to the iterator protocol by providing a ``next()`` method that returns an iterator result object. All built-in iterators inherit from the Iterator class. The Iterator class provides a [Symbol.iterator]() method that returns the iterator object itself, making the iterator also iterable.
It also provides some helper methods for working with iterators.

* do ... while

* for

* for ... in

.. code-block:: javascript
    :linenos:

    // Syntax
    for (variable in object)
        statement


The loop will iterate over all enumerable properties of the object itself and those the object inherits from its prototype chain (properties of nearer prototypes take precedence over those of prototypes further away from the object in its prototype chain).

Like other looping statements, you can use control flow statements inside statement:

    - ``break`` stops statement execution and goes to the first statement after the loop
    - ``continue`` stops statement execution and goes to the next iteration of the loop


* for..of

* for await ... of (see in asynchronous section)

* while


Interop rules
^^^^^^^^^^^^^

- If try to iterate an object from ArkTS in JS world. The object should in ArkTS world must be iterable(can be used in for-of statements). Otherwise, the runtime will throw an error.

Relational operators: ``<``, ``>``, ``<=``, ``>=``  (empty)
***********************************************************

Description
^^^^^^^^^^^

Operators ``<``, ``>``, ``<=``, ``>=`` are used to compare the order of two operands. If the condition is true, the result is ``true``; otherwise, it is ``false``.

.. code-block:: javascript
    :linenos:

    let foo = 123;
    let bar = 456;

    // Expected output: true
    console.log(foo <= bar);

Interop rules
^^^^^^^^^^^^^

- If one of the operands is from ArkTS, then the type of this operand JS must be primitive. Otherwise, the runtime will throw an error.

.. code-block:: javascript
    :linenos:

    // file1.js
    export function is_greater(a, b) {
        return a > b
    }

.. code-block:: typescript
    :linenos:

    // file2.ets ArkTS
    import { is_greater } from './file1'

    let res = is_greater(2, 1); // ok
    res = is_greater('2', 1);  // ok

    class Foo {
        a: number;
        constructor(arg: number) {
            this.a = arg;
        }
    }
    let a = new Foo(1);
    let res = is_greater(2, a); // error

Relational operators: ``in``  (empty)
*************************************

Description
^^^^^^^^^^^

``${name} in ${object}`` returns the bool value ``true`` if there is a property ``name`` in the object ``${object}`` (or in its prototype chain).
Throw an runtime error if the ``${object}`` is not an object.

.. code-block:: javascript
    :linenos:

    let foo = { bar: "bar_value"};

    console.log("bar" in foo); // true
    console.log(undefined in foo); // false
    // console.log("bar" in "not a object"); // error: not an object
    console.log("length" in new String("it is a object")); // true: it is an object
    console.log(Symbol.iterator in new String("it is a object")) // check the iterator

Relational operators: ``instanceof``  (empty)
*********************************************

Description
^^^^^^^^^^^

Operator ``instanceof`` checks whether an object belongs to a certain class. In other words, object instanceof constructor checks if an object is present constructor.prototype in the prototype chain object.

.. code-block:: javascript
    :linenos:

    function Car(brand, model, year) {
        this.brand = brand;
        this.model = model;
        this.year = year;
    }

    const auto = new Car('Honda', 'Camry', 1998);

    console.log(auto instanceof Car);
    // Expected output: true

    console.log(auto instanceof Object);
    // Expected output: true

Closure (empty)
***************

Closure provides access to the outer scope of a function from inside the inner function, even after the outer function has closed.

Description
^^^^^^^^^^^

.. code-block:: javascript
    :linenos:

    // nested function example

    // outer function
    function greet(name) {

        // inner function
        function displayName() {
            console.log('Hi' + ' ' + name);
        }

        // calling inner function
        displayName();
    }

    // calling outer function
    greet('John'); // Hi John

Interop rules
^^^^^^^^^^^^^

- A JS closure can be called in ArkTS world through Interop.
- A JS closure can captures a variable from ArkTS world.
- GC will handle the variables' lifetime properly.

.. code-block:: javascript
    :linenos:

    // js file
    // handler.js
    let handler = null

    export function initHandler(foo) {
      function my_handler() {
        console.log("in my_handler")
        console.log(foo.bar);  // interop: my_handler captures foo from ArkTS world
      }

      handler = my_handler
    }

    export function getHandler() {
      if (handler === null) {
        throw new Error('Handler is not set')
      }
      return handler
    }

.. code-block:: typescript
    :linenos:

    // arkts file
    // init.ets
    import { initHandler } from './handler'

    class Foo {
      bar: string = ''
    }

    export function init() {
      let foo = new Foo()
      foo.bar = 'this is bar property'
      initHandler(foo)
    }

.. code-block:: typescript
    :linenos:

    // arkts file
    import { init } from './init'
    import { getHandler } from './handler'

    init()
    let handler = getHandler()                
    handler()                                   // interop: closure is called in ArkTS world

Object of primitive types (empty)
*********************************

Description
^^^^^^^^^^^

The primitive types in JavaScript have no methods or properties. 
When accessing a property or method, the primitive value is wrapped into an Object(aka auto-boxing).
And actually the method or property is called on the object, not on the primitive value.

.. code-block:: javascript
    :linenos:

    let str = "string literal";

    // Actually, variable str is auto-boxed into an String Object. 
    // Then String.toUpperCase() is called.
    console.log(str.toUpperCase()); 

The converting rules are:

- null -> Null
- undefined -> Undefined
- boolean -> Boolean
- number -> Number
- bigint -> Bigint
- string -> String
- symbol-> Symbol

The ``typeof`` Operator (empty)
*******************************

Description
^^^^^^^^^^^

The ``typeof`` operator returns a string indicating the type of the operand's value. 
The rules are:

   +--------------------------------------------------------------------------------+-------------+
   | Type                                                                           |    Result   |
   +================================================================================+=============+
   | undefined                                                                      | "undefined" |
   +--------------------------------------------------------------------------------+-------------+
   | null                                                                           |   "object"  |
   +--------------------------------------------------------------------------------+-------------+
   | boolean                                                                        |   "boolean" |
   +--------------------------------------------------------------------------------+-------------+
   | number                                                                         |   "number"  |
   +--------------------------------------------------------------------------------+-------------+
   | bigInt                                                                         |   "bigint"  |
   +--------------------------------------------------------------------------------+-------------+
   | string                                                                         |   "string"  |
   +--------------------------------------------------------------------------------+-------------+
   | symbol                                                                         |   "symbol"  |
   +--------------------------------------------------------------------------------+-------------+
   | Function (implements [[Call]] in ECMA-262 terms; classes are functions as well)|  "function" |
   +--------------------------------------------------------------------------------+-------------+
   | any other object                                                               |   "object"  |
   +--------------------------------------------------------------------------------+-------------+

.. code-block:: javascript
    :linenos:

    let foo = "bar";
    console.log(typeof foo); // "string"

Generators (empty)
******************

Description
^^^^^^^^^^^

Generators can return (``yield``) multiple values, one after another, on-demand.

    - key word ``yeild``, ``function*``, ``yeild* generator``

    - methods:

        - ``generator.next()`` : returns a value of yield
        - ``generator.return()``: returns a value and terminates the generator
        - ``generator.throw()``: throws an error and terminates the generator

``With`` statement (empty)
**************************

Description
^^^^^^^^^^^

Use of the ``with`` statement is not recommended, as it may be the source of confusing bugs and compatibility issues, makes optimization impossible, and is forbidden in ``strict mode``.
The recommended alternative is to assign the object whose properties you want to access to a temporary variable.


The ``debugger`` statement
**********************************

Description
^^^^^^^^^^^

The ``debugger`` statement invokes any available debugging functionality, such as setting a breakpoint.
If no debugging functionality is available, this statement has no effect.

.. code-block:: javascript
    
    :linenos:
    // dummy code
    dubugger; // interrupt when debugging

Interop rules
^^^^^^^^^^^^^
- This statement has not interop with ArkTS.

Proxies (empty)
***************

Description
^^^^^^^^^^^

The ``Proxy`` object enables you to create a proxy for another object, which can intercept and redefine fundamental operations for that object.

You create a ``Proxy`` with two parameters:

- ``target``: the original object which you want to proxy
- ``handler``: an object that defines which operations will be intercepted and how to redefine intercepted operations.

.. code-block:: javascript
    :linenos:

    const target = {
        message1: "hello",
        message2: "everyone",
    };

    const handler2 = {
        get(target, prop, receiver) {
            return "world";
        },
    };

    const proxy2 = new Proxy(target, handler2);

    console.log(proxy2.message1); // world
    console.log(proxy2.message2); // world

The Global Object (empty)
*************************

Description
^^^^^^^^^^^

The ``global object`` in JavaScript is an object which represents the global scope.

The ``globalThis`` global property allows one to access the ``global object`` regardless of the current environment.

Value properties of the ``Global Object``:

    - globalThis

    - Infinity

    - NaN

    - underfined

Other properties of the ``Global Object``:

    - Atomics

    - JSON (see below)

    - Math

    - Reflect

Reflect (empty)
***************

Description
^^^^^^^^^^^

The ``Reflect`` object provides a collection of static functions which have the same names as the ``proxy`` handler methods.
The ``Reflect`` object has a number of methods that allow developers to access and modify the internal state of an object.

.. code-block:: javascript
    :linenos:

    const person = {
        name: 'John Doe'
    };

    Reflect.set(person, 'name', 'Jane Doe');

    console.log(person.name); // 'Jane Doe'


``Reflect.get()``, ``Reflect.set()``, ``Reflect.apply()``, ``Reflect.construct()`` methods

.. _JS Std library:

Symbol (empty)
**************

Description
^^^^^^^^^^^

Symbol is a primitive in JS which stands for a unique symbol(aka symbol value).

.. code-block:: javascript
    
    let s = Symbol("foo");
    let t = Symbol("foo");
    console.log(typeof s);  // "symbol"
    console.log(s == t);    // false
    console.log(s === t);   // false

JS Std library
##############

Arrays
******

Description
^^^^^^^^^^^

The Array object in JavaScript represents a collection of some elements. The elements can be of different types.

.. code-block:: javascript
  :linenos:

  let arr = new Array(1, "2", 3, 4, 5)
  console.log(arr)
  console.log(arr[0])

Interop rules
^^^^^^^^^^^^^

- In JS [] and Array are indistinguishable, so interop rules are the same for both of them
- When JS array is passed through interop to ArkTS, the proxy object is constructed in ArkTS and user can work with the array as if it was passed by reference. So any modification to the array will be reflected in JS

.. code-block:: javascript
  :linenos:

  //file1.js
  export let a = new Array(1, 2, 3, 4, 5)
  export let b = [1, 2, 3, 4 ,5]

.. code-block:: typescript
  :linenos:

  //file2.ets  ArkTS

  import {a, b} from 'file1'
  let val1 = a[0] // ok
  let val2 = b[0] // ok
  let val3 = a.length // ok
  let val4 = b.length // ok
  a.push(6) // ok, will affect original Array
  b.push(6) // ok, will affect original Array

Set
***

Description
^^^^^^^^^^^

The ``Set`` object lets you store unique values of any type, whether primitive values or object references.

Interop rules
^^^^^^^^^^^^^

- When JS set is passed through interop to ArkTS, the proxy object is constructed in ArkTS and user can work with the set as if it was passed by reference. So any modification to the set will be reflected in JS

.. code-block:: javascript
  :linenos:

  //file1.js
  export let mySet = new Set();
  export let obj1 = {};

  mySet.add(1);
  mySet.add(2);
  mySet.add('3');
  mySet.add(obj1);

.. code-block:: javascript
  :linenos:

  //file2.ets ArkTS
  import {mySet, obj1} from 'file1'
  let val1 = mySet.has(1) as boolean        // true
  let val2 = mySet.has(2) as boolean        // true
  let val3 = mySet.has('3') as boolean      // true
  let val4 = mySet.has(obj1) as boolean     // true

  class X {}
  let x = new X()

  let val5 = mySet.has(x) as boolean        // false
  mySet.add(x)                              // ok, will affect original Set
  let val6 = mySet.has(x) as boolean        // true

  let val7 = mySet.delete(2) as boolean     // true, will affect original Set
  let val8 = mySet.has(2) as boolean        // false

Limitations
"""""""""""

- It is not supported to converte JS Set to ArkTS Set.

.. code-block:: javascript
  :linenos:

  //file3.ets ArkTS
  import {mySet} from 'file1'
  mySet as Set<ESObject> // can not be converted, RTE

Map
***

Description
^^^^^^^^^^^

The ``Map`` object holds key-value pairs and remembers the original insertion order of the keys. Any value (both objects and primitive values) may be used as either a key or a value.

Interop rules
^^^^^^^^^^^^^

- When JS map is passed through interop to ArkTS, the proxy object is constructed in ArkTS and user can work with the map as if it was passed by reference. So any modification to the map will be reflected in JS

.. code-block:: javascript
  :linenos:

  //file1.js
  export let myMap1 = new Map();
  export let key = {};

  myMap1.set(1, 1);
  myMap1.set('2', 'hello');
  myMap1.set(key, {'value': '1'});

.. code-block:: javascript
  :linenos:

  //file2.ets ArkTS
  import {myMap1, key} from 'file1'
  let val1 = myMap1.get(1) as number      // ok, val1 is 1
  let val2 = myMap1.get('2') as string    // ok, val2 is 'hello'
  let val3 = myMap1.get(key)              // ok, val3 is ESObject, content is {'value': '1'}
  let val4 = myMap1.size as number        // ok, val4 is 3

  class X {}
  let x = new X();

  myMap1.set(2, 2)  // ok, will affect original Map
  myMap1.set(3, x)  // ok, will affect original Map
  myMap1.set(x, 4)  // ok, will affect original Map

  let val5 = myMap1.size as number    // ok, val5 is 6
  let val6 = myMap1.get(3) as X       // ok, val6 is x
  let val7 = myMap1.get(x) as number  // ok, val7 is 4

  let y = new X()
  let val8 = myMap1.get(y) as number  // key y dose not exist in myMap1, throw RTE


Limitations & Solutions
"""""""""""""""""""""""
- It is not supported to converte JS Map to ArkTS Map.
- On ArkTS side getting a value from map maybe undefined while key does not exist, at this time use ``as`` to convert value will be RTE, to avoid RTE, we can use has mathod to check if a key exist before calling get.

.. code-block:: javascript
  :linenos:

  //file3.ets ArkTS
  import {myMap1} from 'file1'
  myMap1 as Map<ESObject, ESObject>;   // can not be converted, RTE

  let val1 = 0
  if (myMap1.has(5) as boolean) {      // key 5 is not exist
      val1 = myMap1.get(5) as number
  }
  console.log(val1) // val1 is 0

ArrayBuffer (empty)
*******************

ArrayBuffer stands for byte buffer in JavaScript. ArrayBuffer cannot be directly manipulated. Instead, it's needed to create `TypedArray` or `DataView` to write or read the buffer.

.. code-block:: javascript
  :linenos:

  // file1.js
  let buf = new ArrayBuffer(128);
  let bytebuf = new Uint8Array(buf);

  // manipulate bytebuf
  // ...

BigInt64Array (empty)
^^^^^^^^^^^^^^^^^^^^^

BigUint64Array (empty)
^^^^^^^^^^^^^^^^^^^^^^

Float32Array
************
Description
^^^^^^^^^^^

Float32Array is a typed array in JavaScript that represents an array of 32-bit floating-point numbers.

- create instance of Float32Array with length 5

.. code-block:: javascript
    :linenos:

    let float32Array = new Float32Array(5);
    console.log(float32Array); // Float32Array [ 0, 0, 0, 0, 0 ]

- create instance of Float32Array from an array of numbers

.. code-block:: javascript
    :linenos:

    let array = [1.1, 2.2, 3.3, 4.4, 5.5];
    let float32ArrayFromArray = new Float32Array(array);
    console.log(float32ArrayFromArray); // output: Float32Array [ 1.1, 2.2, 3.3, 4.4, 5.5 ]

- use index to access and modify elements in Float32Array

.. code-block:: javascript
    :linenos:

    float32ArrayFromArray[0] = 10.1;
    console.log(float32ArrayFromArray[0]); // output: 10.1

Interop rules
^^^^^^^^^^^^^
- When JS Float32Array is passed through interop to ArkTS, the proxy object is constructed in ArkTS and user can work with the object as if it was passed by reference. So any modification to the object will be reflected in JS.

.. code-block:: javascript
    :linenos:

    // file1.js
    export let arrf32_1 = new Float32Array(2);
    
    let array = [1.1, 2.2, 3.3, 4.4, 5.5];
    export let arrf32_2 = new Float32Array(array);

.. code-block:: typescript
    :linenos:

    // file2.ets  ArkTS
    import { arrf32_1, arrf32_2 } from './file1';

    arrf32_1[0] = 1;    // ok, 'arrf32_1' is [1, 0]
    arrf32_2[5] = 2;    // ok, will be passed to JS, and JS will ignore it

    arrf32_1.set([10, 20]);    // ok, 'arrf32_1' is [10, 20]
    let slicedArray = arrf32_2.slice(1, 4);  // ok, 'slicedArray' is [2.2, 3.3, 4.4] wrapped by ESObject

    console.log(arrf32_2.length as number);  // ok, 5

Float64Array
************

Description
^^^^^^^^^^^
- Float64Arrayis a typed array in JavaScript that represents an array of 64-bit floating-point numbers.
- create instance of Float64Array with length 5

.. code-block:: javascript
    :linenos:

    let float64Array = new Float64Array(5);
    console.log(float64Array); // output: Float64Array [ 0, 0, 0, 0, 0 ]

    // create instance of Float64Array from an array of numbers
    let array = [1.1, 2.2, 3.3, 4.4, 5.5];
    let float64ArrayFromArray = new Float64Array(array);
    console.log(float64ArrayFromArray); // output: Float64Array [ 1.1, 2.2, 3.3, 4.4, 5.5 ]

    // use index to access and modify elements in Float64Array
    float64ArrayFromArray[0] = 10.1;
    console.log(float64ArrayFromArray[0]); // output: 10.1

Interop rules
^^^^^^^^^^^^^

- When a Float64Array object is passed from the JS side to ArkTS through interop, a proxy object is generated in ArkTS.
- When a user interacts with a proxy object, the same operation is fed back to the JS side.

.. code-block:: javascript
    :linenos:

    // file1.js
    export let arrf64_1 = new Float64Array(2);
    
    let array = [1.1, 2.2, 3.3, 4.4, 5.5];
    export let arrf64_2 = new Float64Array(array);

.. code-block:: typescript
    :linenos:

    // file2.ets  ArkTS
    import { arrf64_1, arrf64_2 } from './file1';

    arrf64_1[0] = 1;       // ok, 'arrf64_1' is [1, 1]
    arrf64_1[1] = 2.2;     // ok, 'arrf64_1' is [1, 2.2]

    arrf64_1.set([10, 20]);   // ok, 'arrf64_1' is [10, 20]
    let slicedArray = arrf64_2.slice(1, 4);  // ok, 'slicedArray' is [2.2, 3.3, 4.4] wrapped by ESObject

    console.log(arrf64_2.length as number)   // ok, 5

Int8Array
*********

Description
^^^^^^^^^^^

- Int8Array is a typed array in JavaScript that represents an array of 8-bit signed integers with values ranging from -128 to 127.

.. code-block:: javascript
    :linenos:

    // create Int8Array
    let array = [10, 20, 30, 40, 50];
    let int8ArrayFromArray = new Int8Array(array);

    // Array elements can be accessed and modified by index
    int8ArrayFromArray[2] = 128; // out of range, becomes -128
    console.log(int8ArrayFromArray[2]); // output: -128

    // create Int8Array using existing array
    let array = [10, 20, 30, 40, 50];
    let int8ArrayFromArray = new Int8Array(array);
    console.log(int8ArrayFromArray); // output: Int8Array [ 10, 20, 30, 40, 50 ]

Interop rules
^^^^^^^^^^^^^

- When a Int8Array object is passed from the JS side to ArkTS through interop, a proxy object is generated in ArkTS.
- When a user interacts with a proxy object, the same operation is fed back to the JS side.

.. code-block:: javascript
    :linenos:

    // file1.js
    export let int8arr1 = new Int8Array(3);

    let array = [10, 20, 30, 40, 50];
    export let int8arr2 = new Int8Array(array);

.. code-block:: typescript
    :linenos:

    // file2.ets  ArkTS
    import { int8arr1, int8arr2 } from './file1';

    int8arr1[0] = 1;    // ok, 'int8arr1' is [1, 0, 0]
    int8arr1[1] = 2.2;  // ok, 'int8arr1' is [1, 2, 0]
    int8arr2[2] = 128;  // ok, 'int8arr1' is [1, 2, -128]

    console.log(int8arr1.length as number);  // ok, 3
    console.log(int8arr1.byteLength as number);  // ok, 3

Int16Array
**********

Description
^^^^^^^^^^^

- When a Int16Array object is passed from the JS side to ArkTS through interop, a proxy object is generated in ArkTS.
- When a user interacts with a proxy object, the same operation is fed back to the JS side.

.. code-block:: javascript
    :linenos:
    
    // create Int16Array
    let int16Array = new Int16Array(5);
    console.log(int16Array);            // output: Int16Array [ 0, 0, 0, 0, 0 ]

    // create instance of Int16Array from an array
    let array = [1000, 2000, 3000, 4000, 5000];
    let int16ArrayFromArray = new Int16Array(array);
    console.log(int16ArrayFromArray);   // output: Int16Array [ 1000, 2000, 3000, 4000, 5000 ]

    // use index to access and modify elements in Int16Array
    int16ArrayFromArray[0] = 32768;      // out of range, becomes -32768
    console.log(int16ArrayFromArray[0]); // output: -32768

Interop rules
^^^^^^^^^^^^^

- When a Int16Array object is passed from the JS side to ArkTS through interop, a proxy object is generated in ArkTS.

.. code-block:: javascript
    :linenos:

    // file1.js
    export let int16arr1 = new Int16Array(3);
    export let int16arr2;
    
    let array = [1000, 2000, 3000, 4000, 5000];
    export let int16arr3 = new Int16Array(array);

.. code-block:: typescript
    :linenos:

    // file2.ets  ArkTS
    import { int16arr1, int16arr2, int16arr3 } from './file1';

    int16arr1[0] = 1;        // ok, 'int8arr1' is [1, 0, 0]
    int16arr1[1] = 2.2222;   // ok, 'int8arr1' is [1, 2, 0]
    int16arr1[2] = -32769;   // ok, 'int8arr1' is [1, 32767, 0]
    int16arr1[2] = 'a';      // ok, will be `nan` on JS side
    int16arr1[3] = 128;      // ok, will be passed to JS, and JS will ignore it

Int32Array
**********

Description
^^^^^^^^^^^

- When a Int32Array object is passed from the JS side to ArkTS through interop, a proxy object is generated in ArkTS.
- When a user interacts with a proxy object, the same operation is fed back to the JS side.

.. code-block:: javascript
    :linenos:

    // create Int32Array
    let int32Array = new Int32Array(5);
    console.log(int32Array); // output: Int32Array [ 0, 0, 0, 0, 0 ]

    // create instance of Int32Array from an array
    let array = [1000, 2000, 3000, 4000, 5000];
    let int32ArrayFromArray = new Int32Array(array);
    console.log(int32ArrayFromArray); // output: Int32Array [ 1000, 2000, 3000, 4000, 5000 ]

    // use index to access and modify elements in Int32Array
    int32ArrayFromArray[0] = 2147483648; // out of range，becomes -2147483648
    console.log(int32ArrayFromArray[0]); // output: -2147483648

Interop rules
^^^^^^^^^^^^^

- When a Int32Array object is passed from the JS side to ArkTS through interop, a proxy object is generated in ArkTS.

.. code-block:: javascript
    :linenos:

    // file1.js
    export let int32arr1 = new Int32Array(3);
    export let int32arr2;
    
    let array = [1000, 2000, 3000, 4000, 5000];
    export let int32arr3 = new Int32Array(array);

.. code-block:: typescript
    :linenos:

    // file2.ets  ArkTS
    import { int32arr1, int32arr2, int32arr3 } from './file1';

    int32arr1[0] = 1;        // ok, 'int8arr1' is [1, 0, 0]
    int32arr1[1] = 2.2222;   // ok, 'int8arr1' is [1, 2, 0]
    int32arr1[2] = -32769;   // ok, 'int8arr1' is [1, 32767, 0]
    int32arr1[2] = 'a';      // ok, will be `nan` on JS side
    int32arr1[3] = 128;      // ok, will be passed to JS, and JS will ignore it

    int32arr2 = int32arr1.slice(0, 5);  // ok, out of range will be ignored on JS side

    let arr = new Int32Array(3);
    int32arr2 = Int32Array.from(arr);
    arr = Int32Array.of(int32arr1);    // CTE, static `int32arr2` is not compatibale with dynamic int32arr1

Uint8Array
**********

Description
^^^^^^^^^^^

- Uint8Array typed array represents an array of 8-bit unsigned integers. The contents are initialized to 0 upon creation. Once established, you can access elements in the array using the object's methods or standard array index syntax (i.e., using square brackets).

.. code-block:: javascript
    :linenos:

    // create Uint8Array
    let uint8Array = new Uint8Array(5);
    console.log(uint8Array); // output: Uint8Array [ 0, 0, 0, 0, 0 ]

    // create instance of Uint8Array from an array
    let array = [10, 20, 30, 40, 50];
    let uint8ArrayFromArray = new Uint8Array(array);
    console.log(uint8ArrayFromArray); // output: Uint8Array [ 10, 20, 30, 40, 50 ]

    // use index to access and modify elements in Uint8Array
    uint8ArrayFromArray[0] = 256; // out of range，becomes 255
    console.log(uint8ArrayFromArray[0]); // output: 255

Interop rules
^^^^^^^^^^^^^

- When a Uint8Array object is passed from the JS side to ArkTS through interop, a proxy object is generated in ArkTS.
- When a user interacts with a proxy object, the same operation is fed back to the JS side.

.. code-block:: javascript
    :linenos:

    // file1.js
    export let uint8arr1 = new Uint8Array(3);
    export let uint8arr2;
    
    let array = [10, 20, 30, 40, 50];
    export let uint8arr3 = new Uint8Array(array);

.. code-block:: typescript
    :linenos:

    // file2.ets  ArkTS
    import { uint8arr1, uint8arr2, uint8arr3 } from './file1';

    uint8arr1[0] = 1;        // ok, 'uint8arr1' is [1, 0, 0]
    uint8arr1[1] = 2.2222;   // ok, 'uint8arr1' is [1, 2, 0]
    uint8arr1[2] = -32769;   // ok, 'uint8arr1' is [1, 2, 255]
    uint8arr1[2] = 'a';      // ok, will be `nan` on JS side
    uint8arr1[3] = 128;      // ok, will be passed to JS, and JS will ignore it
    
    uint8arr2 = uint8arr1.slice(0, 5);  // ok, out of range will be ignored on JS side

Uint8ClampedArray
*****************

Description
^^^^^^^^^^^

- The Uint8ClampedArray typed array represents an array of 8-bit unsigned integers with values clamped between 0 and 255. If a specified value is outside this range, it will be adjusted to 0 or 255. If a specified value is not an integer, it will be set to the nearest integer. The array contents are initialized to 0 upon creation. Once established, you can access elements in the array using the object's methods or standard array index syntax (i.e., using square brackets).

.. code-block:: javascript
    :linenos:

    // create Uint8ClampedArray
    let uint8clampedarr1 = new Uint8ClampedArray(5);
    console.log(uint8clampedarr1); // output: Uint32Array [ 0, 0, 0, 0, 0 ]

    // create instance of Uint32Array from an array
    let array = [-100, -10, 3.5, 100, 500];
    let uint8ClampedArrayFromArray = new Uint8ClampedArray(array);
    console.log(uint8ClampedArrayFromArray); // output: uint8ClampedArrayFromArray [ 0, 0, 3, 100, 255 ]

    // use index to access and modify elements in Uint32Array
    uint8ClampedArrayFromArray[0] = 4294967296; // out of range，becomes 255
    console.log(uint8ClampedArrayFromArray[0]); // output: 255

Interop rules
^^^^^^^^^^^^^

- When a Uint8ClampedArray object is passed from the JS side to ArkTS through interop, a proxy object is generated in ArkTS.
- When a user interacts with a proxy object, the same operation is fed back to the JS side.

.. code-block:: javascript
    :linenos:

    // file1.js
    export let uint8clampedarr1 = new Uint8ClampedArray(4);
    export let uint8clampedarr2;
    
    let array = [10, 20, 30, 40, 50];
    export let uint8clampedarr3 = new Uint8ClampedArray(array);

.. code-block:: typescript
    :linenos:

    // file2.ets  ArkTS
    import { uint8clampedarr1, uint8clampedarr2, uint8clampedarr3 } from './file1';
    uint8clampedarr1[0] = 1;        // ok, 'uint8clampedarr1' is [1, 0, 0, 0]
    uint8clampedarr1[1] = 2.2222;   // ok, 'uint8clampedarr1' is [1, 2, 0, 0]
    uint8clampedarr1[2] = -32769;   // ok, 'uint8clampedarr1' is [1, 2, 0, 0]
    uint8clampedarr1[3] = 32769;    // ok, 'uint8clampedarr1' is [1, 2, 0, 255]
    uint8clampedarr1[2] = 'a';      // ok, will be `nan` on JS side
    uint8clampedarr1[3] = 128;      // ok, will be passed to JS, and JS will ignore it
    
    uint8clampedarr2 = uint8clampedarr.slice(0, 5);  // ok, out of range will be ignored on JS side

Uint16Array
***********

Description
^^^^^^^^^^^

- Uint16Array is a typed array in JavaScript that represents an array of 16-bit unsigned  integers with values ranging from 0 to 65535.

.. code-block:: javascript
    :linenos:

    // create Uint16Array
    let uint16Array = new Uint16Array(5);
    console.log(uint16Array); // output: Uint16Array [ 0, 0, 0, 0, 0 ]

    // create instance of Uint16Array from an array
    let array = [1000, 2000, 3000, 4000, 5000];
    let uint16ArrayFromArray = new Uint16Array(array);
    console.log(uint16ArrayFromArray); // ouput: Uint16Array [ 1000, 2000, 3000, 4000, 5000 ]

    // use index to access and modify elements in Uint16Array
    uint16ArrayFromArray[0] = 65536; // out of range，becomes 0
    console.log(uint16ArrayFromArray[0]); // output: 0

Interop rules
^^^^^^^^^^^^^

- When a Uint16Array object is passed from the JS side to ArkTS through interop, a proxy object is generated in ArkTS.
- When a user interacts with a proxy object, the same operation is fed back to the JS side.

.. code-block:: javascript
    :linenos:

    // file1.js
    export let uint16arr1 = new Uint16Array(3);
    export let uint16arr2;
    
    let array = [10, 20, 30, 40, 50];
    export let uint16arr3 = new Uint16Array(array);

.. code-block:: typescript
    :linenos:

    // file2.ets  ArkTS
    import { uint16arr1, uint16arr2, uint16arr3 } from './file1';

    uint16arr1[0] = 1;        // ok, 'uint16arr1' is [1, 0, 0]
    uint16arr1[1] = 2.2222;   // ok, 'uint16arr1' is [1, 2, 0]
    uint16arr1[2] = -32769;   // ok, 'uint16arr1' is [1, 2, 255]
    uint16arr1[2] = 'a';      // ok, will be `nan` on JS side
    uint16arr1[3] = 128;      // ok, will be passed to JS, and JS will ignore it
    
    uint16arr2 = uint16arr.slice(0, 5);  // ok, out of range will be ignored on JS side

Uint32Array
***********

Description
^^^^^^^^^^^

- Uint32Array typed array represents an array of 32-bit unsigned integers.

.. code-block:: javascript
    :linenos:

    let uint32Array = new Uint32Array(5);
    console.log(uint32Array); // output: Uint32Array [ 0, 0, 0, 0, 0 ]

    // create instance of Uint32Array fro an array 
    let array = [1000, 2000, 3000, 4000, 5000];
    let uint32ArrayFromArray = new Uint32Array(array);
    console.log(uint32ArrayFromArray); // output: Uint32Array [ 1000, 2000, 3000, 4000, 5000 ]

    // use index to access and modify elements in Uint32Array
    uint32ArrayFromArray[0] = 4294967296;  // out of range，becomes 0
    console.log(uint32ArrayFromArray[0]);  // output: 0

Interop rules
^^^^^^^^^^^^^

- When a Uint32Array object is passed from the JS side to ArkTS through interop, a proxy object is generated in ArkTS.
- When a user interacts with a proxy object, the same operation is fed back to the JS side.

.. code-block:: javascript
    :linenos:

    // file1.js
    export let uint32arr1 = new Uint32Array(3);
    export let uint32arr2;
    
    let array = [10, 20, 30, 40, 50];
    export let uint32arr3 = new Uint32Array(array);

.. code-block:: typescript
    :linenos:

    // file2.ets  ArkTS
    import { uint32arr1, uint32arr2, uint32arr3 } from './file1';

    uint32arr1[0] = 1;        // ok, 'uint32arr1' is [1, 0, 0]
    uint32arr1[1] = 2.2222;   // ok, 'uint32arr1' is [1, 2, 0]
    uint32arr1[2] = -32769;   // ok, 'uint32arr1' is [1, 2, 255]
    uint32arr1[2] = 'a';      // ok, will be `nan` on JS side
    uint32arr1[3] = 128;      // ok, will be passed to JS, and JS will ignore it
    
    uint32arr2 = uint32arr.slice(0, 5);  // ok, out of range will be ignored on JS side

DataView (empty)
****************

DataView is used to read and write byte buffer.

.. code-block:: javascript
  :linenos:

  // 00 00 00 00 00 00 00 00
  let buf = new ArrayBuffer(8);
  let view = new DataView(buf);
  
  // 00 01 00 00 00 00 00 00
  view.setInt16(0, 1); 
  
  // read as big-endian
  // 00 01 00 00 00 00 00 00
  // ^~~~~~~~~~^
  // |--Int32--|  High --- Low
  console.log(view.getInt32(0));

  // read as little-endian
  // 00 01 00 00 00 00 00 00
  // ^~~~~~~~~~^
  // |--Int32--|  Low --- High
  console.log(view.getInt32(0, true)); // 256

Date (empty)
************

WeakMap (empty)
***************

WeakSet (empty)
***************

BigInt (empty)
**************

Boolean (empty)
***************

DataView (empty)
****************

Date (empty)
************

Error (empty)
*************

EvalError
*********

Description
^^^^^^^^^^^

- EvalError is a built-in error type in JavaScript, typically used to indicate errors that occur when executing code through the eval() function.
- Constructor function, creates a new EvalError object.

.. code-block:: javascript
    :linenos:

    try {
        throw new EvalError("Hello, world!");
    } catch (e) {
        console.log("EvalError:", e.message);
    }

Interop rules
^^^^^^^^^^^^^

- ArkTS does not support the eval() function

AggregateError (empty)
^^^^^^^^^^^^^^^^^^^^^^

URIError
********

Description
^^^^^^^^^^^

- The URIError object is used to represent errors produced by using global URI handling functions incorrectly.
- cry to decode an invalid URI component using decodeURIComponent.

.. code-block:: javascript
    :linenos:

    try {
        decodeURIComponent('%');
    } catch (e) {
        if (e instanceof URIError) {
            console.log("URIError:", e.message);
        } else {
            console.log("Other Error", e.message);
        }
    }

- create a new URIError.

.. code-block:: javascript
    :linenos:

    try {
        throw new URIError("Hello World");
    } catch (e) {
        console.log(e instanceof URIError, e.message); // true Hello World
    }

RangeError (empty)
^^^^^^^^^^^^^^^^^^

ReferenceError (empty)
^^^^^^^^^^^^^^^^^^^^^^

SyntaxError (empty)
^^^^^^^^^^^^^^^^^^^

TypeError (empty)
^^^^^^^^^^^^^^^^^

InternalError (empty)
^^^^^^^^^^^^^^^^^^^^^

FinalizationRegistry
********************

Description
^^^^^^^^^^^

- The FinalizationRegistry is a mechanism in JavaScript that executes a callback function when an object is garbage collected.

.. code-block:: javascript
    :linenos:

    const registry = new FinalizationRegistry((heldValue) => {
        console.log(`The object has been garbage collected: ${heldValue}`);
    });

- create a target object, register and unbind it with the FinalizationRegistry

.. code-block:: javascript
    :linenos:

    const target = {};
    registry.register(target, "target is recycled");

    registry.unregister(target);

Interop rules
^^^^^^^^^^^^^

- When JS FinalizationRegistry object is passed through interop from the JS side to ArkTS, the ESObject is constructed in ArkTS.

.. code-block:: javascript
    :linenos:

    // file1.js
    export const registry = new FinalizationRegistry((heldValue) => {
        console.log(`The object has been garbage collected: ${heldValue}`);
    });

.. code-block:: typescript
    :linenos:

    // file2.ets  ArkTS
    import { registry } from './file1';

    const target = {};
    registry.register(target, "target is recycled");  // ok
    registry.unregister(target);                      // ok

Function (empty)
****************

Description
^^^^^^^^^^^

- 'Function' is a built-in constructor that can be used to create functions dynamically. This feature is not supported.

Math (empty)
************

See :ref:`Async and concurrency features JS`


JSON Data (empty)
*****************

Description
^^^^^^^^^^^

``JSON data`` consists of key/value pairs similar to ``JavaScript object`` properties.

.. code-block:: javascript
    :linenos:

    // JSON data
    "name": "John"

JSON Object (empty)
*******************

Description
^^^^^^^^^^^

Contain multiple key/value pairs:

.. code-block:: javascript
    :linenos:

    // JSON object
    { "name": "John", "age": 22 }

- ``JavaScript Objects``VS ``JSON``

- Converting ``JSON`` to ``JavaScript Object`` : using the built-in ``JSON.parse()`` function

- Converting ``JavaScript Object`` to ``JSON`` : ``JSON.stringify()`` function

JSON Array  (empty)
*******************

Description
^^^^^^^^^^^

Is written inside square brackets ``[ ]``:

.. code-block:: javascript
    :linenos:

    // JSON array
    [ "apple", "mango", "banana"]

    // JSON array containing objects
    [
        { "name": "John", "age": 22 },
        { "name": "Peter", "age": 20 }.
        { "name": "Mark", "age": 23 }
    ]

Regular Expression Liteals (empty)
**********************************

Description
^^^^^^^^^^^

Regular expressions are patterns used to match character combinations in strings. Regular expressions are also objects. These patterns are used with the ``exec()`` and ``test()`` methods of RegExp, and with the ``match()``, ``matchAll()``, ``replace()``, ``replaceAll()``, ``search()``, and ``split()`` methods of String.

Including:

- Regular expression match indices via the /d flag, which provides start and end indices for matched substrings.

Standart functions (empty)
**************************

- decodeURI
- decodeURIComponent
- encodeURI
- encodeURIComponent
- eval
- isFinite
- isNaN
- parseFloat
- parseInt

TODO: More std library entities
*******************************

.. _Async and concurrency features JS:

Async and concurrency features JS (empty)
#########################################

async/await (empty)
*******************

Description
^^^^^^^^^^^

``async`` keyword ensures that the function returns a ``promise``, and wraps non-promises in it.

``await`` keyword works only inside ``async`` functions. It wait until the promise settles and returns its result.

.. code-block:: javascript
    :linenos:

    async function f() {

        let promise = new Promise((resolve, reject) => {
            setTimeout(() => resolve("done!"), 1000)
        });

        let result = await promise; // wait until the promise resolves (*)

        alert(result); // "done!"
    }

    f();


Promise Objects (empty)
***********************

Description
^^^^^^^^^^^

A ``Promise`` is a proxy for a value not necessarily known when the ``promise`` is created. It allows you to associate handlers with an asynchronous action's eventual success value or failure reason. This lets asynchronous methods return values like synchronous methods: instead of immediately returning the final value, the asynchronous method returns a ``promise`` to supply the value at some point in the future.

A ``Promise`` is in one of these states:

- pending: initial state, neither fulfilled nor rejected

- fulfilled: meaning that the operation was completed successfully

- rejected: meaning that the operation failed

setTimeout()  (empty)
*********************

setInterval()  (empty)
**********************

Promise.prototype.finally()  (empty)
************************************

Asynchronous iteration for-await-of  (empty)
********************************************

Description
^^^^^^^^^^^

Allows you to call asynchronous functions that return a promise (or an array with a bunch of promises) in a loop:

.. code-block:: javascript
    :linenos:

    const promises = [
    new Promise(resolve => resolve(1)),
    new Promise(resolve => resolve(2)),
    new Promise(resolve => resolve(3))];

    async function testFunc() {
        for await (const obj of promises) {
            console.log(obj);
        }
    }

    testFunc(); // 1, 2, 3

Atomics (empty)
***************

Description
^^^^^^^^^^^

Unlike most global objects, ``Atomics`` is not a constructor. You cannot use it with the ``new`` operator or invoke the ``Atomics`` object as a function.
All properties and methods of ``Atomics`` are static (just like the ``Math`` object).

The ``Atomics`` namespace object are used with ``SharedArrayBuffer`` and ``ArrayBuffer`` objects.
When memory is shared, multiple threads can read and write the same data in memory. Atomic operations make sure that predictable values are written and read, that operations are finished before the next operation starts and that operations are not interrupted.

    - ``wait()`` and ``notify()`` methods

    The ``wait()`` and ``notify()`` methods are modeled on Linux futexes ("fast user-space mutex") and provide ways for waiting until a certain condition becomes true and are typically used as blocking constructs.

Await (empty)
*************

Description
^^^^^^^^^^^

- Including top-level await, allowing the keyword to be used at the top level of modules

