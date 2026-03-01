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

++++++++++++
Introduction
++++++++++++

Terminology explaination
------------------------

+-----------------+------------------------------------------------------------------------------------------+
| Terminology     | Explaination                                                                             |
+=================+==========================================================================================+
| ArkTS           | The ArkTS with evolved ArkTS syntax and semantics                                        |
+-----------------+------------------------------------------------------------------------------------------+
| TS              | abbreviation for Typescript                                                              |
+-----------------+------------------------------------------------------------------------------------------+
| JS              | abbreviation for Javascript                                                              |
+-----------------+------------------------------------------------------------------------------------------+
| interop         | abbreviation for interoperation                                                          |
+-----------------+------------------------------------------------------------------------------------------+
| CTE             | abbreviation for compile time error                                                      |
+-----------------+------------------------------------------------------------------------------------------+
| RTE             | abbreviation for runtime error                                                           |
+-----------------+------------------------------------------------------------------------------------------+
| UB              | abbreviation for undefined behavior                                                      |
+-----------------+------------------------------------------------------------------------------------------+
| basic operators | property access ``.``, invocation ``()``, index access ``[]``, and instantiation ``new`` |
+-----------------+------------------------------------------------------------------------------------------+

Background
----------

With evolution of ArkTS language, ArkTS will provide more language features and some syntax and semantics also evolve. Moreover, ArkTS native compiler with complete type checking capabilities will be introduced. Althrough it is not mandatory to migrate existing ArkTS and TS/JS code to Evolved ArkTS. However, for those developers who want to migrate, it is necessary to provide interoperability between ArkTS and TS/JS for better migration experience and for the requirement of reusing existing code assets. This document shows how ArkTS interoperates with TS/JS, which is helpful in the following scenarios:

- migrate existing code to ArkTS

- reuse some libraries of TS/JS code in migrated ArkTS modules

We will elaborate interop rules in the upcoming chapters. Before that we emphasize that we basically provide these rules in the way of "white list", which means that if some scenarios are not mentioned, then they are not supported. For unsupported scenarios, there may be CTE or RTE or UB. In the future we may extend our "white list" and some unsupported scenarios will become supported, but before that unsupported scenarios should be avoided for better stability.

Managed interop is part of runtime and frontend of ArkTS. In runtime ArkTS communicates with TS/JS via special native bridge. Additionally there can be typecheck for some cases at compile time. It is just additional stage for better user experience, but it is independent with runtime part.

This document describes specification of interop between ArkTS and TS/JS, and additional compile time type checking for interop between ArkTS and TS/JS.

Interop via 'import' and import rules
-------------------------------------

Our interop can be made via `import`, like what we have done in TS. However, we do have some rules for import directions.

- ArkTS files can import TS/JS files, with additional restrictions:

    1. ArkTS class cannot `extends` TS/JS class

    2. ArkTS class cannot `implements` TS interfaces

- TS/JS files cannot import ArkTS files

For now we only support the syntax ``"import {something} from 'path'"`` for interop import scenarios, and other import syntax, such as ``"import {something as alias} from 'path'"`` and ``"import defaultSomething from 'path'"`` and ``"import * as something from 'path'"``, is not supported.

Some preliminaries about ArkTS
---------------------------------------

Here we list some important points for completeness. These points should be able to be implied by ArkTS language specification.

ArkTS static semantics
======================

- ArkTS has static nominal type system and structural typing is forbidden.
- There is complete strict type checking during ArkTS compilation and execution.
- The layout of ArkTS objects cannot be dynamically changed. For example, it is forbidden to add/delete fields or methods of ArkTS classes and its instances. It is also forbidden to assign filed with a value whose type is different from the type of field declaration.
- There is runtime type check for implicit and explicit type conversion.

ESObject in ArkTS
=================

- ESObject is a special type, which is independent of Object type.
- ESObject can be used as type annotation and type parameter in generics.
- ESObject can be used to wrap data and objects from TS/JS and ArkTS. In particular, it can be used to wrap null and undefined.
- Basic operators and explicit type conversion on object of ESObject type are supported without compilation type check. And result type of basic operators on object of ESObject is still ESObject.
- Types in ArkTS can be implicitly or explicitly converted to ESObject types, but ESObject is not allowed to be implicitly converted to other types of ArkTS.
- It is not allowed to defined Object literals of ESObject type.

Note: Abusing ESObject is strongly not recommended, because type checking for ESObject capability is weakened at compile time, which can easily leave coding errors to runtime. Also, there will be additional execution logics for operations on ESObject at runtime, resulting in worse runtime performance.

Note: Using ESObject as type arguments in generics is not supported for now.

Examples
********

.. code-block:: typescript
    :linenos:

    // ArkTS code
    class A {
      v: number = 123
    }
    function foo(x: ESObject) {}

    let a: ESObject = new A()  // OK, implicitly convert A to ESObject
    a.v     // OK
    a[0]    // no CTE, but RTE, a is not indexable
    a()     // no CTE, but RTE, a is not a function
    new a() // no CTE, but RTE, a is not a constructor

    let n1: number = a.v  // CTE, cannot implicitly convert ESObject to number
    let n2: number = a.v as number // OK
    let n3: ESObject = a.v  // OK
    a.v + 1  // CTE, no '+' operation between ESObject and number
    (a.v as number) + 1  // OK

    let s1: ESObject = 1  // OK, implicitly convert number to ESObject
    let s2: ESObject = new A()  // OK, implicitly convert A to ESObject
    let s3: ESObject = 1 as ESObject // OK, explicitly convert number to ESObject
    let s4: ESObject = new A() as ESObject; // OK, explicitly convert A to ESObject

    foo(1); // OK, implicitly convert number to the ESObject
    foo(new A()) // OK, implicitly convert A to ESObject

    let point: ESObject = {x: 0, y: 1}  // CTE, cannot create object literal of type ESObject

    foo({x: 0, y: 1})  // CTE, cannot create object literal of type ESObject

    class G<T> {
      foo(x: T) {}
    }

    new G<number>()  // OK
    new G<ESObject>()  // CTE, cannot use ESObject as type arguments in generics

-  If basic operators on objects of type ESObject cannot be done properly, a runtime exception will be thrown.

.. code-block:: typescript
  :linenos:

  // ArkTS code
  let x: ESObject = undefined
  x[0]        // RTE, x is not indexable
  x()         // RTE, x is not a function
  x.f         // RTE, cannot access property of undefined
  new x()     // RTE, x is not a constructor
  x as string // RTE, cannot cast undefined to string

Primitive types in ArkTS
=================================

In ArkTS, there are the following primitive types:

- number
- byte
- short
- int
- long
- float
- double
- bigint
- string
- boolean
- undefined
- null

Principles for compilation type check
-------------------------------------

- Support use TS types in ArkTS files as type annotations.

- In ArkTS, everything imported from JS has type ESObject.

- Compile-time type checking should be checked according to the rules of the file type: ArkTS files should follow the type checking rules of ArkTS, TS files should follow the type checking rules of TS. The interop part will be checked based on :ref:`Principles for compilation type mapping rules`, after type mapping to the type of the file where it is located.

- When the ArkTS files are compiled into bytecode, all type annotations from TS in the ArkTS files will degenerate into ESObject.

- When the ArkTS files are compiled into bytecode, the types of all objects imported from TS/JS will degenerate into ESObject.

.. _Principles for compilation type mapping rules:

Principles for compilation type mapping rules
---------------------------------------------
- Primitive types are mapped into primitive types(e.g number -> number).(see :ref:`Typecheck conversion rules. Primitive values`)

- Composed types (classes/interfaces/structs) are mapped into composed types. Moreover, nested types will be mapped into nested types.(e.g interface Y {x: X, s: string} -> interface Y {x: X, s: string}) (see :ref:`Typecheck conversion rules. Reference values`)

Principles for runtime mapping rules
--------------------------------------------

- Primitive types will be mapped into primitive types (except special cases, see :ref:`Conversion rules Primitive values`).

- ArkTS reference types will be mapped into some proxy objects in TS/JS.

- TS/JS objects will be mapped into `ESObject` in ArkTS runtime.
