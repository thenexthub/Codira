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

Modules
=======

|

Programs are organized as sets of compilation units or modules.

Each module creates its own scope, i.e., any declarations (variables,
functions, classes, etc.) declared in the module are only visible outside
that module if exported explicitly.

Conversely, a variable, function, class, interface, etc., exported from
another module must be imported to a module first.

|

Export
------

A top-level declaration can be exported by using the keyword ``export``.

A declared name that is not exported is considered private, and can be used
only in the module it is declared in:

.. code-block:: typescript

    export class Point {
        x: number = 0
        y: number = 0
        constructor(x: number, y: number) {
          this.x = x
          this.y = y
        }
    }
    export let Origin = new Point(0, 0)
    export function Distance(p1: Point, p2: Point): number {
        return Math.sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y))
    }

|

Import
------

Import declarations are used to import entities exported from other modules,
and to provide their bindings in the current module. An import declaration
consists of the following two parts:

* Import path that determines the module to import from;
* Import bindings that define the set of usable entities in the imported
  module, and the form of use (i.e., qualified or unqualified use).

Import bindings can have several forms.

If a module has the path '``./utils``', export entities 'X' and 'Y', and
an import binding of the form '``* as A``' binds the name '``A``', then
the qualified name ``A.name`` can be used to access all entities exported
from the module defined by the import path:

.. code-block:: typescript

    import * as Utils from "./utils"
    Utils.X // denotes X from Utils
    Utils.Y // denotes Y from Utils

An import binding of the form '``{ ident1, ..., identN }``' binds an exported
entity with a specified name, which can be used as a simple name:

.. code-block:: typescript

    import { X, Y } from "./utils"
    X // denotes X from Utils
    Y // denotes Y from Utils

If a list of identifiers contains aliasing of the form '``ident as alias``',
then entity ``ident`` is bound under the name ``alias``:

.. code-block:: typescript

    import { X as Z, Y } from "./utils"
    Z // denotes X from Utils
    Y // denotes Y from Utils
    X // Compile-time error: 'X' is not visible

|

Top-Level Statements
---------------------

At the module level, a module can contain any statements, except ``return``
ones.

If a module contains a ``main`` function (program entry point), then
top-level statements of the module are executed immediately before
the body of that function. Otherwise, such statements are executed
before the execution of any other function of the module.

|

Program Entry Point
--------------------

The top-level ``main`` function is an entry point of a program (application).
The ``main`` function must have either an empty parameter list, or a single
parameter of ``string[]`` type.

.. code-block:: typescript

    function main() {
        console.log("this is the program entry")
    }

|
|
