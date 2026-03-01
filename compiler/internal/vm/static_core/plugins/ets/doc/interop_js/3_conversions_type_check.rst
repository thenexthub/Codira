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

.. _Typecheck conversion rules:

++++++++++++++++++++++++++
Typecheck conversion rules
++++++++++++++++++++++++++

Typecheck performs only for ArkTS/TS files. Let us describe the relationships:

1. ArkTS to TS type conversion
2. TS to ArkTS type conversion

.. _Typecheck conversion rules. Primitive values:

Typecheck conversion rules: Primitive values
--------------------------------------------

ArkTS to TS type conversion
==========================================

+-------------------------+-------------------------+
| ArkTS                   | TS                      |
+=========================+=========================+
| ``number/Number``       | ``number``              |
+-------------------------+-------------------------+
| ``byte/Byte``           | ``number``              |
+-------------------------+-------------------------+
| ``short/Short``         | ``number``              |
+-------------------------+-------------------------+
| ``int/Int``             | ``number``              |
+-------------------------+-------------------------+
| ``long/Long``           | ``number``              |
+-------------------------+-------------------------+
| ``float/Float``         | ``number``              |
+-------------------------+-------------------------+
| ``double/Double``       | ``number``              |
+-------------------------+-------------------------+
| ``char/Char``           | ``string``              |
+-------------------------+-------------------------+
| ``boolean/Boolean``     | ``boolean``             |
+-------------------------+-------------------------+
| ``string/String``       | ``string``              |
+-------------------------+-------------------------+
| ``bigint/BigInt``       | ``bigint``              |
+-------------------------+-------------------------+
| ``enum``                | ``enum``                |
+-------------------------+-------------------------+
| ``literal type string`` | ``literal type string`` |
+-------------------------+-------------------------+
| ``undefined``           | ``undefined``           |
+-------------------------+-------------------------+
| ``null``                | ``null``                |
+-------------------------+-------------------------+
| ``ESObject``            | ``any``                 |
+-------------------------+-------------------------+
| ``void(return type)``   | ``void``                |
+-------------------------+-------------------------+
| ``never``               | ``never``               |
+-------------------------+-------------------------+

TS to ArkTS type conversion
============================

+--------------------------+-------------------------+
| TS                       | ArkTS                   |
+==========================+=========================+
| ``number/Number``        | ``number``              |
+--------------------------+-------------------------+
| ``boolean/Boolean``      | ``boolean``             |
+--------------------------+-------------------------+
| ``string/String``        | ``string``              |
+--------------------------+-------------------------+
| ``bigint/BigInt``        | ``bigint``              |
+--------------------------+-------------------------+
| ``enum``                 | ``enum``                |
+--------------------------+-------------------------+
| ``literal type string``  | ``literal type string`` |
+--------------------------+-------------------------+
| ``literal type number``  | ``number``              |
+--------------------------+-------------------------+
| ``literal type bigint``  | ``bigint``              |
+--------------------------+-------------------------+
| ``undefined``            | ``undefined``           |
+--------------------------+-------------------------+
| ``null``                 | ``null``                |
+--------------------------+-------------------------+
| ``unknown``              | ``ESObject``            |
+--------------------------+-------------------------+
| ``ESObject``             | ``ESObject``            |
+--------------------------+-------------------------+
| ``void(variable type)``  | ``undefined``           |
+--------------------------+-------------------------+
| ``void(return type)``    | ``void``                |
+--------------------------+-------------------------+
| ``never``                | ``never``               |
+--------------------------+-------------------------+

.. _Typecheck conversion rules. Reference values:

Typecheck conversion rules: Reference values
--------------------------------------------

For each type, inside of any part of reference type will be used transformation
according its rules. For example if some class includes two feilds, each field will be
converted according its type conversion rules. This conversion will be marked in
tables below by function ``f()``. (e.g. ``f(Number)`` = ``Number`` ).

This is just general information, more detailed information and examples can be checked in
proper sections: :ref:`Features TS`, :ref:`Features ArkTS`

.. list-table::
  :align: center
  :header-rows: 1
  :widths: 20 40 40

  * - **Type**
    - **TS**
    - **ArkTS**
  * - | Interface
    - | Interface X {
      |   p1: T1;
      |   p2: T2;
      | }
    - | Interface X {
      |   p1: f(T1);
      |   p2: f(T2);
      | }
  * - | function
    - | function foo(a1: T1, a2: T2) : T3
    - | function foo(a1: f(T1), a2: f(T2)) : f(T3)
  * - | Arrow function
    - | (a1: T1, a2: T2) => T3
    - | (a1: f(T1), a2: f(T2)) => f(T3)
  * - | Class
    - | class X {
      |   p1: T1;
      |   p2: T2;
      | }
    - | class X {
      |   p1: f(T1);
      |   p2: f(T2);
      | }
  * - | Union
    - | T1 | T2
    - | f(T1)) | f(T2)
  * - | Utility Types
    - | Record<K, V>
    - | Record<f(K), f(V)>
  * - | Supported standard libriary
    - | Array<T>
    - | Array<f(T)>
  * - | Unsupported types in ArkTS
    - | SharedArrayBuffer<T>
      | Function
    - | ESObject
      | ESObject

Interfaces:

.. code-block:: typescript
  :linenos:

  // file1.ts TS
  export type MyString = string;
  export type MyCallBack = (x: number) => void;
  export interface MyInface {
      data: number
  }
  export class MyClass implements MyInface {
      data: number = 0;
  }
  export type MyFunction = Function;

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  import {MyString, MyCallBack, MyInface, MyClass, MyFunction} from "file1";

  let a: MyString = "123"; // OK
  let b: MyCallBack = (x: number) => {}; // OK
  let c: MyInface = new MyClass(); // OK
  let d: MyClass = new MyClass(); // OK
  let e: MyFunction = 0;  // OK, `Function` missing in ArkTS2.0 so it is ESObject
  let f: MyInface = {data: 0};  // OK

  function foo(x: MyString, y: MyCallBack, z: MyInface, t: MyClass) {}
  foo(a, b, c, d); // OK

  function bar(u: string, v: (x: number) => void) {}
  bar(a, b);  // CTE, need to add expilcit cast to static types
  bar(a as string, b as (x: number) => void);  // ok

Function call:

.. code-block:: typescript
  :linenos:

  // file1.ts TS
  // The parameter type is primitive
  export function foo(a1: string, a2: number) {}

  //The parameter type is a reference type
  export class MyClass {}
  export function bar(a1: MyClass) {}
  export function runCallBack(cb: (msg: string | number) => void) {
      cb("123");
      cb(123);
  }
  export function testMyFunction(x: Function) {}

  export interface MyInface {
      data: number;
  }
  export function testMyInterface(iface: MyInface) {}
  export let obj: MyInface = {data: 0};

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  import {foo, MyClass, bar, runCallBack, testMyFunction, testMyInterface, obj} from "file1"
  foo(); // CTE, parameter type does not match
  foo(1, 2); // CTE, parameter type does not match
  foo("1", 2);  // OK

  bar(null); // CTE, parameter type does not match
  bar(new MyClass()); // OK

  runCallBack(); // CTE, parameter type does not match
  runCallBack((msg: number) =>{}); // CTE, parameter type does not match
  runCallBack((msg: string) =>{}); // CTE, parameter type does not match
  runCallBack((msg: string | number) => {});  // OK

  testMyFunction (new MyClass()); // OK, because the parameter type is mapped to ESObject

  testMyInterface(obj); // OK
  testMyInterface ({data:0}); // CTE, cannot create object of type ESObject

Property access

.. code-block:: typescript
  :linenos:

  // file1.ts TS
  interface MyInface {
      name: string;
  }

  export class MyClass {
      age: number = 0;
      info: MyInface = {name: ""};
      id: Function = () => {};
  }

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  import {MyClass} from "file1"

  let a = new MyClass();

  a.age = 1; // OK
  let b = a.age; //OK
  a.agg; // CTE

  a.info.name = "Allen"; // OK
  console.log(a.info.name);  // OK
  a.info.NAME ; // CTE

  let c = a.id;  // OK
  console.log(a.id);  // OK
  a.id =123; // OK, because the id type is mapped to ESObject

Method call

.. code-block:: typescript
  :linenos:

  // file1.ts TS
  export class MyClass {
      foo(a: number): void {}
      cb: (c: string) => void = (c: string) => {}
      test(t: Function): void {}
  }

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  import {MyClass} from "file1"

  let a = new MyClass();

  a.foo("123"); // CTE
  a.foo(123);  // OK

  a.cb("123");  // OK
  a.cb(123); // CTE

  a.test("123"); // OK, because the parameter type is mapped to ESObject
  a.test(123); // OK, because the parameter type is mapped to ESObject

Index access

.. code-block:: typescript
  :linenos:

  // file1.ts TS
  export interface MyInface {
      data: number;
  }
  export class MyClass implements MyInface {
      data: number = 0;
      name: string = "";
  }
  export let xx: Array<number> = [0, 1, 2];
  export let yy: Array<Function>;
  export let zz: Array<MyInface>;
  export let tt: Array<MyClass>;

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  import {xx, yy} from "file1";

  xx[3] = 3  // OK
  xx[4] = "4"; // CTE

  yy[0] =3; // OK, because the element type is mapped to ESObject
  yy[1] = "5"; // OK, because the element type is mapped to ESObject

  zz[0] ={data:0}; // OK

  tt[0] = new MyClass();  // OK
  tt[1] = {data:0}; // CTE
  tt[2] = {data:0, name: "12"}; // ok

Instantiation

.. code-block:: typescript
  :linenos:

  // file1.ts TS
  // The constructor parameter type is primitive type
  export class A {
      constructor(arg: number) {}
  }
  // The constructor parameter type is a reference type
  export class B {
      constructor(arg: A) {}
  }
  export interface CC {
      data: number;
  }

  export class C implements CC {
      data: number = 0;
  }
  export class D {
      constructor(arg: CC) {}
  }
  export class E {
      constructor(arg: Function) {}
  }

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  import {A, B, C, D, E} from "file1"

  new A(); // CTE
  new A(1);  // OK

  new B(); // CTE
  new B(new A());  // OK
  let a = new A();
  new B(a);  // OK

  new C(); // OK
  new C(1); // CTE

  new D({data:0}); // OK
  new D(new C());  // OK

  new E(); // CTE
  new E(1); // OK, because the element type is mapped to ESObject
  new E(() => {}) // OK

Parameter passing

.. code-block:: typescript
  :linenos:

  // file1.ts TS
  export function dynFoo(arg: number) {}
  export function dynBar(arg: Function) {}
  export function dynCallCb(arg: (x: number) => void) {}
  export interface MyInface {
      data: number;
  }
  export class MyClass {}
  export function dynHandleMyInface(x: MyInface) {}
  export function dynHandleMyClass(x: MyClass) {}

  export let dynNum = 1;
  export let dynCb: (x: number) => void = (x: number) => {}
  export let dynMyInface: MyInface = {data: 0};
  export let dynMyClass = new MyClass();

.. code-block:: typescript
  :linenos:

  // file2.ets ArkTS
  import {...} from "file1"
  dynFoo(dynNum);  // OK
  dynBar(dynFoo);  // OK
  dynCallCb(dynCb); // OK
  dynHandleMyInface(dynMyInface);  // OK
  dynHandleMyClass(dynMyClass);  // OK

  function foo(arg: number) {}
  function fooDyn(arg: ESObject) {}
  function callCb(arg: (x: number) => void) {}
  function callCbDyn(arg: ESObject) {}
  function handleMyInface(x: MyInface) {}
  function handleMyInfaceDyn(x: ESObject) {}
  function handleMyClass(x: MyClass) {}
  function handleMyClassDyn(x: ESObject) {}

  foo(dynNum); // CTE, ESObject cannot be implicitly cast to number
  foo(dynNum as number);  // ok
  fooDyn(dynNum);  // OK

  callCb(dynCb); // CTE, ESObject cannot be implicitly cast to (x: number) => void
  callCb((x: number) => {dynCb(x)});  // OK
  callCbDyn(dynCb);  // OK

  handleMyInface(dynMyInface); // OK, The parameter type MyInface will degenerate into ESObject
  handleMyInfaceDyn(dynMyInface);  // OK

  handleMyClass(dynMyClass); // OK, The parameter type MyInface will degenerate into ESObject
  handleMyClassDyn(dynMyClass);  // OK
