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

.. _RT Runtime Type System:

Runtime Type System
*******************

The |LANG| runtime operates upon the following types:

- *Primitive types* : ``void``, ``u1``, ``i8``, ``u8``, ``i16``,
  ``u16``, ``i32``, ``u32``, ``f32``, ``f64``, ``i64``, ``u64``,
  ``any``.
- *Reference types* : All other types. All *reference types* have
  corresponding :ref:`ForeignClass <RT ForeignClass>` or :ref:`Class <RT Class>` in a binary file.

The |LANG| runtime additionally distinguishes between the above *reference type* groups as follows:

- ``Any`` : Top type of the runtime type system. Correspondent to the 
  :ref:`Class <RT Class>` with the :ref:`Type Descriptor <RT Type Descriptor>` that matches ``RefType`` with a
  :ref:`Fully Qualified Name <RT Fully Qualified Name>` ``Y``. **NOTE:** NOT the same as ``any``
  in *primitive types*.
- ``never`` : Bottom type of the runtime type system. Correspondent to the
  :ref:`Class <RT Class>` with the :ref:`Type Descriptor <RT Type Descriptor>` that matches ``RefType`` with a
  :ref:`Fully Qualified Name <RT Fully Qualified Name>` ``N``.
- *Array types* : Types that represent arrays of some *component type*.
  Correspondent to a :ref:`Class <RT Class>` with the :ref:`Type Descriptor <RT Type Descriptor>` that matches
  ``ArrayType``.
- *Union types* : Types that represent unions of their *component types*.
  Correspondent to a :ref:`Class <RT Class>` with the :ref:`Type Descriptor <RT Type Descriptor>` that
  matches ``UnionType``.
- ``Object`` : Base type of all *User-defined types* and *Array types*.
  Correspondent to the :ref:`Class <RT Class>` with the :ref:`Type Descriptor <RT Type Descriptor>` that matches
  ``RefType`` with a :ref:`Fully Qualified Name <RT Fully Qualified Name>` ``std.core.Object``.
- *User-defined types* : *Reference types* that belong
  to none of the groups above.

Each of these types is represented at runtime with instances of the
structure named ``Class``. Each unique type has only a single instance
of ``Class`` structure that falls into one of the categories above.

|

.. _RT Subtyping:

Subtyping
=========

*Reference types* and *Primitive types* form the type hierarchy described
below:

::

                         Any
               __________/ \__________
              /                       \
   *Primitive types*                Object
             |                ________/ \________
             |               /                   \
             |        *Array types*      *User-defined types*        *Union types*
             |               \___                 |                   ___/
             |                   \                |                  /
             |                   +-----------------------------------+
             |                   | language reference type hierarchy |
             |                   +-----------------------------------+
              \__________   _____________________/
                         \ /
                        never

The following statement holds true for *primitive type* subtyping: one *primitive type*
is a subtype of another *primitive type* only if both are of the same type.

For *Array types*, *Union types* and *User-defined types* subtyping is
mandated by the |LANG| subtyping rules.