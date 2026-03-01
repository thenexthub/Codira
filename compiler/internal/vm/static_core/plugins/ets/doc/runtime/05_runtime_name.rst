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

.. _RT Runtime Name:

Runtime Name
************

The |LANG| runtime (e.g. standard library reflection, and class loading APIs)
and build time (build system, compiler, bytecode manipulation tools) use
*runtime name* to work with modules, classes, and other entities.
The syntax of a *runtime name* is presented below:

.. code-block:: abnf

    RuntimeName:
        PrimitiveType
        | ArrayType
        | RefType
        | UnionType
        ;
    PrimitiveType:
        "void"
        | "u1"
        | "i8"
        | "u8"
        | "i16"
        | "u16"
        | "i32"
        | "u32"
        | "f32"
        | "f64"
        | "i64"
        | "u64"
        | "any"
        ;
    ArrayType:
        RuntimeName '[]'
        ;
    RefType:
        RefTypeName '[]'
        ;
    UnionType:
        '{U' RuntimeName ',' RuntimeName (',' RuntimeName)* '}'
        ;

``RefTypeName`` is the :ref:`Fully Qualified Name <RT Fully Qualified Name>` of the type.

**Constraint**: ``UnionType`` must be canonicalized. Canonicalization
presumes sorting ``RuntimeName`` s of all *constituent types* alphabetically.
