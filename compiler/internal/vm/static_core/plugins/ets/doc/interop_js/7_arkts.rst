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

++++++
ArkTS
++++++

.. _Features ArkTS:

Features ArkTS
###############

.. _Features ArkTS Functions:

Functions
*********
- When some ArkTS function is passed through interop to JS runtime, the proxy function is constructed in JS and user can work with the function as if it was passed by reference.
- When some parameters are passed in this function, their values will be converted into ArkTS values.
- The function will be executed in the ArkTS environment, and the return value will be passed through interop to JS runtime. Further, some runtime type checks are applied to ArkTS functions.

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  export function foo(val: number) : void;

.. code-block:: javascript
  :linenos:

  // file1.ts
  import {foo} from "file1";
  foo(1); // ok
  foo("123");  // CTE

Limitations
===========
- On JS side function call should match the signature, or will be RTE

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  export function foo(val: number) : void;

.. code-block:: javascript
  :linenos:

  // file1.ts
  import {foo} from "file1";
  import {bar} from "file3";
  foo("1"); // CTE
  foo(); // CTE
  foo(1, 2); // CTE
  foo(1);  // ok

  bar(foo);

.. code-block:: javascript
  :linenos:

  // file3.js
  export function bar(cb) {
    cb(1, 2, 3);  // RTE when passing foo as cb
  }

Solutions
---------
- Arguments can be converted on JS side

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  export function foo(val: number) : void;

.. code-block:: javascript
  :linenos:

  // file1.ts
  import {foo} from "file1";
  import {bar} from "file3";
  foo(Number("1")); // ok

.. code-block:: javascript
  :linenos:

  // file3.js
  export function bar(cb) {
    cb(1);  // ok when passing foo as cb
  }

Exceptions
**********

- Exception objects are converted with common interop rules by proxy wrapper on js side

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
    function foo(a) {
      throw new Error();
      return a;
    }

.. code-block:: javascript
  :linenos:

  // file1.ts
  import {foo} from "file1";
    try {
        foo();
    } catch (e) {
        e.message; // ok
    }

Overloading
***********
- Overloading in ArkTS has more features than ArkTS 1.0. It will be very hard to achieve the semantic of 2.0 overload resolution in interop.

Limitations
===========

- Developer can not use static overloaded fucntios in dynamic source code

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  export function foo(val: number) : void;
  export function foo(val: string) : void

.. code-block:: javascript
  :linenos:

  // file2.ts

  import {foo} from "file1"; // runtime exception for ambiguous import

Solutions
---------

- User can change names of static funtions and do not overloading for them

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  export function fooNum(val: number) : void;
  export function fooStr(val: string): void

.. code-block:: javascript
  :linenos:

  // file2.js
  import {fooInt, fooStr} from "file1"; // ok
  fooNum(1); // ok
  fooStr("hi"); // ok

Overriding(empty)
*****************

Rest/Spread parameters(empty)
*****************************
- (Need to add more cases)ArkTS can pass any count of parameters and types to any ESObject. So no any issues and limitations here.

.. code-block:: javascript
    :linenos:

    // 1.js
    function foo(x, y, z) {
      console.log(x + y + z);
    }

.. code-block:: typescript
    :linenos:

    //  file2.ets  ArkTS
    import { foo } from './1.js'

    let arr = [1, 2, 3];
    foo(...arr);

Imports (empty)
***************

- Dynamic import

Exports (empty)
***************

- Declaration Modules
- Selective Export Directive
- Single Export Directive
- Export Type Directive
- Re-Export Directive

Packages (empty)
****************

Namespace (empty)
*****************

Spread expression (empty)
*************************

Readonly parameters (empty)
***************************

Optional parameters (empty)
***************************

Default parameters (empty)
**************************

Getter/Setter
*************
- Accesing to getter/setter will do on JS side, so here should not be any additinal side effects or limitations, just the same as fo functions.

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  class A {
      get val() { return 42};
      set val(v : number) { console.log(v)};
  }

  export let a = new A();

.. code-block:: javascript
  :linenos:

  // file2.ts

  import {a} from "file1"; // ok
  a.val = 23 ; // ok

.. _Features ArkTS Classes:

Classes
*******
- Proxing ArkTS ``class``.

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  class A {
    val : string = "hi";
  };

  export function foo(a: A) : void;

.. code-block:: javascript
  :linenos:

  // file2.ts
  import { A, foo } from "file2"

  let a = new A();  // ok
  foo(a);  // ok

.. _Features ArkTS Interfaces:

Interfaces (empty)
******************

.. _Features ArkTS Tuple:

Tuple (empty)
*************

- Proxing ArkTS ``tuple``.

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  type Tuple = [int, boolean];
  const etsTuple: Tuple = [1, true];

.. code-block:: javascript
  :linenos:

  // file2.js
  import { etsTuple } from "file2" // intermediate src with decl

  let a = etsTuple[0]; // ok

.. _Features ArkTS Union:

Union (empty)
*************

- Proxing ArkTS ``union``.

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  export let a: number | string = 123;

.. code-block:: javascript
  :linenos:

  // file2.js
  import { a } from "file2" // intermediate src with decl
  let b = a; // ok, b is number

Enumerations (empty)
********************

- Enumeration operations: the method toString()

Literals (empty)
****************

- IntergerLiteral
- FloatLiteral
- BigIntLiteral
- BooleanLiteral
- StringLiteral
- MultilineStringLiteral

Can contain arbitrary text delimited by '`'. Multiline strings can contain any character, except '\'.

- NullLiteral

Denotes a reference without pointing at any entity.

- UnderfinedLiteral

Denotes a reference with a value that is not defined.

- CharLiteral (experimental)

Boxed Types (empty)
*******************

Object Class Type (empty)
*************************

``this`` keyword (empty)
*************************

Class Extension Clause (empty)
******************************

Abstract classes, Final classes (empty)
***************************************

Final Methods (empty)
*********************

Implementation Clause (empty)
*****************************

Native constructors (empty)
***************************

Native methods (empty)
**********************

Native functions (empty)
************************

Interface (empty)
*****************

Class and Interface Inheritance (empty)
***************************************

Type Alias Declaration (empty)
******************************

Generics (empty)
****************

Type parameters (empty)
***********************

Type Arguments (empty)
**********************

Type Parameter Variance (``in`` or ``out`` keywords) (empty)
************************************************************

Implicit/Explicit Generic Instantiations (empty)
************************************************

Partial Utility Type (empty)
****************************

Required Utility Type (empty)
*****************************

Readonly Utility (empty)
************************

Record Utility Type (empty)
***************************

typeOf (empty)
**************

InstanceOf (empty)
******************

Chaining Operator (empty)
*************************

Lambda expression (empty)
*************************

Lambda Expressions with Receiver (empty)
****************************************

Trailing Lambdas (empty)
************************

Array Literals (empty)
**********************

Object Literal (empty)
**********************

Cast Operator (empty)
*********************

Ensure-Not-Nullish Expression (empty)
*************************************

Nullish-Coalescing Expression (empty)
*************************************

Object Class Type (empty)
*************************

Type never (empty)
******************

For of statement and Iterable types (empty)
*******************************************

Ambient declarations (empty)
****************************

Subtyping and Supertyping (empty)
*********************************

Smart Types (empty)
*******************

GUI Structs (empty)
*******************

Builder Function (empty)
************************

Methods Returning this (empty)
******************************

Array Creation Expression (empty)
*********************************

Indexing Expressions and Indexable Types (empty)
************************************************

Callable Types (empty)
**********************
- $_invoke Method
- $_instantiate Method

Functions with Reciever and Accessors with Reciever (empty)
***********************************************************


DynamicObject Type (empty)
**************************

Annotations (empty)
*******************

Exporting and Importing Annotations (empty)
*******************************************

Ambient Annotations (empty)
***************************

.. _ArkTS Std library:

ArkTS Std library
#####################

.. _ArkTS Std library. STDLib Mimic-proxy:

STDLib Mimic-proxy
******************

- Interop runtime may create a mimic-class that inherits some class(standart library) or implements an interface and intercepts all the defined operations so everything is redirected to the original JS Object
- ArkTS Mimic-class is lazily constructed for any 2.0 class or interface if necessary
- ArkTS Mimic-instance wraps the JS original object so it can be used as the 2.0 type
- This feature will be implemented for part of JS Std lib classes, at least for: ``Array``, ``Map``, ``Set``, ``Error``

- In this example ``foo`` is pure static function without any abilities to get ``ESObject`` as parameter. That's why it should be repacked to a special mimic-proxy way.

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  export function foo(a: Array<string>) {
      a.push("aaa"); // ok
  }

  export let a = new A();

.. code-block:: javascript
  :linenos:

  // file1.ts

  import {foo} from "file2"; // ok
  let arr: Array<string> = ["hello", "str"];
  foo(arr); // ok, now arr is ["hello", "str", "aaa"]. But without 'Mimic-proxy' feature we will get RTE here. That's why we need it.

Limitations
===========

- ArkTS Mimic-class will follow strict type checking for elements

.. What about type erasure here???

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  export function foo(a: Array<string>) {
      a.push("aaa"); // ok
      a.at(0); // RTE, type of element [0] is number
  }

  export let a = new A();

.. code-block:: javascript
  :linenos:

  // file1.ts
  import {foo} from "file2"; // ok
  let arr = [1, "hello"];
  foo(arr); // RTE

Solutions
---------

Provide the objects that match the types.

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  export function foo(a: Array<string>) {
      a.push("aaa"); // ok
      a.at(0); // ok, type of element [0] is string
  }

  export let a = new A();

.. code-block:: javascript
  :linenos:

  // file1.ts
  import {foo} from "file2"; // ok
  let arr = ["1", "hello"];
  foo(arr); // ok

Arrays
******

- In ArkTS T[] and Array<T> are same types, so they have same interop rules.
- When an ArkTS is passed through interop to JS runtime, the proxy array object is constructed in JS runtime, and user works with proxy array as if the original ArkTS array was passed by reference, so modifying the proxy array will affect the original ArkTS Array.

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  export let a = new Array<number>(1, 2, 3, 4, 5));

.. code-block:: javascript
  :linenos:

  // file2.ts
  import {a} from "file2"
  let val1 = a[0]; // ok
  let val2 = a.length; // ok
  a.push(6); // ok

Limitations
===========

- There are strict type check during ArkTS runtime when ArkTS array is modified.

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  export let a = new Array<number>(1, 2, 3, 4, 5));

.. code-block:: javascript
  :linenos:

  // file2.ts
  import {a} from "file2"
  let str: number = "hello" as Object as number;  // use trick to avoid compilation type check
  a.push(str); // RTE

Solutions
---------

- Do not violate the types of ArkTS array.

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  export let a = new Array<number>(1, 2, 3, 4, 5));

.. code-block:: javascript
  :linenos:

  // file2.ts
  import {a} from "file2"
  let num: number = 111;
  a.push(num); // ok

ArrayBuffer (empty)
*******************

Atomics (empty)
***************

BigInt (empty)
**************

BigInt64Array (empty)
*********************

BigUint64Array (empty)
**********************

Boolean (empty)
***************

DataView (empty)
****************

Date (empty)
************

Error (empty)
*************

EvalError (empty)
*****************

RangeError (empty)
******************

ReferenceError (empty)
**********************

SyntaxError (empty)
*******************

URIError (empty)
****************

FinalizationRegistry<T> (empty)
*******************************

Float32Array (empty)
********************

Float64Array (empty)
********************

Int16Array (empty)
******************

Int32Array (empty)
******************

Int8Array (empty)
*****************

Uint16Array (empty)
*******************

Uint32Array (empty)
*******************

Uint8Array (empty)
******************

Uint8ClampedArray (empty)
*************************

IterableIterator<T> (empty)
***************************

Iterator<T, TReturn, TNext> (empty)
***********************************

IteratorResult<V> (empty)
*************************

JSON (empty)
************

Map<K, V> (empty)
*****************

Math (empty)
************

Number (empty)
**************

Object (empty)
**************

Promise<T>
**********

(see :ref:`Async and concurrency features ArkTS. Promise <T> Class`)

RegExp (empty)
**************

RegExpExecArray (empty)
***********************

Set<K> (empty)
**************

String (empty)
**************

WeakMap<K, V> (empty)
*********************

WeakSet<K> (empty)
******************

WeakKey (empty)
***************

Standart Functions 
******************
- decodeURIComponent
- encodeURI
- encodeURIComponent

.. _Async and concurrency features ArkTS:

Async and concurrency features ArkTS
#####################################

Promise<T> Class (empty)
************************

.. _Async and concurrency features ArkTS. Promise <T> Class:

Async Functions and Methods (empty)
***********************************

Basic Coroutines (empty)
************************
- awaitExpression

Communication Channels (empty)
******************************
