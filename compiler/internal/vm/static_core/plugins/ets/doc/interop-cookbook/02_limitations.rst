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

.. _IO_R001:

|IO_DECL_RULE| ``Symbol`` type cannot be used in |LANG| interface declarations
------------------------------------------------------------------------------

|LANG| does not support ``Symbol`` type, which means that it cannot be used in
interface declarations. Use ``ESObject`` instead.

|IO_SEE_CB| ``arkts-no-symbol``.
|IO_SEE_CB| ``arkts-no-any-unknown`` FIXME: generalize
|IO_SEE_CB| ``arkts-no-conditional-types``  FIXME: generalize
|IO_SEE_CB| ``arkts-no-utility-types``  FIXME: generalize
|IO_SEE_CB| ``arkts-no-intersection-types``  FIXME: generalize
|IO_SEE_CB| ``arkts-no-obj-literals-as-types``  FIXME: generalize

|IO_DECL_TS|
~~~~~~~~~~~~

.. code-block:: typescript
    :linenos:

    // File: lib.ts
    export function foo(s: Symbol) { /* ... */ }
    export function bar(): Symbol { return new Symbol() }

|IO_DECL_IFACE|
~~~~~~~~~~~~~~~

.. code-block:: typescript
    :linenos:

    // File: lib.d.ets
    export function foo(s: Symbol) { /* ... */ }
    export function bar(): Symbol { return new Symbol() }

|IO_DECL_USAGE|
~~~~~~~~~~~~~~~

.. code-block:: typescript
    :linenos:

    // File: app.ets
    import { foo, bar } from "lib"

    let sym: ESObject = bar()
    foo(sym)

.. _IO_R002:

|IO_DECL_RULE| Call signatures cannot be used in |LANG| interface declarations
------------------------------------------------------------------------------

|LANG| does not support call signatures for interfaces, which means that they
cannot be used in interface declarations. FIXME: What should be used instead?

|IO_SEE_CB| ``arkts-no-call-signatures``

|IO_DECL_TS|
~~~~~~~~~~~~

.. code-block:: typescript
    :linenos:

    // File: lib.ts
    interface I {
        (n: number, s: string): boolean
    };

|IO_DECL_IFACE|
~~~~~~~~~~~~~~~

.. code-block:: typescript
    :linenos:

    // File: lib.d.ets
    /* FIXME: Interface declaration suitable for ArkTS */

|IO_DECL_USAGE|
~~~~~~~~~~~~~~~

.. code-block:: typescript
    :linenos:

    // File: app.ets
    /* FIXME: API usage here */

.. _IO_R003:

|IO_DECL_RULE| Constructor signatures cannot be used in |LANG| interface declarations
-------------------------------------------------------------------------------------

|LANG| does not support constructor signatures for interfaces, which means that they
cannot be used in interface declarations. FIXME: What should be used instead?

|IO_SEE_CB| ``arkts-no-ctor-signatures-type``
|IO_SEE_CB| ``arkts-no-ctor-signatures-iface`` FIXME: generalize?
|IO_SEE_CB| ``arkts-no-ctor-signatures-funcs`` FIXME: generalize?

|IO_DECL_TS|
~~~~~~~~~~~~

.. code-block:: typescript
    :linenos:

    // File: lib.ts
    export interface Iface {
        (n: number, s: string): boolean
    };

|IO_DECL_IFACE|
~~~~~~~~~~~~~~~

.. code-block:: typescript
    :linenos:

    // File: lib.d.ets
    /* FIXME: Interface declaration suitable for ArkTS */

|IO_DECL_USAGE|
~~~~~~~~~~~~~~~

.. code-block:: typescript
    :linenos:

    // File: app.ets
    /* FIXME: API usage here */

.. _IO_R004:

|IO_DECL_RULE| Index signatures cannot be used in |LANG| interface declarations
-------------------------------------------------------------------------------

|LANG| does not support index signatures for interfaces, which means that they
cannot be used in interface declarations. FIXME: What should be used instead?

|IO_SEE_CB| ``arkts-no-indexed-signatures``

|IO_DECL_TS|
~~~~~~~~~~~~

.. code-block:: typescript
    :linenos:

    // File: lib.ts
    export interface Iface {
        [key: string]: string | number | boolean
        anotherProperty: number
    }

|IO_DECL_IFACE|
~~~~~~~~~~~~~~~

.. code-block:: typescript
    :linenos:

    // File: lib.d.ets
    /* FIXME: Interface declaration suitable for ArkTS */

|IO_DECL_USAGE|
~~~~~~~~~~~~~~~

.. code-block:: typescript
    :linenos:

    // File: app.ets
    /* FIXME: API usage here */

.. _IO_Rxxx:

|IO_DECL_RULE| Short rule description
-------------------------------------

Detailed rule description.

|IO_SEE_CB| ``arkts-cookbook-rule-id``

|IO_DECL_TS|
~~~~~~~~~~~~

.. code-block:: typescript
    :linenos:

    // File: lib.ts
    /* FIXME: Original TypeScript interface */

|IO_DECL_IFACE|
~~~~~~~~~~~~~~~

.. code-block:: typescript
    :linenos:

    // File: lib.d.ets
    /* FIXME: Interface declaration suitable for ArkTS */

|IO_DECL_USAGE|
~~~~~~~~~~~~~~~

.. code-block:: typescript
    :linenos:

    // File: app.ets
    /* FIXME: API usage here */

|IO_SEE_CB| ``arkts-implements-only-iface``
|IO_SEE_CB| ``arkts-extends-only-class``
|IO_SEE_CB| ``arkts-no-extend-same-prop``

|IO_SEE_CB| ``arkts-identifiers-as-prop-names``
|IO_SEE_CB| ``arkts-no-private-identifiers``
|IO_SEE_CB| ``arkts-unique-names``
|IO_SEE_CB| ``arkts-no-var``
|IO_SEE_CB| ``arkts-no-typing-with-this``
|IO_SEE_CB| ``arkts-no-aliases-by-index``

|IO_SEE_CB| ``arkts-no-type-query``
|IO_SEE_CB| ``arkts-no-mapped-types``
|IO_SEE_CB| ``arkts-no-destruct-params``
|IO_SEE_CB| ``arkts-no-generators``
|IO_SEE_CB| ``arkts-no-is``

|IO_SEE_CB| ``arkts-no-decl-merging``

|IO_SEE_CB| ``arkts-no-enum-mixed-types``
|IO_SEE_CB| ``arkts-no-enum-merging``

|IO_SEE_CB| ``arkts-no-import-default-as``
|IO_SEE_CB| ``arkts-no-export-assignment``

|IO_SEE_CB| ``arkts-no-ambient-decls``
|IO_SEE_CB| ``arkts-no-module-wildcards``
|IO_SEE_CB| ``arkts-no-umd``
|IO_SEE_CB| ``arkts-no-import-assertions``
|IO_SEE_CB| ``arkts-no-misplaced-imports``
|IO_SEE_CB| ``arkts-no-ts-deps``
|IO_SEE_CB| ``arkts-limited-esobj``
