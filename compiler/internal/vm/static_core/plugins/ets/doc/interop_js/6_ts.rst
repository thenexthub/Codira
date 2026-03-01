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
TypeScript
++++++++++

.. _Features TS:

Features TS
###########

Functions (empty)
*****************

https://www.typescriptlang.org/docs/handbook/2/everyday-types.html#functions

Description
^^^^^^^^^^^

Functions are the primary means of passing data in TypeScript. TypeScript allows you to specify the input parameter type and return value type of a function.

- Parameter Type Annotations
  When declaring a function, you can add a parameter type to the end of each input parameter to declare the type of the input parameter.

.. code-block:: typescript
    :linenos:

    //Parameter type annotation
    function foo(num: number) {
      console.log('this number: ' + num);
    }

- Return Type Annotations
  Add a return type annotations. The return type annotations is added after the parameter list.

.. code-block:: typescript
    :linenos:

    //Return type annotation
    function foo(): number {
      return 1;
    }

- Anonymous Functions
  Use function keyword or arrows to represent anonymous functions.

.. code-block:: typescript
    :linenos:

    const numbers = [1, 2, 3];

    // Contextual typing for function
    numbers.forEach(function (s) {
      console.log(s);
    });

    // Contextual typing also applies to arrow functions
    numbers.forEach((s) => {
      console.log(s);
    });

Object literal (empty)
**********************

https://www.typescriptlang.org/docs/handbook/2/everyday-types.html#object-types

Description
^^^^^^^^^^^

To define an object, we simply list all its properties and types.You can use , or ; to separate the properties.

.. code-block:: typescript
    :linenos:

    function funcPerson(person: {name: string; age: number}) {
      console.log('personName: ' + person.name);
      console.log('personAge: ' + person.age);
    }

Object types can also specify whether or not their properties are optional. Add a ? after the property name, indicates an optional property

.. code-block:: typescript
    :linenos:

    function funcPerson(person: {name: string; age?: number}) {
      console.log('personName: ' + person.name);
      console.log('personAge: ' + person.age);
    }

If you access a property that does not exist, you will get an undefined instead of a runtime error. Therefore, when you read from an optional property, you must check whether it exists before using it.

.. code-block:: typescript
    :linenos:

    function funcFullName(fullName: { firstName: string; lastName?: string }) {
      console.log(fullName.lastName.toUpperCase()); //error 'fullName.lastName' is possibly 'undefined'.
      if (fullName.lastName !== undefined) {
        console.log(fullName.lastName.toUpperCase()); //ok
      }
      // Safer syntax:
      console.log(fullName.lastName?.toUpperCase()); //undefined
    }

Union (empty)
*************

https://www.typescriptlang.org/docs/handbook/2/everyday-types.html#union-types

Description
^^^^^^^^^^^

A union type is a combination of at least two different types, and it can be the value of any of these types. We refer to each of these types as the union's members.

.. code-block:: typescript
    :linenos:

    function printCode(code: number | string) {
      console.log('this code: ' + code)
    }

    printCode(111) //ok
    printCode('222') //ok
    printCode({ code: 333 }) //error

If you want to do something about a union type, you must provide an action that works for each type. For example, if you have an union number | string, you can't use methods that are only available on string.

.. code-block:: typescript
    :linenos:

    function split(code: number | string) {
      console.log('split this code: ' + code.substring(0, 5)); //CTE
    }

You can use "if else" to solve this problem, taking different actions on different types.

.. code-block:: typescript
    :linenos:

    function split(code: number | string) {
      if (typeof code === 'string') {
        // In this branch, code is of type 'string'
        console.log('split this code: ' + code.substring(0, 1));
      } else {
        // Here, code is of type 'number'
        console.log('this code: ' + code);
      }
    }

Type Aliases (empty)
********************

https://www.typescriptlang.org/docs/handbook/2/everyday-types.html#type-aliases

Description
^^^^^^^^^^^

The type alias can be the name of any type, including numeric and reference types, as well as custom types. A type alias acts as a new name only. It neither changes the original type meaning nor introduces a new type. The syntax for a type alias is:

.. code-block:: typescript
    :linenos:

    type newString = string;

    function inputString(s: string) : newString {
      return s;
    }

    let newInput = inputString('s');
    console.log(newInput); //ok, print s
    newInput = 'new Input';
    console.log(newInput); //ok, print new Input


custom type:

.. code-block:: typescript
    :linenos:

    type Code = {
      letter: string;
      no: number;
    };

    function printCode(pc: Code) {
      console.log('code is: ' + pc.letter + pc.no);
    }

    printCode({letter: 'R', no: 10000}); //ok, print code is: R10000


union type:

.. code-block:: typescript
    :linenos:

    type code = string | number;

Interfaces (empty)
******************

https://www.typescriptlang.org/docs/handbook/2/everyday-types.html#interfaces

Description
^^^^^^^^^^^

An interface declaration specifies a new named reference type:

.. code-block:: typescript
    :linenos:

    interface Person {
      name: string;
      age: number;
    }

    function funcPerson(p: Person) {
      console.log('person name: ' + p.name);
      console.log('person age: ' + p.age);
    }

    funcPerson({name: 'John', age: 23});

Type Assertions (empty)
***********************

https://www.typescriptlang.org/docs/handbook/2/everyday-types.html#type-assertions

Description
^^^^^^^^^^^

Type assertions allow developers to explicitly specify the type of a value, which is useful for addressing type system limitations. There are two frms of type assertion, angle bracket syntax and 'as' syntax.

.. code-block:: typescript
    :linenos:

    let someValue: any = "this is a string";
    let strLength: number = (<string>someValue).length;
    console.log(strLength); // print 16


    let variable: any = "this is a string";
    let length: number = (variable as string).length;
    console.log(length); // print 16

Literal type string (empty)
***************************

https://www.typescriptlang.org/docs/handbook/2/everyday-types.html#literal-types

Description
^^^^^^^^^^^

Literal type string is a type that consists of a specific string value. This means that a variable or parameter of this type must be exactly equal to the specified string value at compile time.

.. code-block:: typescript
    :linenos:

    function changeColor(color: "red" | "green" | "blue") {
        console.log("new Color: ", color);
    }

    changeColor("red"); //ok
    changeColor("green"); //ok
    changeColor("blue"); //ok
    changeColor("yellow"); //CTE

Literal type number (empty)
***************************

https://www.typescriptlang.org/docs/handbook/2/everyday-types.html#literal-types

Description
^^^^^^^^^^^

Literal type numbers in TypeScript are a feature that allows specific numeric values to be used as types. This means that a variable or parameter of this type must exactly match the specified numeric value at compile time.

.. code-block:: typescript
    :linenos:

    function checkStatus(code: 200 | 404 | 500) {
      if (code === 200) {
        console.log('OK');
      } else if (code === 404) {
        console.log('Not Found');
      } else if (code === 500) {
        console.log('Internal Server Error');
      }
    }
    checkStatus(200); //ok
    checkStatus(404); //ok
    checkStatus(500); //ok
    checkStatus(300); //CTE


Literal type bigint (empty)
***************************

https://www.typescriptlang.org/docs/handbook/2/everyday-types.html#literal-types

Description
^^^^^^^^^^^

Literal type bigint can handle arbitrarily large integers, limited only by the available memory. Literal type bigint provides a way to represent whole numbers larger than 2^53 - 1, which is the largest number JavaScript can reliably represent with the Number primitive.

.. code-block:: typescript
    :linenos:

let b: 10n = 10n;

Interop rules
^^^^^^^^^^^^^

- No interop rules, ArkTS2.0 not support.

Null (empty)
************

Description
^^^^^^^^^^^

In TypeScript, null is a special literal value that represents the absence or lack of a value. Unlike JavaScript, TypeScript provides stricter type-checking mechanisms, allowing developers to explicitly specify whether variables can contain null values.
you can use union types (like string | null) to specify that a variable can be either a string or null.

.. code-block:: typescript
    :linenos:

    let person: string | null;
    person = 'John'; //ok
    person = null; //ok

When using the strictNullChecks compiler option, TypeScript will more strictly check the use of null, preventing them from being assigned to unintended types.

.. code-block:: typescript
    :linenos:

    let age: number;
    age = 25;
    if (age == 25) {
      age = null; // Error, if strictNullChecks is enabled
    }

Undefined (empty)
*****************

Description
^^^^^^^^^^^

Undefined is a special literal value that represents the absence or lack of a value.  It is similar to null but has different semantics and usage in the language.Undefined is a primitive value in JavaScript, indicating that a variable has been declared but not yet assigned a value.
In TypeScript, you can use union types (like string | undefined) to specify that a variable can be either a string or undefined.

.. code-block:: typescript
    :linenos:

    let person: string | undefined;
    person = 'John'; //ok
    person = undefined; //ok

When using the strictNullChecks compiler option, TypeScript will more strictly check the use of undefined, preventing them from being assigned to unintended types.

.. code-block:: typescript
    :linenos:

    let age: number;
    age = 25;
    if (age == 25) {
      age = undefined; // Error, if strictNullChecks is enabled
    }

Unknown (empty)
***************

Description
^^^^^^^^^^^

The unknown type can hold any value, just like the any type. However, unlike any type, it does not allow any operations on it without first narrowing its type through type assertions or type guards.
This makes unknown a safer alternative to any because it prevents accidental use of unknown values, which can lead to runtime errors.

.. code-block:: typescript
    :linenos:

    let value: unknown;
    value = 42;
    console.log(value); //ok print 42

    value = { name: 'John' };
    console.log(value); //ok print { 'name': 'John' }

    value = 'Hello';
    console.log(value); //ok print Hello

    value.toUpperCase(); //CTE

    if (typeof value === 'string') {
      console.log(value.toUpperCase()); //ok print HELLO
    }

Never (empty)
*************

Description
^^^^^^^^^^^

The never type is a special type that represents values that never occur. It is commonly used in functions that either throw an exception, enter an infinite loop, or have unreachable code paths.

.. code-block:: typescript
    :linenos:

    function throwError(message: string): never {
        throw new Error(message);
    }

    throwError("This is an error message"); //RTE

    function processValue(value: string | number): void {
        if (typeof value === "string") {
            console.log("Processing string:", value);
        } else if (typeof value === "number") {
            console.log("Processing number:", value);
        } else {
            const check: never = value; //This line of code will not be executed
        }
    }

Void (empty)
************

Description
^^^^^^^^^^^

The void type is used to indicate that a function does not return any value. It is commonly used in functions that perform actions but do not need to return a result.

.. code-block:: typescript
    :linenos:

    function message(): void {
      console.log('This is my message');
    }

Enums (empty)
*************

https://www.typescriptlang.org/docs/handbook/2/everyday-types.html#enums

Description
^^^^^^^^^^^

An enum is a special data type used to define a set of named constants.
Enums help organize and understand constant values in the code, enhancing readability and maintainability.
They are particularly useful when dealing with fixed sets of values like days of the week, directions, or status codes.

Characteristics:

Members can have explicit values assigned.
Default values start at 0 and increment by 1 if not specified.
Enum members are immutable once defined.
Enums support reverse mapping, allowing access to names via values.

Numeric enums:

.. code-block:: typescript
    :linenos:

    enum Color {
      Red,
      Green,
      Blue
    }

    console.log(Color.Red); //0
    console.log(Color.Green); //1
    console.log(Color.Blue); //2

We can also initializing enum values with 1

.. code-block:: typescript
    :linenos:

    enum Color {
      Red = 1,
      Green,
      Blue
    }

    console.log(Color.Red); //1
    console.log(Color.Green); //2
    console.log(Color.Blue); //3


String enums:

.. code-block:: typescript
    :linenos:

    enum UserRole {
      Admin = 'admin',
      Editor = 'editor',
      Viewer = 'viewer',
      Guest = 'guest'
    }

    console.log(UserRole.Admin); //admin

Bigint (empty)
**************

https://www.typescriptlang.org/docs/handbook/2/everyday-types.html#less-common-primitives

Description
^^^^^^^^^^^

Bigint is a data type used to represent arbitrarily large integers.The bigint type allows you to work with integers that are larger than the safe integer limit (2^53 - 1) of the number type.
Bigint supports most arithmetic operations such as addition, subtraction, and multiplication. However, it cannot be directly mixed with number types without explicit conversion.

.. code-block:: typescript
    :linenos:

    let a: bigint = BigInt(1234567890123456789012345678901234567890n); //BigInt function
    let b: bigint = 9876543210987654321098765432109876543210n; // BigInt literal
    let sum: bigint = a + b;
    console.log(sum); //11111111101111111110111111111011111111100

Symbol (empty)
**************

https://www.typescriptlang.org/docs/handbook/2/everyday-types.html#less-common-primitives

Narrowing (empty)
*****************

Description
^^^^^^^^^^^

Narrowing can be used to narrow the type of a variable in a conditional block.

.. code-block:: typescript
    :linenos:
    
    function foo(bar: number | string) {
        if(typeof(bar) == "number"){
            console.log(bar.toFixed(2)); // asumeing that bar is number type
        } else {
            console.log(bar.charAt(0)); // asumeing that bar is string type 
        }
    }

https://www.typescriptlang.org/docs/handbook/2/narrowing.html

Function Type Expressions
*************************

Description
^^^^^^^^^^^

- A function type expression describes a function type, with it's parameters and return type.

.. code-block:: typescript
    :linenos:
  
    // (x: number) => number is a function type expression, 
    // it describes a function that takes an int and returns an int
    function foo(f: (x: number) => number): number { 
        return f(1) + 123;
    }

https://www.typescriptlang.org/docs/handbook/2/functions.html#function-type-expressions

Call Signatures
***************

Description
^^^^^^^^^^^

- A call signature describes a callable function with properties.

.. code-block:: typescript
  :linenos:

  type LogHandler = {
    name_str: string
    (content: string): void
  }

  function log(content: string, handler: LogHandler) {
    console.log("use handler: {handler.name}")
    handler(content)
  }

  function log_handler(content: string) {
    console.log(content)
  }

  log_handler.name_str = "log handler"
  log("log test", log_handler)

https://www.typescriptlang.org/docs/handbook/2/functions.html#call-signatures

Interop rules
=============

- A Call Signature will be mappped into ESObject when interoping with ArkTS.

.. code-block:: typescript
  :linenos:

  // file1.ets
  export type LogHandler = {
    name_str: string
    (content: string): void
  }

  export function my_log_handler(content: string) {
    console.log(content)
  }

  log_handler.name_str = "MY_LOG_HANDLER"

.. code-block:: typescript
  :linenos:

  // file2.ets
  import { log, my_log_handler } from 'file1'

  let log_handler: LogHandler = my_log_handler       // OK: Call Signature `LogHandler` will be mappped into ESObject
  console.log(typeof log_handler)                    // OK: ESObject
  log_handler("log test", log_handler)               // OK: log test
  console.log(log_handler.name_str)                  // OK: MY_LOG_HANDLER

Construct Signatures
********************

Description
^^^^^^^^^^^

- A construct signature describes a callable function which can be call by ``new`` operator.

.. code-block:: typescript
  :linenos:

  class Foo {
    bar: number = 0

    constructor(bar: number) {
      this.bar = bar
    }
  }

  type Constructor = {
    new (bar: number): Foo
  }

  let MyFooConstructor: Constructor = Foo
  let foo = new MyFooConstructor(123)
  console.log(foo.bar); // 123

Interop rules
=============

- A construct signature will be mappped into ESObject when interoping with ArkTS.

.. code-block:: typescript
  :linenos:

  // file1.ets
  export class Foo {
    bar: number = 0

    constructor(bar: number) {
      this.bar = bar
    }
  }

  export type FooConstructor = {
      new (bar: number): Foo
  }

  export let my_foo_ctor: FooConstructor = Foo


.. code-block:: typescript
    :linenos:

    // file2.ets
    import { Foo, FooConstructor, my_foo_ctor } from 'file1'

    let foo_ctor: FooConstructor = Foo         // OK: Construct signature `FooConstructor` will be mappped into ESObject
    console.log(typeof foo_ctor)               // OK: ESObject
    let foo = new foo_ctor(123)
    console.log(foo.bar)                       // OK: 123

    let foo2 = new my_foo_ctor(123)            // OK: Construct signature Object `my_foo_ctor` will be mappped into ESObject


Generic Functions (empty)
*************************

https://www.typescriptlang.org/docs/handbook/2/functions.html#construct-signatures

Optional Parameters (empty)
***************************

https://www.typescriptlang.org/docs/handbook/2/functions.html#optional-parameters

Function Overloads (empty)
**************************

https://www.typescriptlang.org/docs/handbook/2/functions.html#function-overloads

Global type Function (empty)
****************************

https://www.typescriptlang.org/docs/handbook/2/functions.html#declaring-this-in-a-function

Rest Parameters (empty)
***********************

https://www.typescriptlang.org/docs/handbook/2/functions.html#rest-parameters-and-arguments

Parameter Destructuring (empty)
*******************************

https://www.typescriptlang.org/docs/handbook/2/functions.html#parameter-destructuring

Assignability of Functions (empty)
**********************************

https://www.typescriptlang.org/docs/handbook/2/functions.html#assignability-of-functions

Object Types (empty)
********************

https://www.typescriptlang.org/docs/handbook/2/objects.html

Optional Properties (empty)
***************************

https://www.typescriptlang.org/docs/handbook/2/objects.html#optional-properties

Readonly Properties (empty)
***************************

https://www.typescriptlang.org/docs/handbook/2/objects.html#readonly-properties
https://www.typescriptlang.org/docs/handbook/2/classes.html#readonly

Index Signatures (empty)
************************

https://www.typescriptlang.org/docs/handbook/2/objects.html#index-signatures
https://www.typescriptlang.org/docs/handbook/2/classes.html#index-signatures

Excess Property Checks (empty)
******************************

https://www.typescriptlang.org/docs/handbook/2/objects.html#excess-property-checks

Extending Types (empty)
***********************

https://www.typescriptlang.org/docs/handbook/2/objects.html#extending-types

Intersection Types (empty)
**************************

https://www.typescriptlang.org/docs/handbook/2/objects.html#intersection-types

Generic (empty)
***************

https://www.typescriptlang.org/docs/handbook/2/objects.html#generic-object-types
https://www.typescriptlang.org/docs/handbook/2/generics.html

Keyof Type Operator (empty)
***************************

https://www.typescriptlang.org/docs/handbook/2/keyof-types.html

Typeof Type Operator (empty)
****************************

https://www.typescriptlang.org/docs/handbook/2/typeof-types.html

Indexed Access Types (empty)
****************************

https://www.typescriptlang.org/docs/handbook/2/indexed-access-types.html

Conditional Types (empty)
*************************

https://www.typescriptlang.org/docs/handbook/2/conditional-types.html

Mapped Types (empty)
********************

https://www.typescriptlang.org/docs/handbook/2/mapped-types.html

Template Literal Types (empty)
******************************

https://www.typescriptlang.org/docs/handbook/2/template-literal-types.html

Classes (empty)
***************

- Fields
- Constructors
- Methods
- Getters / Setters
- Index Signatures

https://www.typescriptlang.org/docs/handbook/2/classes.html#class-members

Getters/Setters (empty)
^^^^^^^^^^^^^^^^^^^^^^^

https://www.typescriptlang.org/docs/handbook/2/classes.html#getters--setters

Class Heritage: extends (empty)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

https://www.typescriptlang.org/docs/handbook/2/classes.html#extends-clauses

Class Heritage: implements (empty)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

https://www.typescriptlang.org/docs/handbook/2/classes.html#implements-clauses

Class Member Visibility (empty)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- public
- protected
- private

https://www.typescriptlang.org/docs/handbook/2/classes.html#member-visibility

Static Members (empty)
^^^^^^^^^^^^^^^^^^^^^^

https://www.typescriptlang.org/docs/handbook/2/classes.html#static-members

static Blocks in Classes (empty)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

https://www.typescriptlang.org/docs/handbook/2/classes.html#static-blocks-in-classes

Generic Classes (empty)
^^^^^^^^^^^^^^^^^^^^^^^

https://www.typescriptlang.org/docs/handbook/2/classes.html#generic-classes

this at Runtime in Classes (empty)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

https://www.typescriptlang.org/docs/handbook/2/classes.html#this-at-runtime-in-classes

this Types (empty)
^^^^^^^^^^^^^^^^^^

https://www.typescriptlang.org/docs/handbook/2/classes.html#this-types

Parameter Properties (empty)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

https://www.typescriptlang.org/docs/handbook/2/classes.html#parameter-properties

Class Expressions (empty)
^^^^^^^^^^^^^^^^^^^^^^^^^

https://www.typescriptlang.org/docs/handbook/2/classes.html#class-expressions

Constructor Signatures (empty)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

https://www.typescriptlang.org/docs/handbook/2/classes.html#constructor-signatures

abstract Classes and Members (empty)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

https://www.typescriptlang.org/docs/handbook/2/classes.html#abstract-classes-and-members

Relationships Between Classes (empty)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

https://www.typescriptlang.org/docs/handbook/2/classes.html#relationships-between-classes

Modules (empty)
***************

https://www.typescriptlang.org/docs/handbook/2/modules.html

Partial<Type> (empty)
*********************

https://www.typescriptlang.org/docs/handbook/utility-types.html#partialtype

Required<Type> (empty)
**********************

https://www.typescriptlang.org/docs/handbook/utility-types.html#requiredtype

Readonly<Type> (empty)
**********************

https://www.typescriptlang.org/docs/handbook/utility-types.html#readonlytype

Record<Keys, Type> (empty)
**************************

https://www.typescriptlang.org/docs/handbook/utility-types.html#recordkeys-type

Pick<Type, Keys> (empty)
************************

https://www.typescriptlang.org/docs/handbook/utility-types.html#picktype-keys
Description
^^^^^^^^^^^

- Pick is a built-in tool type in TypeScript that allows to select a set of attributes from an object type to create a new object type.

The syntax is as follows:

.. code-block:: typescript
    :linenos:

    type Pick<T, K extends keyof T> = {
        [P in K]: T[P];
    };

- For example:

.. code-block:: typescript
    :linenos:

    interface Person {
        name: string;
        age: number;
        address: string;
    }

    type PartialPerson = Pick<Person, "name" | "age">;

    const person: PartialPerson = {
        name: "Alice",
        age: 25
    };

Omit<Type, Keys> (empty)
************************

https://www.typescriptlang.org/docs/handbook/utility-types.html#omittype-keys

Description
^^^^^^^^^^^

- Omit is a built-in tool type in TypeScript that excludes a set of properties from an object type to create a new object type.

The syntax is as follows:

.. code-block:: typescript
    :linenos:

    type Omit<T, K extends keyof any> = Pick<T, Exclude<keyof T, K>>;

- For example:

.. code-block:: typescript
    :linenos:

    interface Person {
        name: string;
        age: number;
        address: string;
        phoneNumber: string;
    }

    type PersonWithoutContactInfo = Omit<Person, 'address' | 'phoneNumber'>;
    const personWithoutContactInfo: PersonWithoutContactInfo = {
        name: 'Jack',
        age: 30
    };

Exclude<UnionType, ExcludedMembers> (empty)
*******************************************

https://www.typescriptlang.org/docs/handbook/utility-types.html#excludeuniontype-excludedmembers

Description
^^^^^^^^^^^

- Exclude is a built-in utility type in TypeScript that excludes members from one Union Type to create a new Union Type.
- When TS Object is passed through interop to ArkTS2.0, the ESObject is constructed in ArkTS2.0, and user works with ESObject as if the original TS Object was passed by reference, so modifying the ESObject will affect the original TS Object.

The syntax is as follows:

.. code-block:: typescript
    :linenos:

    type Exclude<T, U> = T extends U ? never : T

- For example:

.. code-block:: typescript
    :linenos:

    type A = 'name' | 'age' | 'address' | 'phoneNumber';
    type B = 'address' | 'phoneNumber';

    type Result = Exclude<A, B>;

    let value: Result;
    value = 'name';        // ok
    value = 'age';         // ok
    value = 'address';     // error, address not in Result
    value = 'phoneNumber'; // error, phoneNumber not in Result

Extract<Type, Union> (empty)
****************************

https://www.typescriptlang.org/docs/handbook/utility-types.html#extracttype-union

Description
^^^^^^^^^^^

- Extract is a built-in utility type in TypeScript that is used to extract members from a Union Type that can be assigned to another Union Type to create a new Union Type.

The syntax is as follows:

.. code-block:: typescript
    :linenos:

    type Extract<T, U> = T extends U ? T : never

- For example:

.. code-block:: typescript
    :linenos:

    type A = 'name' | 'age' | 'address' | 'phoneNumber';
    type B = 'address' | 'phoneNumber' | 'email';

    type Result = Extract<A, B>;

    let value: Result;
    value = 'name';        // error, name not in Result
    value = 'age';         // error, age  not in Result
    value = 'address';     // ok
    value = 'phoneNumber'; // ok

NonNullable<Type> (empty)
*************************

https://www.typescriptlang.org/docs/handbook/utility-types.html#nonnullabletype

Description
^^^^^^^^^^^

- NonNullable is a built-in utility type in TypeScript that excludes null and undefined from a Union Type, creating a new Union Type.

The syntax is as follows:

.. code-block:: typescript
    :linenos:

    type NonNullable<T> = T extends null | undefined ? never : T

- For example:

.. code-block:: typescript
    :linenos:

    type A = string | number | null | undefined;

    type Result = NonNullable<A>;

    let value: Result;
    value = 'hello';       // ok
    value = 42;            // ok
    value = null;          // error
    value = undefined;     // error

Parameters<Type> (empty)
************************

https://www.typescriptlang.org/docs/handbook/utility-types.html#parameterstype

Description
^^^^^^^^^^^

- Parameters is a built-in tool type in TypeScript that takes all the parameter types of a function type and returns them as a tuple.

The syntax is as follows:

.. code-block:: typescript
    :linenos:

    type Parameters<T extends (...args: any) => any> = T extends (...args: infer P) => any ? P : never;

- For example:

.. code-block:: typescript
    :linenos:

    type Func = (name: string, age: number) => void;

    type FuncParams = Parameters<Func>;

    let params: FuncParams;
    params = ['zhangsan', 30];  // ok
    params = ['lisi'];          // error, the second parameter is missing
    params = [30, 'wangwu'];    // error, the order and type of parameters do not match

ConstructorParameters<Type> (empty)
***********************************

https://www.typescriptlang.org/docs/handbook/utility-types.html#constructorparameterstype

Description
^^^^^^^^^^^

- Constructs a tuple or array type from the types of a constructor function type. It produces a tuple type with all the parameter types (or the type never if Type is not a function).

The syntax is as follows:

.. code-block:: typescript
    :linenos:

    type ConstructorParameters<T extends new (...args: any) => any> = T extends new (...args: infer P) => any ? P : never;

For example:

.. code-block:: typescript
    :linenos:

    class Car {
        constructor(public brand: string, public model: string, public year: number) {}
    }

    type CarParams = ConstructorParameters<typeof Car>;

    const carArgs: CarParams = ["Toyota", "Corolla", 2020];
    const car = new Car(...carArgs);

    console.log(car); // output: Car { brand: 'Toyota', model: 'Corolla', year: 2020 }


ReturnType<Type> (empty)
************************

https://www.typescriptlang.org/docs/handbook/utility-types.html#returntypetype

Description
^^^^^^^^^^^
- ReturnType is a built-in tool type in TypeScript that extracts the return type of a function type.

.. code-block:: typescript
    :linenos:

    type ReturnType<T extends (...args: any) => any> = T extends (...args: any) => infer R ? R : any;


- For example:

.. code-block:: typescript
    :linenos:

    function greet(name: string, age: number): string {
        return `Hello, ${name}! You are ${age} years old.`;
    }

    type GreetReturn = ReturnType<typeof greet>;

    let result: GreetReturn;
    result = greet('Tom', 30);  // ok


InstanceType<Type> (empty)
**************************
https://www.typescriptlang.org/docs/handbook/utility-types.html#instancetypetype
Description
^^^^^^^^^^^
- InstanceType is a built-in tool type in TypeScript that extracts instance types of constructor types.
The syntax is as follows:

.. code-block:: typescript
    :linenos:

    type InstanceType<T extends abstract new (...args: any) => any> = T extends abstract new (...args: any) => infer R ? R : any;


- For example:

.. code-block:: typescript
    :linenos:

    class Person {
        name: string;
        age: number;

        constructor(name: string, age: number) {
            this.name = name;
            this.age = age;
        }

        greet(): string {
            return `Hello, ${this.name}! You are ${this.age} years old.`;
        }
    }

    type PersonInstance = InstanceType<typeof Person>;

    let person: PersonInstance;
    person = new Person('Bob', 30);    // ok


NoInfer<Type> (empty)
*********************
https://www.typescriptlang.org/docs/handbook/utility-types.html#noinfertype
Description
^^^^^^^^^^^

- NoInfer is not a built-in utility type for TypeScript, but rather a utility type that prevents TypeScript from automatically inferring types in certain situations.

.. code-block:: typescript
    :linenos:

    type NoInfer<T> = T & { [K in keyof T]: T[K] };

For example:

.. code-block:: typescript
    :linenos:

    function createValue<T>(
        defaultValue: NoInfer<T>,
        callback: (value: T) => void
    ): void {
        callback(defaultValue);
    }

    createValue(42, (value) => {
        console.log(value.toFixed(2));    // output: 42.00
    });

    createValue(42, (value: string) => {
        console.log(value.toUpperCase()); // error: type mismatch
    });

ThisParameterType<Type> (empty)
*******************************
https://www.typescriptlang.org/docs/handbook/utility-types.html#thisparametertypetype
Description
^^^^^^^^^^^

- ThisParameterType is a utility type in TypeScript that extracts the type of the this parameter of a function.
- It is unknown if the function type has no this parameter.

The syntax is as follows:

.. code-block:: typescript
    :linenos:

    type ThisParameterType<T> = T extends (this: infer U, ...args: any[]) => any ? U : unknown;

For example:

.. code-block:: typescript
    :linenos:

    function greet(this: { name: string }, message: string) {
        console.log(`${message}, ${this.name}!`);
    }

    type GreetThisType = ThisParameterType<typeof greet>;

    const context: GreetThisType = { name: "Alice" };
    greet.call(context, "Hello");   // Output: Hello, Alice!

OmitThisParameter<Type> (empty)
*******************************
https://www.typescriptlang.org/docs/handbook/utility-types.html#omitthisparametertype
Description
^^^^^^^^^^^

- OmitThisParameter is a utility type in TypeScript that removes this parameter from a function type.
- It returns a new function type that does not contain this parameter. If the original type does not have this parameter, the original type is returned.

The syntax is as follows:

.. code-block:: typescript
    :linenos:

    type OmitThisParameter<T> = unknown extends ThisParameterType<T> ? T : T extends (...args: infer A) => infer R ? (...args: A) => R : T;

For example:

.. code-block:: typescript
    :linenos:

    function greet(this: { name: string }, message: string) {
        console.log(`${message}, ${this.name}!`);
    }

    type GreetWithoutThis = OmitThisParameter<typeof greet>;

    const greetWithoutThis: GreetWithoutThis = (message: string) => {
        greet.call({ name: "Alice" }, message);
    };

    greetWithoutThis("Hello"); // Output: Hello, Alice!

    greet("Hi"); // Error: The 'this' context is not correctly bound


ThisType<Type> (empty)
**********************
https://www.typescriptlang.org/docs/handbook/utility-types.html#thistypetype
Description
^^^^^^^^^^^

- ThisType is a tool type in TypeScript that explicitly specifies the type of this in an object or function.

the syntax is as follows:

.. code-block:: typescript
    :linenos:

    type ThisType<T> = T;

For example:

.. code-block:: typescript
    :linenos:

    type Context = {
        name: string;
        greet: () => void;
    };

    const obj: ThisType<Context> = {
        name: "Alice",
        greet() {
            console.log(`Hello, ${this.name}!`); // `this` is inferred as Context
        },
    };

    obj.greet(); // Output: Hello, Alice!

    const wrongObj: ThisType<Context> = {
        name: "Bob",
        greet() {
            console.log(`Hi, ${this.age}!`); // Error: Property 'age' does not exist on type 'Context'
        },
    };

Intrinsic String Manipulation Types (empty)
*******************************************
https://www.typescriptlang.org/docs/handbook/utility-types.html#intrinsic-string-manipulation-types

Description
^^^^^^^^^^^

Uppercase<StringType>
=====================

- Convert string literal types to uppercase.

For example:

.. code-block:: typescript
    :linenos:

    type Greeting = "hello, world";
    type Shout = Uppercase<Greeting>; // "HELLO, WORLD"

Lowercase<StringType>
=====================

- Convert string literal types to lowercase.

For example:

.. code-block:: typescript
    :linenos:

    type Greeting = "HELLO, WORLD";
    type Shout = Lowercase<Greeting>; // "hello, world"

Capitalize<StringType>
======================

- Converts the first letter of a string literal type to uppercase.

For example:

.. code-block:: typescript
    :linenos:

    type Greeting = "hello, world";
    type Shout = Capitalize<Greeting>; // "Hello, world"

Uncapitalize<StringType>
========================

- Converts the first letter of a string literal type to lowercase.

.. code-block:: typescript
    :linenos:

    type Greeting = "Hello, world";
    type Shout = Uncapitalize<Greeting>; // "hello, world"

Decorators (empty)
******************

https://www.typescriptlang.org/docs/handbook/decorators.html

- Decorators
- Decorator Factories
- Decorator Composition
- Decorator Evaluation
- Class Decorators
- Method Decorators
- Accessor Decorators
- Property Decorators
- Parameter Decorators
- Metadata

Declaration Merging (empty)
***************************

https://www.typescriptlang.org/docs/handbook/declaration-merging.html

- Basic Concepts
- Merging Interfaces
- Merging Namespaces
- Merging Namespaces with Classes, Functions, and Enums
- Merging Namespaces with Classes
- Disallowed Merges
- Module Augmentation
- Global augmentation

Enums (empty)
*************

https://www.typescriptlang.org/docs/handbook/enums.html

- Numeric enums
- String enums
- Heterogeneous enums
- Computed and constant members
- Union enums and enum member types
- Enums at runtime
- Enums at compile time
- Reverse mappings
- const enums
- Ambient enums
- Objects vs Enums

Iterators and Generators (empty)
********************************

https://www.typescriptlang.org/docs/handbook/iterators-and-generators.html

- Iterable interface
- for..of statements
- for..of vs. for..in statements
- Code generation

JSX (empty)
***********

https://www.typescriptlang.org/docs/handbook/jsx.html

- The as operator
- Type Checking
- The JSX namespace
- Intrinsic elements
- Value-based elements
- Attribute type checking
- Children Type Checking
- The JSX result type
- The JSX function return type
- Embedding Expressions
- React integration
- Configuring JSX

Mixins (empty)
**************

https://www.typescriptlang.org/docs/handbook/mixins.html

Namespaces (empty)
******************

https://www.typescriptlang.org/docs/handbook/namespaces.html

- Validators in a single file
- Namespacing
- Namespaced Validators
- Splitting Across Files
- Multi-file namespaces
- Aliases
- Working with Other JavaScript Libraries
- Ambient Namespaces

Namespaces and Modules (empty)
******************************

https://www.typescriptlang.org/docs/handbook/namespaces-and-modules.html

- Using Modules
- Using Namespaces
- Pitfalls of Namespaces and Modules
- Needless Namespacing
- Trade-offs of Modules

Symbols
*******

Description
^^^^^^^^^^^

Symbol is a primitive date type which stand for a unique symbol(aka symbol value).

.. code-block:: typescript
    :linenos:

    let sym = Symbol();
    console.log(typeof(sym)) // symbol

https://www.typescriptlang.org/docs/handbook/symbols.html

- Symbol.asyncIterator
- Symbol.hasInstance
- Symbol.isConcatSpreadable
- Symbol.iterator
- Symbol.match
- Symbol.replace
- Symbol.search
- Symbol.species
- Symbol.split
- Symbol.toPrimitive
- Symbol.toStringTag
- Symbol.unscopables

Interop rules
^^^^^^^^^^^^^

- Primitive type ``symbol`` and Object ``Symbol`` are not supported by ArkTS standard library.
- A Symbol Object from TypeScript will be converted to an ESObject when interoping with ArkTS.

.. code-block:: typescript
  :linenos:

  // file1.ts
  export let sym = Symbol()

.. code-block:: typescript
  :linenos:

  // file2.ets
  import { sym } from 'file1'
  console.log(typeof sym) // ESObject
  console.log(sym.foobar) // undefined

Triple-Slash Directives (empty)
*******************************

https://www.typescriptlang.org/docs/handbook/triple-slash-directives.html

Type Compatibility (empty)
**************************

https://www.typescriptlang.org/docs/handbook/type-compatibility.html

Type Inference (empty)
**********************

https://www.typescriptlang.org/docs/handbook/type-inference.html

Variable Declaration (empty)
****************************

https://www.typescriptlang.org/docs/handbook/variable-declarations.html

TS Std library (empty)
######################

Array (empty)
*************

https://www.typescriptlang.org/docs/handbook/2/everyday-types.html#arrays
https://www.typescriptlang.org/docs/handbook/2/objects.html#the-array-type

ReadonlyArray (empty)
*********************

https://www.typescriptlang.org/docs/handbook/2/objects.html#the-readonlyarray-type

Tuple (empty)
*************

https://www.typescriptlang.org/docs/handbook/2/objects.html#tuple-types

Readonly Tuple (empty)
**********************

https://www.typescriptlang.org/docs/handbook/2/objects.html#readonly-tuple-types

Async and concurrency features TS (empty)
#########################################

Awaited<Type> (empty)
*********************

https://www.typescriptlang.org/docs/handbook/utility-types.html#awaitedtype

